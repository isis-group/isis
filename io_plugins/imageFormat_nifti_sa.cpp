#include <isis/core/fileptr.hpp>
#include "imageFormat_nifti_sa.hpp"
#include "imageFormat_nifti_dcmstack.hpp"
#include <isis/math/transform.hpp>
#include <errno.h>
#include <fstream>
#include <regex>


namespace isis
{
namespace image_io
{
namespace _internal
{
WriteOp::WriteOp( const data::Image &image, size_t bitsPerVoxel ): data::NDimensional<4>( image ), m_bpv( bitsPerVoxel ) {}

void WriteOp::addFlip( data::dimensions dim ) {flip_list.insert( dim );}

size_t WriteOp::getDataSize()
{
	size_t bitsize = getVolume() * m_bpv;

	if( bitsize % 8 )
		bitsize += 8;

	return bitsize / 8;
}

template<typename TYPE1, typename TYPE2, size_t N> void copyArray2Mem(const std::array<TYPE1,N> &src, TYPE2 *dst){
	std::copy(std::begin(src),std::end(src),dst);
}

bool WriteOp::setOutput( const std::string &filename, size_t voxelstart )
{
	m_out = data::FilePtr( filename, voxelstart + getDataSize(), true );
	m_voxelstart = voxelstart;

	if( m_out.good() ) {
		nifti_1_header *header = getHeader();
		memset( header, 0, sizeof( _internal::nifti_1_header ) );

		// store the image size in dim and fill up the rest with "1" (to prevent fsl from exploding)
		header->dim[0] = getRelevantDims();
		_internal::copyArray2Mem(getSizeAsVector(), header->dim + 1 );
		std::fill( header->dim + 5, header->dim + 8, 1 );

		//some nifti readers expect analyze fields, but for now we disable this
		/*header->extents=16*1024;
		header->regular='r';
		memcpy(header->data_type,"dsr        ",10);*/
		header->sizeof_hdr = 348; // must be 348
		header->vox_offset = m_voxelstart;
		header->bitpix = m_bpv;
		return m_out.good();
	} else
		return false;
}

nifti_1_header *WriteOp::getHeader() {return reinterpret_cast<nifti_1_header *>( &m_out[0] );}

bool WriteOp::operator()( data::Chunk &ch, util::vector4<size_t> posInImage )
{
	if( doCopy( ch, posInImage ) )
		return true;
	else {
		LOG( Runtime, error ) << "Failed to copy chunk at " << posInImage;
		return true;
	}
}
void WriteOp::applyFlipToData ( data::ValueArrayReference &dat, util::vector4< size_t > chunkSize )
{
	if( !flip_list.empty() ) {
		// wrap the copied part back into a Chunk to flip it
		data::Chunk cp( dat, chunkSize[data::rowDim], chunkSize[data::columnDim], chunkSize[data::sliceDim], chunkSize[data::timeDim] ); // this is a cheap copy
		applyFlipToData( cp ); // and apply the flipping
	}
}
void WriteOp::applyFlipToData ( data::Chunk &dat )
{
	//iterate through all flips within the block dimensionality
	for( std::set<data::dimensions>::const_iterator i = flip_list.begin(); i != flip_list.end() && *i < dat.getRelevantDims(); i++ ) {
		dat.flipAlong( *i ); // .. so changing its data, will also change the data we just copied
	}
}

void WriteOp::applyFlipToCoords ( util::vector4< size_t >& coords, data::dimensions blockdims )
{
	if( !flip_list.empty() ) {
		//iterate through all flips above than the block dimensionality
		for( std::set<data::dimensions>::const_iterator i = flip_list.lower_bound( blockdims ); i != flip_list.end(); i++ ) {
			coords[*i] = getDimSize( *i ) - coords[*i] - 1;
		}
	}
}


class CommonWriteOp: public WriteOp
{
	const unsigned short m_targetId;
	data::scaling_pair m_scale;
public:
	CommonWriteOp( const data::Image &image, unsigned short targetId, size_t bitsPerVoxel ):
		WriteOp( image, bitsPerVoxel ),
		m_targetId( targetId ) {
			if(image.getMajorTypeID() == targetId){
				m_scale.first=util::Value<uint8_t>(1);
				m_scale.second=util::Value<uint8_t>(0);
			} else
				m_scale=image.getScalingTo( m_targetId );
		}

	bool doCopy( data::Chunk &ch, util::vector4<size_t> posInImage ) {
		applyFlipToCoords( posInImage, ( data::dimensions )ch.getRelevantDims() );
		size_t offset = m_voxelstart + getLinearIndex( posInImage ) * m_bpv / 8;
		data::ValueArrayReference out_data = m_out.atByID( m_targetId, offset, ch.getVolume() );
		ch.asValueArrayBase().copyTo( *out_data, m_scale );

		applyFlipToData( out_data, ch.getSizeAsVector() );
		return true;
	}

	short unsigned int getTypeId() {return m_targetId;}
};

class FslRgbWriteOp: public WriteOp
{
	const data::scaling_pair m_scale;
	struct VoxelCp: data::VoxelOp<util::color24> {
		uint8_t mode;
		uint8_t *ptr;
		virtual bool operator()( util::color24 &vox, const isis::util::vector4<size_t>& /*pos*/ ) {
			switch( mode ) {
			case 0:
				*ptr = vox.r;
				break;
			case 1:
				*ptr = vox.g;
				break;
			case 2:
				*ptr = vox.b;
				break;
			}

			ptr++;
			return true;
		}
	};
public:
	FslRgbWriteOp( const data::Image &image ):
		WriteOp( image, 8 ), m_scale( util::ValueReference( util::Value<uint8_t>( 1 ) ), util::ValueReference( util::Value<uint8_t>( 0 ) ) ) {
		assert( image.getDimSize( 3 ) == 1 ); //make sure the image has only one timestep
		std::array<size_t,4> dims=image.getSizeAsVector();
		dims[3] = 3;
		init( dims ); // reset our shape to use 3 timesteps as colors
	}

	bool doCopy( data::Chunk &src, util::vector4<size_t> posInImage ) {
		data::Chunk ch = src;
		ch.convertToType( data::ValueArray<util::color24>::staticID(), m_scale );
		VoxelCp cp;
		assert( posInImage[data::timeDim] == 0 );

		for( ; posInImage[data::timeDim] < 3; posInImage[data::timeDim]++ ) { //copy each color/timestep into m_out
			const size_t offset = m_voxelstart + getLinearIndex( posInImage ) * m_bpv / 8;
			data::ValueArray<uint8_t> out_data = m_out.at<uint8_t>( offset, ch.getVolume() );
			cp.ptr = &out_data[0];
			cp.mode = ( uint8_t )posInImage[data::timeDim]; //the "timesteps" represent the color thus there are just 3
			ch.foreachVoxel( cp );
			assert( cp.ptr == &out_data[0] + out_data.getLength() );
		}

		return true;
	}

	short unsigned int getTypeId() {return data::ValueArray<uint8_t>::staticID();}
};

class BitWriteOp: public WriteOp
{
public:
	BitWriteOp( const data::Image &image ): WriteOp( image, 1 ) {}

	bool doCopy( data::Chunk &src, util::vector4<size_t> posInImage ) {
		data::ValueArray<bool> in_data = src.asValueArrayBase().as<bool>();
		const size_t offset = m_voxelstart + getLinearIndex( posInImage ) * m_bpv / 8;

		//the length parameter actually expets elements and then computes the bytes internally
		//but that computing will fail for bool so we ask for 8bit / 1byte type and give the needed bytes instead of elements
		//also the writing below works with uint8 anyway
		data::ValueArray<uint8_t> out_data = m_out.at<uint8_t>( offset, in_data.getLength() / 8 );
		memset( &out_data[0], 0, out_data.getLength() );

		for( size_t i = 0; i < in_data.getLength(); i++ ) {
			const size_t byte = i / 8;
			const uint8_t mask = 128 >> ( i % 8 );

			if( in_data[i] ) {
				out_data[byte] |= mask;
			}
		}

		return true;
	}

	short unsigned int getTypeId() {return data::ValueArray<bool>::staticID();}
};

}

ImageFormat_NiftiSa::ImageFormat_NiftiSa()
{
	nifti_type2isis_type[NIFTI_TYPE_INT8 ] = data::ValueArray< int8_t>::staticID();
	nifti_type2isis_type[NIFTI_TYPE_INT16] = data::ValueArray<int16_t>::staticID();
	nifti_type2isis_type[NIFTI_TYPE_INT32] = data::ValueArray<int32_t>::staticID();
	nifti_type2isis_type[NIFTI_TYPE_INT64] = data::ValueArray<int64_t>::staticID();

	nifti_type2isis_type[NIFTI_TYPE_UINT8 ] = data::ValueArray< uint8_t>::staticID();
	nifti_type2isis_type[NIFTI_TYPE_UINT16] = data::ValueArray<uint16_t>::staticID();
	nifti_type2isis_type[NIFTI_TYPE_UINT32] = data::ValueArray<uint32_t>::staticID();
	nifti_type2isis_type[NIFTI_TYPE_UINT64] = data::ValueArray<uint64_t>::staticID();

	nifti_type2isis_type[NIFTI_TYPE_FLOAT32] = data::ValueArray<float>::staticID();
	nifti_type2isis_type[NIFTI_TYPE_FLOAT64] = data::ValueArray<double>::staticID();

	nifti_type2isis_type[NIFTI_TYPE_RGB24] = data::ValueArray<util::color24>::staticID();

	nifti_type2isis_type[NIFTI_TYPE_COMPLEX64] = data::ValueArray<std::complex<float> >::staticID();
	nifti_type2isis_type[NIFTI_TYPE_COMPLEX128] = data::ValueArray<std::complex<double> >::staticID();

	nifti_type2isis_type[NIFTI_TYPE_BINARY] = data::ValueArray<bool>::staticID();

	typedef std::map<short, unsigned short>::const_reference ref_type;
	for( ref_type ref :  nifti_type2isis_type ) {
		isis_type2nifti_type[ref.second] = ref.first;
	}

}

void ImageFormat_NiftiSa::flipGeometry( data::Image &image, data::dimensions flipdim )
{
	static const char *names[] = {"rowVec", "columnVec", "sliceVec"};
	assert( flipdim <= data::sliceDim );
	const float vsize = 
		image.getValueAs<util::fvector3>( "voxelSize" )[flipdim] +
		image.getValueAsOr<util::fvector3>( "voxelGap", util::fvector3{0,0,0} )[flipdim];
	const float middle_to_middle = ( image.getSizeAsVector()[flipdim] - 1 ) * vsize; // the distance from the middle of the current first voxel to the "going to be first"
	util::fvector3 &vec = image.refValueAs<util::fvector3>( names[flipdim] );
	util::fvector3 &origin = image.refValueAs<util::fvector3>( "indexOrigin" );
	origin += vec * middle_to_middle; // move the origin along the repective edge to "the other end"
	LOG( Debug, verbose_info ) << "moved indexOrigin along " << vec *middle_to_middle << " to " << origin;
	vec *= -1; // and invert that vector
}

float ImageFormat_NiftiSa::determinant( const util::Matrix3x3< float >& m )
{
	return 
		  m[0][0] * m[1][1] * m[2][2] + m[1][0] * m[2][1] * m[0][2] + m[2][0] * m[0][1] * m[1][2]
		- m[0][0] * m[2][1] * m[1][2] - m[1][0] * m[0][1] * m[2][2] - m[2][0] * m[1][1] * m[0][2];
}

void ImageFormat_NiftiSa::guessSliceOrdering( const data::Image img, char &slice_code, float &slice_duration )
{

	if( img.getChunk( 0, 0, 0, 0, false ).getRelevantDims() != data::sliceDim || img.getSizeAsVector()[data::sliceDim] <= 1 ) {
		// chunks are no slices, or there is only one slice - just choose NIFTI_SLICE_SEQ_INC
		slice_code = NIFTI_SLICE_SEQ_INC;
	} else {
		util::PropertyMap::PropPath order = img.getChunk( 0, 0, 0, 0, false ).hasProperty( "acquisitionTime" ) ? "acquisitionTime" : "acquisitionNumber";
		const util::PropertyValue first = img.getChunk( 0, 0, 0, 0, false ).queryProperty( order ).get(); // acquisitionNumber _must_ be chunk-unique - so it is there even without a join
		const util::PropertyValue second = img.getChunk( 0, 0, 1, 0, false ).queryProperty( order ).get();
		const util::PropertyValue middle = img.getChunk( 0, 0, img.getSizeAsVector()[data::sliceDim] / 2 + .5, 0, false ).queryProperty( order ).get();

		if( first.gt( second ) ) { // second slice has a lower number than the first => decrementing
			if( middle.gt( second ) ) { // if the middle number is greater than the second its interleaved
				LOG( Runtime, info )
						<< "The \"middle\" " << order << " (" << middle.toString() << ") is greater than the second (" << second.toString()
						<< ") assuming decrementing interleaved slice order";
				slice_code = NIFTI_SLICE_ALT_DEC;
			} else { // assume "normal" otherwise
				LOG( Runtime, info )
						<< "The first " << order << " (" << first.toString() << ") is greater than the second (" << second.toString()
						<< ") assuming decrementing slice order";
				slice_code = NIFTI_SLICE_SEQ_DEC;
			}
		} else { // assume incrementing
			if( middle.lt( second ) ) { // if the middle number is less than the second ist interleaved
				LOG( Runtime, info )
						<< "The \"middle\" " << order << " (" << middle.toString() << ") is less than the second (" << second.toString()
						<< ") assuming incrementing interleaved slice order";
				slice_code = NIFTI_SLICE_ALT_INC;
			} else { // assume "normal" otherwise
				LOG( Runtime, info )
						<< "The first " << order << " (" << first.toString() << ") is not greater than the second (" << second.toString()
						<< ") assuming incrementing slice order";
				slice_code = NIFTI_SLICE_SEQ_INC;
			}
		}
		if(first.is<util::timestamp>() && second.is<util::timestamp>()){
			//can't simply use first-second because that assumes result type timestamp
			//@todo add support for such operations
			util::duration duration= first.as<util::timestamp>() - second.as<util::timestamp>();  
			if(slice_code == NIFTI_SLICE_ALT_INC || slice_code == NIFTI_SLICE_ALT_DEC) //@todo test me
				duration /=  img.getSizeAsVector()[data::sliceDim] / 2;
			slice_duration=std::fabs(std::chrono::milliseconds(duration).count());
		}
	}

}

void ImageFormat_NiftiSa::parseSliceOrdering( const std::shared_ptr< isis::image_io::_internal::nifti_1_header >& head, data::Chunk &current )
{
	
// 	The following table indicates the slice timing pattern, relative to	time=0 for the first slice acquired, for some sample cases.  
// 	
// 	slice  SEQ_INC SEQ_DEC ALT_INC ALT_DEC ALT_INC2 ALT_DEC2
// 	1  :   0.0     0.4     0.0     0.2     0.2      0.4    
// 	2  :   0.1     0.3     0.3     0.4     0.0      0.1    
// 	3  :   0.2     0.2     0.1     0.1     0.3      0.3    
// 	4  :   0.3     0.1     0.4     0.3     0.1      0.0    
// 	5  :   0.4     0.0     0.2     0.0     0.4      0.2    
	
	double time_fac;

	switch( head->xyzt_units & 0x38 ) {
	case NIFTI_UNITS_SEC:
		time_fac = 1e3;
		break;
	case NIFTI_UNITS_USEC:
		time_fac = 1e-3;
		break;
	default:
		time_fac = 1;
		break;
	}

	//if the sequence is "normal"
	current.setValueAs<uint32_t>( "acquisitionNumber", 1 );
	const size_t dims = current.getRelevantDims();
	assert( dims <= 4 ); // more than 4 dimensions are ... well, not expected

	if( head->slice_code <= NIFTI_SLICE_SEQ_INC  || head->slice_code > NIFTI_SLICE_ALT_DEC ) {
		if( head->slice_duration == 0 ) { // and there is no slice duration, there is no use in numbering
			return;
		}
	}

	if( dims < 3 ) { // if there is only one slice, there is no use in numbering
		return;
	} else {// if there are timesteps we have to get a bit dirty
		util::PropertyValue &acqProp=current.touchProperty( "acquisitionNumber" );
		
		switch( head->slice_code ) { //set sub-property "acquisitionNumber" based on the slice_code and the offset
		default:
			LOG( Runtime, error ) << "Unknown slice code " << util::MSubject( ( int )head->slice_code ) << " falling back to NIFTI_SLICE_SEQ_INC";
		case 0:
		case NIFTI_SLICE_SEQ_INC: //system assumes this anyway when the chunk is spliced up -- no explicit values needed
		break;
		case NIFTI_SLICE_SEQ_DEC:{
			acqProp.reserve(head->dim[3]*head->dim[4]);
			acqProp.resize(0,util::Value<uint32_t>(0));

			for(short v=0;v<head->dim[4];v++)
				for(unsigned short i = 0; i < head->dim[3]; i++ ){
					assert(v*head->dim[3]+head->dim[3]>=i);
					acqProp.push_back<uint32_t>(v*head->dim[3]+head->dim[3]-i);
				}
		}
		break;
		case NIFTI_SLICE_ALT_INC: { //interlaced increment
			acqProp.reserve(head->dim[3]*head->dim[4]);
			acqProp.resize(0,util::Value<uint32_t>(0));
			
			for(short v=0;v<head->dim[4];v++){
				for( short i = 0; i < head->dim[3]; i+=2)
					acqProp.push_back<uint32_t>(v*head->dim[3]+i);
				for( short i = 1; i < head->dim[3]; i+=2)
					acqProp.push_back<uint32_t>(v*head->dim[3]+i);
			}
		}
		break;
		case NIFTI_SLICE_ALT_DEC: {
			acqProp.reserve(head->dim[3]*head->dim[4]);
			acqProp.resize(0,util::Value<uint32_t>(0));

			for(short v=0;v<head->dim[4];v++){
				for( short i = head->dim[3]-1; i>=0; i-=2)
					acqProp.push_back<uint32_t>(v*head->dim[3]+i);
				for( short i = head->dim[3]-2; i>=0; i-=2)
					acqProp.push_back<uint32_t>(v*head->dim[3]+i);
			}
		}
		break;
		}

		if( head->slice_duration ) {
			util::PropertyValue &acqTimeProp=current.touchProperty( "acquisitionTime");
                        const util::timestamp start=current.getValueAsOr<util::timestamp>("sequenceStart",util::timestamp());
			acqTimeProp.reserve(head->dim[3]*head->dim[4]);
			for(util::PropertyValue::const_iterator i=acqProp.begin();i!=acqProp.end();i++){
				acqTimeProp.push_back(start+std::chrono::milliseconds(i->as<int>() * int(head->slice_duration * time_fac)));
			}
		}
	}
}


void ImageFormat_NiftiSa::storeDescripForSPM( const util::PropertyMap &props, char desc[] )
{
	std::list<std::string> ret;
	typedef const char *prop_pair[3];
	const prop_pair  pairs[] = {{"TR", "repetitionTime", "ms"}, {"TE", "echoTime", "ms"}, {"FA", "flipAngle", "deg"}, {"timestamp", "sequenceStart", ""}};
	for( const prop_pair & p :  pairs ) {
		if( props.hasProperty( p[1] ) ) {
			ret.push_back( std::string( p[0] ) + "=" + props.getValueAs<std::string>( p[1] ) + p[2] );
		}
	}
	strncpy( desc, util::listToString( ret.begin(), ret.end(), "/", "", "" ).c_str(), 80 );
}

bool ImageFormat_NiftiSa::parseDescripForSPM( isis::util::PropertyMap &props, const char desc[] )
{
	//check description for tr, te and fa and date which is written by spm8
	// @todo test against recent spm
	static const std::regex descriptionRegex(
		".*TR=([\\d]+)ms.*TE=([\\d]+)ms.*FA=([\\d]+)deg\\ *([\\d]{1,2}).([\\w]{3}).([\\d]{4})\\ *([\\d]{1,2}):([\\d]{1,2}):([\\d]{1,2}).*",
		std::regex_constants::ECMAScript|std::regex_constants::optimize
	); 
	std::cmatch results;

	if ( std::regex_match( desc, results,  descriptionRegex ) ) {
		props.setValueAs( "repetitionTime", std::stoi( results.str( 1 ) ) );
		props.setValueAs( "echoTime", std::stoi( results.str( 2 ) ) );
		props.setValueAs( "flipAngle", std::stoi( results.str( 3 ) ) );

		tm t={
			std::stoi(results.str( 9 )), //tm_sec
			std::stoi(results.str( 8 )), //tm_min
			std::stoi(results.str( 7 )), //tm_hour
			std::stoi(results.str( 4 )), // tm_mday
			std::stoi(results.str( 5 ))-1, // tm_mon [0, 11] 
			std::stoi(results.str( 6 ))-1900, // tm_year years since 1900
			0,0,-1,0,                           // tm_wday, tm_yday, tm_isdst
			nullptr
		};

		util::timestamp sequenceStart = std::chrono::time_point_cast<util::timestamp::duration>(std::chrono::system_clock::from_time_t(mktime(&t)));

		props.setValueAs<util::timestamp>( "sequenceStart", sequenceStart );

		LOG( Runtime, info ) << "Using Tr=" << props.queryProperty( "repetitionTime" ) << ", Te=" << props.queryProperty( "echoTime" )
							 << ", flipAngle=" << props.queryProperty( "flipAngle" ) << " and sequenceStart=" << props.queryProperty( "sequenceStart" )
							 << " from SPM8 description.";

		return true;
	} else
		return false;
}
void ImageFormat_NiftiSa::storeHeader( const util::PropertyMap &props, _internal::nifti_1_header *head )
{
	bool saved_sform = false, saved_qform = false;

	// implicit stuff
	head->intent_code = 0;
	head->slice_start = 0;
	head->slice_end = head->dim[3];
	head->scl_slope = 1;
	head->scl_inter = 0;

	//in isis length is allways mm and time duration is allways msecs
	head->xyzt_units = NIFTI_UNITS_MM | NIFTI_UNITS_MSEC;

	head->dim_info = 1 | ( 2 << 2 ) | ( 3 << 4 ); //readDim=1 phaseDim=2 sliceDim=3

	//store description if there is one
	if( props.hasProperty( "sequenceDescription" ) )
		strncpy( head->descrip, props.getValueAs<std::string>( "sequenceDescription" ).c_str(), 80 );

	// store niftis original sform if its there
	if( props.hasProperty( "nifti/sform_code" ) ) {
		head->sform_code = props.getValueAs<util::Selection>( "nifti/sform_code" );

		if( props.hasProperty( "nifti/srow_x" ) && props.hasProperty( "nifti/srow_y" ) && props.hasProperty( "nifti/srow_z" ) ) {
			_internal::copyArray2Mem(props.getValueAs<util::fvector4>( "nifti/srow_x" ), head->srow_x );
			_internal::copyArray2Mem(props.getValueAs<util::fvector4>( "nifti/srow_y" ), head->srow_y );
			_internal::copyArray2Mem(props.getValueAs<util::fvector4>( "nifti/srow_z" ), head->srow_z );
			saved_sform = true;
		}
	}

	// store niftis original qform if its there
	if( props.hasProperty( "nifti/qform_code" ) ) {
		head->qform_code = props.getValueAs<util::Selection>( "nifti/qform_code" );

		if( props.hasProperty( "nifti/quatern_b" ) && props.hasProperty( "nifti/quatern_c" ) && props.hasProperty( "nifti/quatern_d" ) &&
			props.hasProperty( "nifti/qoffset" ) && props.hasProperty( "nifti/qfac" )
		  ) {
			const util::fvector4 offset = props.getValueAs<util::fvector4>( "nifti/qoffset" );
			head->quatern_b = props.getValueAs<float>( "nifti/quatern_b" );
			head->quatern_c = props.getValueAs<float>( "nifti/quatern_c" );
			head->quatern_d = props.getValueAs<float>( "nifti/quatern_d" );
			head->pixdim[0] = props.getValueAs<float>( "nifti/qfac" );
			head->qoffset_x = offset[0];
			head->qoffset_y = offset[1];
			head->qoffset_z = offset[2];
			saved_qform = true;
		}
	}
	
	//store voxel size (don't use voxelSize, thats without voxelGap)
	const isis::util::Matrix4x4< double > nifti2image = util::transpose(getNiftiMatrix( props )); //use the inverse of image2nifti to extract direction vectors easier

	for( int i = 0; i < 3; i++ ) {
		//vector length of the i'th column (thats ok with vector4, because fourth element is allways 0)
		assert(nifti2image[i][3]==0); //just to be sure
		head->pixdim[i + 1] = util::len(nifti2image[i]); 
	}


	// store current orientation (into fields which hasn't been set by now)

	// qform is the original scanner coordinates and thus should always be there
	if( !saved_qform )
		storeQForm( props, head );

	// sform is to be used for geometry changes in postprocessing and can diverge from qform
	// spm apparently doesn't know about that and only looks for the sform, so we "violate" the rule and store it in any case
	if( !saved_sform )
		storeSForm( props, head );


	strcpy( head->magic, "n+1" );
}
void ImageFormat_NiftiSa::parseHeader( const std::shared_ptr< isis::image_io::_internal::nifti_1_header >& head, data::Chunk &props, data::scaling_pair &scl)
{
	unsigned short dims = head->dim[0];
	double time_fac = 1;
	double size_fac = 1;

	switch( head->xyzt_units & 0x07 ) {
	case NIFTI_UNITS_METER:
		size_fac = 1e3;
		break;
	case NIFTI_UNITS_MICRON:
		size_fac = 1e-3;
		break;
	}

	switch( head->xyzt_units & 0x38 ) {
	case NIFTI_UNITS_SEC:
		time_fac = 1e3;
		break;
        case NIFTI_UNITS_USEC: // @todo use std::ratio
		time_fac = 1e-3;
		break;
	}


	props.setValueAs<uint16_t>( "sequenceNumber", 0 );

	if( head->sform_code ) { // get srow if sform_code>0
		util::Selection code=formCode;
		if(code.set(head->sform_code))
			props.setValueAs( "nifti/sform_code", code );
		else
			LOG(Runtime,warning) << "ignoring unknown sform_code " << head->sform_code << "(known are: " << formCode.getEntries() << ")";
		
		props.touchProperty( "nifti/srow_x" ) = util::fvector4{head->srow_x[0], head->srow_x[1], head->srow_x[2], head->srow_x[3]};
		props.touchProperty( "nifti/srow_y" ) = util::fvector4{head->srow_y[0], head->srow_y[1], head->srow_y[2], head->srow_y[3]};
		props.touchProperty( "nifti/srow_z" ) = util::fvector4{head->srow_z[0], head->srow_z[1], head->srow_z[2], head->srow_z[3]};
	}

	if( head->qform_code ) { // get the quaternion if qform_code>0
		util::Selection code=formCode;
		if(code.set(head->qform_code))
			props.setValueAs( "nifti/qform_code", code );
		else
			LOG(Runtime,warning) << "ignoring unknown qform_code " << head->qform_code << "(known are: " << formCode.getEntries() << ")";

		props.setValueAs( "nifti/quatern_b", head->quatern_b );
		props.setValueAs( "nifti/quatern_c", head->quatern_c );
		props.setValueAs( "nifti/quatern_d", head->quatern_d );
		props.setValueAs( "nifti/qoffset", util::fvector4{head->qoffset_x, head->qoffset_y, head->qoffset_z, 0} );
		props.setValueAs( "nifti/qfac", ( head->pixdim[0] == -1 ) ? -1 : 1 );

		// copy pixdim
		util::dlist v_size( dims );
		std::copy( head->pixdim + 1, head->pixdim + dims + 1, v_size.begin() ); //@todo implement size_fac
		props.setValueAs( "nifti/pixdim", v_size );
	}

	if( head->sform_code ) { // if sform_code is set, use that regardless of qform
		useSForm( props );
	} else if( head->qform_code ) { // if qform_code is set, but no sform use that (thats the "normal" case)
		useQForm( props );
	} else {
		LOG( Runtime, warning ) << "Neither sform_code nor qform_code are set, using identity matrix for geometry";
		props.setValueAs( "rowVec",      util::fvector3{nifti2isis[0][0], nifti2isis[0][1], nifti2isis[0][2]} ); // we use the transformation from nifti to isis as unity
		props.setValueAs( "columnVec",   util::fvector3{nifti2isis[1][0], nifti2isis[1][1], nifti2isis[1][2]} ); // because the image will very likely be in nifti space
		props.setValueAs( "sliceVec",    util::fvector3{nifti2isis[2][0], nifti2isis[2][1], nifti2isis[2][2]} );
		props.setValueAs( "voxelSize",   util::fvector3{head->pixdim[1], head->pixdim[2], head->pixdim[3]} );
		props.setValueAs( "indexOrigin", util::fvector3{0,0,0} );
	}
	
	if(props.hasProperty("nifti/pixdim")){
		// make a vector3 from the nifti/pixdim-list
		const auto pixdim=props.getValueAs<util::dlist>("nifti/pixdim");
		util::fvector3 buffer;auto pixdim3=pixdim.begin();std::advance(pixdim3,3);
		std::copy(pixdim.begin(),pixdim3,std::begin(buffer));
	
		LOG_IF(!util::fuzzyEqualV(props.getValueAs<util::fvector3>("voxelSize"),buffer),Runtime,warning) 
			<< "the stored voxel size does not fit the computed voxel size (probably from sform)";
	}
	// set space unit factors
	props.refValueAs<util::fvector3>( "voxelSize"   ) *= size_fac;
	props.refValueAs<util::fvector3>( "indexOrigin" ) *= size_fac;

	// Tr
	if( head->pixdim[4] != 0 ) // if pixdim is given for the 4th dim, assume its repetitionTime
		props.setValueAs<uint16_t>( "repetitionTime", head->pixdim[4]*time_fac );

	// sequenceDescription
	if( strlen( head->descrip ) ) {
		if( !parseDescripForSPM( props, head->descrip ) ) // if descrip dos not hold Te,Tr and stuff (SPM dialect)
			props.setValueAs<std::string>( "sequenceDescription", head->descrip );// use it the usual way
	}

	// TODO: at the moment scaling is not supported due to data type changes
	if( ( head->scl_slope !=0 && head->scl_slope != 1) || head->scl_inter != 0)
		scl=data::scaling_pair(
			util::Value<float>(head->scl_slope),
			util::Value<float>(head->scl_inter)
		);

	if( head->intent_code  ) {
		props.setValueAs( "nifti/intent_code", head->intent_code ); // use it the usual way
		LOG( Runtime, warning ) << "Ignoring intent_code " << props.queryProperty( "nifti/intent_code" );
	}


	if( head->cal_max != 0 || head->cal_min != 0 ) { // maybe someone needs that, we dont ...
		props.setValueAs( "window/max", head->cal_max );
		props.setValueAs( "window/min", head->cal_min );
	}

}

std::string ImageFormat_NiftiSa::getName()const {return "Nifti standalone";}

isis::data::ValueArray< bool > ImageFormat_NiftiSa::bitRead( data::ValueArray<uint8_t> src, size_t size )
{
	assert( size );

	if( src.getLength() * 8 < size ) {
		std::string err( "unexpected end of file (missing " );
		err += std::to_string( size - src.getLength() * 8 ) + " bytes)";
		throwGenericError( err );
	}

	isis::data::ValueArray< bool > ret( size );

	for( size_t i = 0; i < size; i++ ) {
		const size_t byte = i / 8;
		const uint8_t mask = 128 >> ( i % 8 );
		ret[i] = mask & src[byte];
	}

	return ret;
}

bool ImageFormat_NiftiSa::checkSwapEndian ( std::shared_ptr< isis::image_io::_internal::nifti_1_header > header )
{
#define DO_SWAP(VAR) VAR=data::endianSwap(VAR)
#define DO_SWAPA(VAR,SIZE) data::endianSwapArray(VAR,VAR+SIZE,VAR);

	if( data::endianSwap( header->sizeof_hdr ) == 348 ) { // ok we have to swap the endianess
		DO_SWAP( header->sizeof_hdr );
		DO_SWAP( header->extents );
		DO_SWAP( header->session_error );

		DO_SWAP( header->intent_p1 );
		DO_SWAP( header->intent_p2 );
		DO_SWAP( header->intent_p3 );

		DO_SWAP( header->intent_code );
		DO_SWAP( header->datatype );
		DO_SWAP( header->bitpix );
		DO_SWAP( header->slice_start );

		DO_SWAP( header->vox_offset );
		DO_SWAP( header->scl_slope );
		DO_SWAP( header->scl_inter );
		DO_SWAP( header->slice_end );

		DO_SWAP( header->cal_max );
		DO_SWAP( header->cal_min );
		DO_SWAP( header->slice_duration );
		DO_SWAP( header->toffset );
		DO_SWAP( header->glmax );
		DO_SWAP( header->glmin );

		DO_SWAP( header->qform_code );
		DO_SWAP( header->sform_code );

		DO_SWAP( header->quatern_b );
		DO_SWAP( header->quatern_c );
		DO_SWAP( header->quatern_d );
		DO_SWAP( header->qoffset_x );
		DO_SWAP( header->qoffset_y );
		DO_SWAP( header->qoffset_z );

		DO_SWAPA( header->dim, 8 );
		DO_SWAPA( header->pixdim, 8 );
		DO_SWAPA( header->srow_x, 4 );
		DO_SWAPA( header->srow_y, 4 );
		DO_SWAPA( header->srow_z, 4 );

		return true;
	} else
		return false;

#undef DO_SWAP
#undef DO_SWAPA
}

std::list< data::Chunk > ImageFormat_NiftiSa::load(
	data::ByteArray source, 
	std::list<util::istring> /*formatstack*/, 
	std::list<util::istring> dialects, 
	std::shared_ptr<util::ProgressFeedback> /*feedback*/ 
) {

	//get the header - we use it directly from the file
	std::shared_ptr< _internal::nifti_1_header > header = std::static_pointer_cast<_internal::nifti_1_header>( source.getRawAddress() );
	const bool swap_endian = checkSwapEndian( header );

	if( header->sizeof_hdr < 348 ) {
		LOG( Runtime, warning ) << "sizeof_hdr of the file (" << header->vox_offset << ") is invalid, assuming 348";
		header->sizeof_hdr = 348;
	}

	if( header->vox_offset < 352 ) {
		LOG( Runtime, warning ) << "vox_offset of the file (" << header->vox_offset << ") is invalid, assuming 352";
		header->vox_offset = 352;
	}

	if( header->slice_duration < 0 ) {
		LOG( Runtime, warning ) << "ignoring invalid slice duration (" << header->slice_duration << ")";
		header->slice_duration = 0;
	}


	//set up the size - copy dim[0] values from dim[1]..dim[5]
	util::vector4<size_t> size;
	uint8_t tDims = 0;

	for( uint_fast8_t i = 1; i < 5; i++ ) {
		if( header->dim[i] <= 0 ) {
			LOG( Runtime, warning ) << "Resetting invalid dim[" << i << "] to 1";
			header->dim[i] = 1;
		}

		if( header->dim[i] > 1 )
			tDims = i;
	}

	LOG_IF( tDims != header->dim[0], Runtime, warning ) << "dim[0]==" << header->dim[0] << " doesn't fit the image, assuming " << ( int )tDims;
	header->dim[0] = tDims;

	std::copy(header->dim + 1, header->dim + 1 + 4, std::begin(size) );
	data::ValueArrayReference data_src;

	if( header->datatype == NIFTI_TYPE_BINARY ) { // image is binary encoded - needs special decoding
		data_src = bitRead( source.at<uint8_t>( header->vox_offset ), util::product(size) );
	} else if( checkDialect(dialects, "fsl") && header->datatype == NIFTI_TYPE_UINT8 && size[data::timeDim] == 3 ) { //if its fsl-three-volume-color copy the volumes
		LOG( Runtime, notice ) << "The image has 3 timesteps and its type is UINT8, assuming it is an fsl color image.";
		const size_t volume = util::product(size) / 3;
		data::ValueArray<util::color24> buff( volume );
		const data::ValueArray<uint8_t> src = source.at<uint8_t>( header->vox_offset, volume * 3 );
		LOG( Runtime, info ) << "Mapping nifti image as FSL RBG set of 3*" << volume << " elements";

		for( size_t v = 0; v < volume; v++ ) {
			buff[v].r = src[v];
			buff[v].g = src[v + volume];
			buff[v].b = src[v + volume * 2];
		}

		data_src = buff;
		size[data::timeDim] = 1;
	} else if( checkDialect(dialects, "fsl") && header->datatype == NIFTI_TYPE_FLOAT32 && size[data::timeDim] == 3 ) { //if its fsl-three-volume-vector copy the volumes
		LOG( Runtime, notice ) << "The image has 3 timesteps and its type is FLOAT32, assuming it is an fsl vector image.";
		const size_t volume = util::product(size) / 3;
		data::ValueArray<util::fvector3> buff( volume );
		const data::ValueArray<float> src = source.at<float>( header->vox_offset, util::product(size), swap_endian );

		for( size_t v = 0; v < volume; v++ ) {
			buff[v][0] = src[v];
			buff[v][1] = src[v + volume];
			buff[v][2] = src[v + volume * 2];
		}

		data_src = buff;
		size[data::timeDim] = 1;
	} else {
		unsigned int type = nifti_type2isis_type[header->datatype];

		if( type ) {
			data_src = source.atByID( type, header->vox_offset, util::product(size), swap_endian );

			if( swap_endian ) {
				LOG( Runtime, info ) << "Opened nifti image as endianess swapped " << data_src->getTypeName() << " of " << data_src->getLength()
									 << " elements (" << std::to_string(data_src->bytesPerElem()*data_src->getLength()*( 1. / 0x100000 ))+"M" <<")";
			} else {
				LOG( Runtime, info ) << "Mapped nifti image natively as " << data_src->getTypeName() << " of " << data_src->getLength()
				                     << " elements (" << std::to_string(data_src->bytesPerElem()*data_src->getLength()*( 1. / 0x100000 ))+"M" <<")";
			}

			LOG_IF( ( size_t )header->bitpix != data_src->bytesPerElem() * 8, Runtime, warning )
					<< "nifti field bitpix does not fit the bytesize of the given datatype (" << data_src->getTypeName() + "/" + std::to_string(header->bitpix) <<  ")";

		} else {
			LOG( Runtime, error ) << "Sorry, the nifti datatype " << header->datatype << " is not (yet) supported";
			throwGenericError( "unsupported datatype" );
		}
	}

	// create original chunk
	data::Chunk orig( data_src, size[0], size[1], size[2], size[3] );

	// check for extenstions and parse them
	data::ValueArray< uint8_t > extID = source.at<uint8_t>( header->sizeof_hdr, 4, swap_endian );
	_internal::DCMStack dcmmeta;

	if( extID[0] != 0 ) { // there is an extension http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/extension.html
		for( size_t pos = header->sizeof_hdr + 4; pos < header->vox_offset; ) {
			data::ValueArray<uint32_t> ext_hdr = source.at<uint32_t>( pos, 2, swap_endian );

			switch( ext_hdr[1] ) {
			case 0: // @todo for now we just assume its DcmMeta https://dcmstack.readthedocs.org/en/v0.6.1/DcmMeta_Extension.html
				dcmmeta.readJson( source.at<uint8_t>( header->sizeof_hdr + 4 + 8, ext_hdr[0] ), '.' );
				dcmmeta.print(std::cout,true);
				break;
			case 2:
				LOG( Runtime, warning ) << "sorry nifti extension for DICOM is not yet supported";
				break;
			case 4:
				LOG( Runtime, warning ) << "sorry nifti extension for AFNI is not yet supported";
				break;
			default:
				LOG( Runtime, warning ) << "sorry unknown nifti extension ID " << util::MSubject( ext_hdr[1] );
				break;
			}

			pos += ext_hdr[0];
		}
	}

	if( checkDialect(dialects, "withExtProtocols") ) { //find and remove MrPhoenixProtocol if not asked for explicitely
		for( util::PropertyMap::PropPath found = dcmmeta.find( "MrPhoenixProtocol", false, true );!found.empty();found = dcmmeta.find( "MrPhoenixProtocol", false, true ) )
			dcmmeta.remove( found );
	}


	//parse the header and add respective properties to the chunk
	data::scaling_pair scl;
	parseHeader( header, orig, scl );
	
	if(!scl.first.isEmpty() || !scl.second.isEmpty() ){
		LOG(Runtime,info) << "Applying scaling " << scl << " from the nifti header, result will be in double";
		orig.convertToType(data::ValueArray<double>::staticID(),scl);
	}
	dcmmeta.translateToISIS( orig );

	if(!orig.hasProperty( "acquisitionNumber"))//if dcmmeta didn't set slice ordering
		parseSliceOrdering( header, orig ); //get it from the header

	if( orig.hasBranch( "DICOM" ) ) // if we got DICOM data clean up some
		sanitise( orig );
	
	return std::list<data::Chunk>(1,orig);
}

std::unique_ptr<_internal::WriteOp > ImageFormat_NiftiSa::getWriteOp( const isis::data::Image &src, std::list<isis::util::istring> dialects )
{
	const size_t bpv = src.getMaxBytesPerVoxel() * 8;
	unsigned short target_id = src.getMajorTypeID(); //default to major type of the image

	//bitmap is not supportet by spm and fsl
	if( target_id == data::ValueArray<bool>::staticID() ) {
		if( checkDialect(dialects, "fsl") || checkDialect(dialects, "spm" ) ) {
			target_id = typeFallBack<bool, uint8_t>();// fall back to uint8_t and use normal writer for that
		} else {
			return std::unique_ptr<_internal::WriteOp>( new _internal::BitWriteOp( src ) ); // use special writer for bit
		}
	}

	// fsl cannot deal with some types
	if( checkDialect(dialects, "fsl") ) {
		switch( target_id ) {
		case data::ValueArray<uint16_t>::staticID():
			target_id = typeFallBack<uint16_t, int16_t>();
			break;
		case data::ValueArray<uint32_t>::staticID():
			target_id = typeFallBack<uint32_t, int32_t>();
			break;
		case data::ValueArray<util::color24>::staticID():

			if( src.getRelevantDims() > 3 ) {
				LOG( Runtime, error ) << "Cannot store color image of size " << src.getSizeAsString() << " using fsl dialect (4th dim is needed for the colors)";
				throwGenericError( "unsupported datatype" );
			} else {
				LOG( Runtime, info ) << data::ValueArray<util::color24>::staticName() <<  " is not supported by fsl falling back to color encoded in 4th dimension";
				return std::unique_ptr<_internal::WriteOp >( new _internal::FslRgbWriteOp( src ) );
			}

			break;
		}
	}

	// generic case (use generic scalar writer for the target_id)
	return std::unique_ptr<_internal::WriteOp >( new _internal::CommonWriteOp( src, target_id, bpv ) );
}


void ImageFormat_NiftiSa::write( const data::Image &img, const std::string &filename, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> /*progress*/ )
{
	data::Image image = img; //have a cheap copy, we're ging to do a lot of nasty things to the metadata

	const size_t voxel_offset = 352; // must be >=352 (and multiple of 16)  (http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/vox_offset.html)
	std::unique_ptr<_internal::WriteOp > writer = getWriteOp( image, dialects ); // get a fitting writer for the datatype
	const unsigned int nifti_id = isis_type2nifti_type[writer->getTypeId()]; // get the nifti datatype corresponding to our datatype

	if( nifti_id ) { // there is a corresponding nifti datatype

		// open/map the new file
		if( !writer->setOutput( filename, voxel_offset ) ) {
			if( errno ) {
				throwSystemError( errno, filename + " could not be opened" );
				errno = 0;
			} else
				throwGenericError( filename + " could not be opened" );
		}

		if( checkDialect(dialects, "spm" ) ) {
			writer->addFlip( math::mapScannerAxisToImageDimension(image, data::z ) );
		} else if( checkDialect(dialects, "fsl") ) {
			//dcm2nii flips the slice ordering of a mosaic if the determinant of the orientation is negative
			//don't ask, dcm2nii does it, fsl seems to expect it, so we do it
			auto image_type=image.queryProperty( "DICOM/ImageType" );
			if( image_type ) {
				const util::slist tp = image_type->as<util::slist>();
				const bool was_mosaic = ( std::find( tp.begin(), tp.end(), "WAS_MOSAIC" ) != tp.end() );
				const util::Matrix3x3<float> mat{
					image.getValueAs<util::fvector3>( "rowVec" ), 
					image.getValueAs<util::fvector3>( "columnVec" ), 
					image.getValueAs<util::fvector3>( "sliceVec" )
				};

				if( was_mosaic  && determinant( mat ) < 0 ) {
					LOG( Runtime, info ) << "Flipping slices of a siemens mosaic image for fsl compatibility";
					flipGeometry( image, data::sliceDim );
					writer->addFlip( data::sliceDim );
				}
			}

			//invert columnVec and flip the order of the images lines
			//well, you know ... don't ask ....
			LOG( Runtime, info ) << "Flipping columns of image for fsl compatibility";
			flipGeometry( image, data::columnDim );
			writer->addFlip( data::columnDim );
		}

		// if the image seems to have diffusion data, and we are writing for fsl we store the data just as dcm2nii does it
		if( checkDialect(dialects, "fsl") && image.getChunkAt( 0 ).hasProperty( "diffusionGradient" ) ) {
			LOG_IF( image.getNrOfTimesteps() < 2, Runtime, warning ) << "The image seems to have diffusion data, but has only one volume";
			std::ofstream bvecFile( ( makeBasename( filename ).first + ".bvec" ).c_str() );
			std::ofstream bvalFile( ( makeBasename( filename ).first + ".bval" ).c_str() );
			bvecFile.exceptions( std::ios::failbit | std::ios::badbit );
			bvalFile.exceptions( std::ios::failbit | std::ios::badbit );
			std::list<util::dvector3> bvecList;

			for( size_t i = 0; i < image.getNrOfTimesteps(); i++ ) { // go through all "volumes"
				const util::PropertyMap chunk = image.getChunk( 0, 0, 0, i );
				util::dvector3 gradient = chunk.getValueAs<util::dvector3>( "diffusionGradient" );
				const util::Matrix3x3<double> M{
					chunk.getValueAs<util::dvector3>( "rowVec" ),
					chunk.getValueAs<util::dvector3>( "columnVec" ),
					chunk.getValueAs<util::dvector3>( "sliceVec" )
				};

				// the bvalue is the length of the gradient direction,
				bvalFile << util::len(gradient) << " ";

				if( util::sqlen(gradient) > 0 ) {
					util::normalize(gradient);// the direction itself must be normalized
					bvecList.push_back( M * gradient ); // .. transformed into slice space and stored
				} else {
					bvecList.push_back( {0, 0, 0} );
				}
			}

			// the bvec file is the x-elements of all directions, then all y-elements and so on...
			bvecFile.precision( 14 );
			for( const util::dvector3 & dir :  bvecList )bvecFile << dir[0] << " ";
			bvecFile << std::endl;
			for( const util::dvector3 & dir :  bvecList )bvecFile << dir[1] << " ";
			bvecFile << std::endl;
			for( const util::dvector3 & dir :  bvecList )bvecFile << dir[2] << " ";
			bvecFile << std::endl;

			LOG( Runtime, notice ) << "Stored bvec information for fsl to " << makeBasename( filename ).first + ".bvec";
			LOG( Runtime, notice ) << "Stored bval information for fsl to " << makeBasename( filename ).first + ".bval";
		}


		// get the first 348 bytes as header
		_internal::nifti_1_header *header = writer->getHeader();
		header->datatype = nifti_id;

		guessSliceOrdering( image, header->slice_code, header->slice_duration );
		
		struct {float cal_min,cal_max,scl_slope,scl_inter;}scaling;
		
		scaling={
			image.getValueAsOr<float>("window/min",0),
			image.getValueAsOr<float>("window/max",0),
			image.getValueAsOr<float>("DICOM/RescaleSlope",1),
			image.getValueAsOr<float>("DICOM/RescaleIntercept",0)
		};

		if( image.getMajorTypeID() == data::ValueArray<util::color24>::staticID() ) {
			header->cal_min = 0;
			header->cal_max = 255;
		} else if (image.getMajorTypeID() == data::ValueArray<std::complex< double > >::staticID() || image.getMajorTypeID() == data::ValueArray<std::complex< float > >::staticID()){
			header->cal_min = 0;
			header->cal_max = 0;
		} else {
			if(image.hasProperty("window/max") && image.hasProperty("window/min")){
				header->cal_min = scaling.cal_min;
				header->cal_max = scaling.cal_min+(scaling.cal_max-scaling.cal_min)*scaling.scl_slope;
			} else { // todo this will store the min/max of the original image, not of the stored (converted) one
				const std::pair< float, float > minmax = image.getMinMaxAs<float>();
				header->cal_min = minmax.first;
				header->cal_max = minmax.second;
			}
		}

		{
			//join the properties of the first chunk and the image and store that to the header
			util::PropertyMap props = image;
			props.join( image.getChunkAt( 0, false ) );
			storeHeader( props, header );
		}
		
		if( image.hasProperty( "repetitionTime" ) )
			header->pixdim[data::timeDim + 1] = image.getValueAs<float>( "repetitionTime" );

		if( checkDialect(dialects, "spm") ) { // override "normal" description with the "spm-description"
			storeDescripForSPM( image.getChunk( 0, 0 ), header->descrip );
		} else {
			header->scl_slope = scaling.scl_slope;
			header->scl_inter = scaling.scl_inter;
		}

		// actually copy the data from each chunk of the image
		const_cast<data::Image &>( image ).foreachChunk( *writer ); // @todo we _do_ need a const version of foreachChunk/Voxel

	} else {
		LOG( Runtime, error ) << "Sorry, the datatype " << util::MSubject( image.getMajorTypeName() ) << " is not supportet for nifti output";
		throwGenericError( "unsupported datatype" );
	}

}

/// get the tranformation matrix from image space to Nifti space using row-,column and sliceVec from the given PropertyMap
util::Matrix4x4<double> ImageFormat_NiftiSa::getNiftiMatrix( const util::PropertyMap &props )
{
	util::dvector4 scale = props.getValueAs<util::dvector4>( "voxelSize" ); //used to put the scaling into the transformation
	const util::dvector4 offset = props.getValueAs<util::dvector4>( "indexOrigin" );

	if( props.hasProperty( "voxelGap" ) ) {
		const util::dvector4 gap = props.getValueAs<util::dvector4>( "voxelGap" );
		scale += gap; //nifti does not know about gaps, just add it to the voxel size
	}

	// the direction vectors should be normalized (says the isis-doc) but we get them in a higher precission than usual - so lets re-norm them
	util::dvector4 mat_rows[3];
	const char *row_names[] = {"rowVec", "columnVec", "sliceVec"};

	for( int i = 0; i < 3; i++ ) {
		mat_rows[i] = props.getValueAs<util::dvector4>( row_names[i] );
		util::normalize(mat_rows[i]);
	}

	util::Matrix4x4<double> image2isis = util::transpose(
		util::Matrix4x4<double>{
			mat_rows[data::rowDim] * scale[data::rowDim],
			mat_rows[data::columnDim] * scale[data::columnDim],
			mat_rows[data::sliceDim] * scale[data::sliceDim],
			offset
		}
	);// the columns of the transform matrix are the scaled row-, column-, sliceVec and the offset
	image2isis[3][3] = 1; // element 4/4 must be "1"

	return util::transpose(nifti2isis) * image2isis; // apply inverse transform from nifti to isis => return transformation from image to nifti space
}

void ImageFormat_NiftiSa::useSForm( util::PropertyMap &props )
{
	// srow_? is the linear map from image space to nifti space (not isis space)
	// [x_nii] [ nifti/srow_x ]   [i]
	// [x_nii]=[ nifti/srow_y ] * [j]
	// [x_nii] [ nifti/srow_z ]   [k]

	LOG( Debug, info ) << "Using sform (" << props.queryProperty( "nifti/sform_code" )->toString() << ") " << util::MSubject(
						   props.getValueAs<std::string>( "nifti/srow_x" ) + "-" +
						   props.getValueAs<std::string>( "nifti/srow_y" ) + "-" +
						   props.getValueAs<std::string>( "nifti/srow_z" )
					   ) << " to calc orientation";


	// transform from image space to nifti space
	const util::Matrix4x4<double> image2nifti{
		props.getValueAs<util::dvector4>( "nifti/srow_x" ),
		props.getValueAs<util::dvector4>( "nifti/srow_y" ),
		props.getValueAs<util::dvector4>( "nifti/srow_z" ),
		0, 0, 0, 0
	};
	auto image2isis = nifti2isis * image2nifti; // add transform to isis-space

	//get position of image-voxel 0,0,0,0 in isis space
	const auto origin = image2isis * util::dvector4{0, 0, 0, 1};
	props.setValueAs( "indexOrigin", util::dvector3{origin[0], origin[1], origin[2]} );
	LOG( Debug, info ) << "Computed indexOrigin=" << props.queryProperty( "indexOrigin" ) << " from sform";

	//remove offset from image2isis
	image2isis = util::Matrix4x4<double>{
		1, 0, 0, -origin[0],
		0, 1, 0, -origin[1],
		0, 0, 1, -origin[2],
		0, 0, 0, 0
	} * image2isis;

	const util::dvector3 voxelSize{ // get voxel sizes by transforming othogonal vectors of one voxel from image to isis
		util::len(image2isis * util::dvector4{1, 0, 0, 0}),
		util::len(image2isis * util::dvector4{0, 1, 0, 0}),
		util::len(image2isis * util::dvector4{0, 0, 1, 0})
	};

	props.setValueAs( "voxelSize", voxelSize );
	LOG( Debug, info ) << "Computed voxelSize=" << props.queryProperty( "voxelSize" ) << " from sform";


	//remove scaling from image2isis
	image2isis = image2isis * util::Matrix4x4<double>{
		1 / voxelSize[0], 0, 0, 0,
		0, 1 / voxelSize[1], 0, 0,
		0, 0, 1 / voxelSize[2], 0,
		0, 0, 0, 0
	};

	const auto r = util::transpose(image2isis);
	props.setValueAs( "rowVec",      util::dvector3{r[0][0], r[0][1], r[0][2]} );
	props.setValueAs( "columnVec",   util::dvector3{r[1][0], r[1][1], r[1][2]} );
	props.setValueAs( "sliceVec",    util::dvector3{r[2][0], r[2][1], r[2][2]} );

	LOG( Debug, info ) << "Computed rowVec=" << props.queryProperty( "rowVec" ) << ", "
					   << "columnVec=" << props.queryProperty( "columnVec" ) << " and "
					   << "sliceVec=" << props.queryProperty( "sliceVec" ) << " from sform";

	props.remove( "nifti/srow_x" );
	props.remove( "nifti/srow_y" );
	props.remove( "nifti/srow_z" );
}
void ImageFormat_NiftiSa::useQForm( util::PropertyMap &props )
{

	// orientation //////////////////////////////////////////////////////////////////////////////////
	//inspired by/stolen from nifticlib/nifti1_io.c:1466
	//see http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/quatern.html
	//and http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/qsform.html for qfac
	util::dvector4 quaternion{
		0,//a
		props.getValueAs<double>( "nifti/quatern_b" ),
		props.getValueAs<double>( "nifti/quatern_c" ),
		props.getValueAs<double>( "nifti/quatern_d" )
	};

	double &a = quaternion[0], &b = quaternion[1], &c = quaternion[2], &d = quaternion[3];

	if( 1 - util::sqlen(quaternion) < 1.e-7 ) { //if the quaternion is to "long"
		util::normalize(quaternion);      //normalize it and leave the angle as 0
	} else {
		a = sqrt( 1 - util::sqlen(quaternion) );                 /* angle = 2*arccos(a) */
	}

	LOG( Debug, info ) << "Using qform (" << props.queryProperty( "nifti/qform_code" )
					   << ") quaternion=" << util::vector4<double>{a, b, c, d} << " with qfac=" << props.queryProperty( "nifti/qfac" )
					   << ", pixdim=" << props.queryProperty( "nifti/pixdim" )
					   << " and qoffset= " << props.queryProperty( "nifti/qoffset" );

	const double a2 = a * a, b2 = b * b, c2 = c * c, d2 = d * d;
	const double _2ab = 2 * a * b, _2ac = 2 * a * c, _2ad = 2 * a * d;
	const double _2bc = 2 * b * c, _2bd = 2 * b * d;
	const double _2cd = 2 * c * d;

	const double r_11 = a2 + b2 - c2 - d2, r_12 = _2bc - _2ad,  r_13 = _2bd + _2ac;
	const double r_21 = _2bc + _2ad,  r_22 = a2 - b2 + c2 - d2, r_23 = _2cd - _2ab;
	const double r_31 = _2bd - _2ac,  r_32 = _2cd + _2ab,  r_33 = a2 - b2 - c2 + d2;
	const int qfac = props.getValueAs<float>( "nifti/qfac" );

	const util::Matrix4x4<double> image2nifti{
		r_11, r_12, r_13 * qfac, 0,
		r_21, r_22, r_23 * qfac, 0,
		r_31, r_32, r_33 * qfac, 0,
		   0,    0,           0, 0
	};

	LOG( Debug, info ) << "The matrix made from the qform is "
					   << util::vector3<double>{r_11, r_12, r_13 * qfac} << "-"
					   << util::vector3<double>{r_21, r_22, r_23 * qfac} << "-"
					   << util::vector3<double>{r_31, r_32, r_33 * qfac};

	const auto image2isis = nifti2isis * image2nifti;

	const auto r = util::transpose(image2isis);
	props.setValueAs( "rowVec",      util::dvector3{r[0][0], r[0][1], r[0][2]} );
	props.setValueAs( "columnVec",   util::dvector3{r[1][0], r[1][1], r[1][2]} );
	props.setValueAs( "sliceVec",    util::dvector3{r[2][0], r[2][1], r[2][2]} );

	LOG( Debug, info ) << "Computed rowVec=" << props.queryProperty( "rowVec" ) << ", "
					   << "columnVec=" << props.queryProperty( "columnVec" ) << " and "
					   << "sliceVec=" << props.queryProperty( "sliceVec" ) << " from qform";

	props.remove( "nifti/quatern_b" );
	props.remove( "nifti/quatern_c" );
	props.remove( "nifti/quatern_d" );
	props.remove( "nifti/qfac" );

	// indexOrigin //////////////////////////////////////////////////////////////////////////////////
	const util::fvector4 origin = nifti2isis * props.getValueAs<util::fvector4>( "nifti/qoffset" );
	props.setValueAs( "indexOrigin", util::fvector3{origin[0], origin[1], origin[2]} );
	LOG( Debug, info ) << "Computed indexOrigin=" << props.queryProperty( "indexOrigin" ) << " from qform";
	props.remove( "nifti/qoffset" );

	// use pixdim[1-3] as voxelSize //////////////////////////////////////////////////////////////////////////////////
	util::dlist vsize = props.getValueAs<util::dlist>( "nifti/pixdim" );
	vsize.resize( 3, 1 );
	props.setValueAs( "voxelSize", util::Value<util::dlist>( vsize ).as<util::fvector3>() );
	LOG( Debug, info ) << "Computed voxelSize=" << props.property( "voxelSize" ) << " from pixdim " << props.property( "nifti/pixdim" );
}
bool ImageFormat_NiftiSa::storeQForm( const util::PropertyMap &props, _internal::nifti_1_header *head )
{

	// take values of the 3x3 matrix == analog to the nifti reference implementation
	const isis::util::Matrix4x4< double > nifti2image = util::transpose(getNiftiMatrix( props )); //use the inverse of image2nifti to extract direction vectors easier

	util::fvector3 col[3];

	for( int i = 0; i < 3; i++ ) {
		const util::dvector4 buff = nifti2image[i];
		std::copy(buff.begin(), buff.begin() + 3, std::begin(col[i]) ); //nth column in image2nifti
		util::normalize(col[i]); // normalize the columns
	}

	// compute the determinant to determine if the transformation is proper
	if( determinant( { col[0], col[1], col[2] } ) > 0 ) {
		head->pixdim[0] = 1;
	} else { // improper => flip 3rd column
		col[2] = -col[2] ;
		head->pixdim[0] = -1;
	}

	if( !head->qform_code )head->qform_code = 1; // default to 1 if not set till now

	// the following was more or less stolen from the nifti reference implementation
	const float a_square = col[0][0] + col[1][1] + col[2][2] + 1;

	if( a_square > 0.5 ) { // simple case
		const float a = 0.5  * sqrt( a_square );
		head->quatern_b = 0.25 * ( col[1][2] - col[2][1] ) / a;
		head->quatern_c = 0.25 * ( col[2][0] - col[0][2] ) / a;
		head->quatern_d = 0.25 * ( col[0][1] - col[1][0] ) / a;
	} else {                       // trickier case
		float xd = 1.0 + col[0][0] - ( col[1][1] + col[2][2] ) ; /* 4*b*b */
		float yd = 1.0 + col[1][1] - ( col[0][0] + col[2][2] ) ; /* 4*c*c */
		float zd = 1.0 + col[2][2] - ( col[0][0] + col[1][1] ) ; /* 4*d*d */
		float a;

		if( xd > 1.0 ) {
			head->quatern_b = 0.5l * sqrt( xd ) ;
			head->quatern_c = 0.25l * ( col[1][0] + col[0][1] ) / head->quatern_b ;
			head->quatern_d = 0.25l * ( col[2][0] + col[0][2] ) / head->quatern_b ;
			a = 0.25l * ( col[1][2] - col[2][1] ) / head->quatern_b ;
		} else if( yd > 1.0 ) {
			head->quatern_c = 0.5l * sqrt( yd ) ;
			head->quatern_b = 0.25l * ( col[1][0] + col[0][1] ) / head->quatern_c ;
			head->quatern_d = 0.25l * ( col[2][1] + col[1][2] ) / head->quatern_c ;
			a = 0.25l * ( col[2][0] - col[0][2] ) / head->quatern_c ;
		} else {
			head->quatern_d = 0.5l * sqrt( zd ) ;
			head->quatern_b = 0.25l * ( col[2][0] + col[0][2] ) / head->quatern_d ;
			head->quatern_c = 0.25l * ( col[2][1] + col[1][2] ) / head->quatern_d ;
			a = 0.25 * ( col[0][1] - col[1][0] ) / head->quatern_d ;
		}

		if( a < 0.0 ) {
			head->quatern_b = -head->quatern_b ;
			head->quatern_c = -head->quatern_c ;
			head->quatern_d = -head->quatern_d;
		}
	}

	head->qoffset_x = nifti2image[3][0];
	head->qoffset_y = nifti2image[3][1];
	head->qoffset_z = nifti2image[3][2];

	return true;
}
void ImageFormat_NiftiSa::storeSForm( const util::PropertyMap &props, _internal::nifti_1_header *head )
{
	const util::Matrix4x4<double> sform = getNiftiMatrix( props );

	if( !head->sform_code )head->sform_code = 1; // default to 1 if not set till now

	_internal::copyArray2Mem(sform[0], head->srow_x );
	_internal::copyArray2Mem(sform[1], head->srow_y );
	_internal::copyArray2Mem(sform[2], head->srow_z );
}


void ImageFormat_NiftiSa::sanitise( data::Chunk &object )
{
	static const util::istring prefix( "DICOM/" );

	transformOrTell<uint16_t>         ( prefix + "SeriesNumber",            "sequenceNumber",     object, warning );

	transformIfNotSet<util::fvector3> ( prefix + "ImagePositionPatient",    "indexOrigin", object, warning );
	transformIfNotSet<float>          ( prefix + "EchoTime",                "echoTime", object, info );
	transformIfNotSet<uint16_t>       ( prefix + "PatientsAge",             "subjectAge",     object, info );
	transformIfNotSet<std::string>    ( prefix + "SeriesDescription",       "sequenceDescription", object, warning );
	transformIfNotSet<std::string>    ( prefix + "PatientsName",            "subjectName",        object, info );
	transformIfNotSet<util::date>     ( prefix + "PatientsBirthDate",       "subjectBirth",       object, info );
	transformIfNotSet<uint16_t>       ( prefix + "PatientsWeight",          "subjectWeigth",      object, info );
	transformIfNotSet<std::string>    ( prefix + "PerformingPhysiciansName", "performingPhysician", object, info );
	transformIfNotSet<uint16_t>       ( prefix + "NumberOfAverages",        "numberOfAverages",   object, warning );
	transformIfNotSet<uint32_t>       ( prefix + "CSAImageHeaderInfo/UsedChannelMask", "coilChannelMask", object, info );
	transformIfNotSet<int16_t>        ( prefix + "FlipAngle", "flipAngle", object, warning );

	if ( hasOrTell( prefix + "PatientsSex", object, info ) ) {
		util::Selection isisGender( "male,female,other" );
		bool set = false;

		switch ( object.getValueAs<std::string>( prefix + "PatientsSex" )[0] ) {
		case 'M':
			isisGender.set( "male" );
			set = true;
			break;
		case 'F':
			isisGender.set( "female" );
			set = true;
			break;
		case 'O':
			isisGender.set( "other" );
			set = true;
			break;
		default:
			LOG( Runtime, warning ) << "Dicom gender code " << util::MSubject( object.queryProperty( prefix + "PatientsSex" ) ) <<  " not known";
		}

		if( set ) {
			object.setValueAs( "subjectGender", isisGender );
			object.remove( prefix + "PatientsSex" );
		}
	}

}


// The nifti coord system:
// The (x,y,z) coordinates refer to the CENTER of a voxel.
// In methods 2 and 3, the (x,y,z) axes refer to a subject-based coordinate system, with +x = Right  +y = Anterior  +z = Superior (RAS).
// So, the transform from nifti to isis is:
const util::Matrix4x4<float> ImageFormat_NiftiSa::nifti2isis{
	-1, 0, 0, 0,
	 0,-1, 0, 0,
	 0, 0, 1, 0,
	 0, 0, 0, 1
};

// define form codes
// UNKNOWN=0      this is implizit as undef
// SCANNER_ANAT=1 scanner-based anatomical coordinates
// ALIGNED_ANAT=2 coordinates aligned to another file's, or to anatomical "truth".
// TALAIRACH    3 coordinates aligned to Talairach-Tournoux Atlas; (0,0,0)=AC, etc
// MNI_152      4 MNI 152 normalized coordinates
const util::Selection ImageFormat_NiftiSa::formCode( "SCANNER_ANAT,ALIGNED_ANAT,TALAIRACH,MNI_152" );

}
}


isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_NiftiSa();
}
