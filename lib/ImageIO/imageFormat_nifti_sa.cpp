#include <DataStorage/fileptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/type_traits/make_signed.hpp>
#include "imageFormat_nifti_sa.hpp"
#include <errno.h>


namespace isis
{
namespace image_io
{

namespace _internal
{
WriteOp::WriteOp( const data::Image &image, size_t bitsPerVoxel, bool doFlip ): data::_internal::NDimensional<4>( image ), m_doFlip( doFlip ), m_bpv( bitsPerVoxel )
{
	if( doFlip ) {
		data::Image dummy( image );
		flip_dim = dummy.mapScannerAxesToImageDimension( data::z );
	}
}
size_t WriteOp::getDataSize() {return getVolume() * m_bpv / 8;}

bool WriteOp::setOutput( const std::string &filename, size_t voxelstart )
{
	m_out = data::FilePtr( filename, voxelstart + getDataSize(), true );
	m_voxelstart = voxelstart;

	if( m_out.good() ) {
		nifti_1_header *header = getHeader();
		memset( header, 0, sizeof( _internal::nifti_1_header ) );

		// store the image size in dim and fill up the rest with "1" (to prevent fsl from exploding)
		header->dim[0] = getRelevantDims();
		getSizeAsVector().copyTo( header->dim + 1 );
		std::fill( header->dim + 5, header->dim + 8, 1 );

		header->sizeof_hdr = 348; // must be 348
		header->vox_offset = m_voxelstart;
		header->bitpix = m_bpv;
		return m_out.good();
	} else
		return false;
}

nifti_1_header *WriteOp::getHeader() {return reinterpret_cast<nifti_1_header *>( &m_out[0] );}

bool WriteOp::operator()( data::Chunk &ch, util::FixedVector< size_t, 4 > posInImage )
{
	if( doCopy( ch, posInImage ) )
		return true;
	else {
		LOG( Runtime, error ) << "Failed to copy chunk at " << posInImage;
		return true;
	}
}
class CommonWriteOp: public WriteOp
{
	const unsigned short m_targetId;
	const data::scaling_pair m_scale;
public:
	CommonWriteOp( const data::Image &image, unsigned short targetId, size_t bitsPerVoxel, bool doFlip = false ):
		WriteOp( image, bitsPerVoxel, doFlip ),
		m_targetId( targetId ), m_scale( image.getScalingTo( m_targetId ) ) {}

	bool doCopy( data::Chunk &ch, util::FixedVector< size_t, 4 > posInImage ) {
		size_t offset = m_voxelstart + getLinearIndex( posInImage ) * m_bpv / 8;
		data::ValuePtrReference out_data = m_out.atByID( m_targetId, offset, ch.getVolume() );
		ch.asValuePtrBase().copyTo( *out_data, m_scale );

		if( m_doFlip ) {
			// wrap the copied part back into a Chunk to flip it
			util::FixedVector< size_t, 4 > sz = ch.getSizeAsVector();
			data::Chunk cp( out_data, sz[data::rowDim], sz[data::columnDim], sz[data::sliceDim], sz[data::timeDim] ); // this is a cheap copy
			cp.swapAlong( flip_dim ); // .. so changing its data, will also change the data we just copied
		}

		return true;
	}

	short unsigned int getTypeId() {return m_targetId;}
};

class FslRgbWriteOp: public WriteOp
{
	const data::scaling_pair m_scale;
	struct VoxelCp: data::VoxelOp<util::color24> {
		int mode;
		uint8_t *ptr;
		virtual bool operator()( util::color24 &vox, const isis::util::FixedVector< size_t, 4 >& /*pos*/ ) {
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
		size_t dims[4];
		image.getSizeAsVector().copyTo( dims );
		dims[3] = 3;
		init( dims ); // reset our shape to use 3 timesteps as colors
	}

	bool doCopy( data::Chunk &src, util::FixedVector< size_t, 4 > posInImage ) {
		data::Chunk ch = src;
		ch.convertToType( data::ValuePtr<util::color24>::staticID, m_scale );
		VoxelCp cp;
		assert( posInImage[data::timeDim] == 0 );

		for( ; posInImage[data::timeDim] < 3; posInImage[data::timeDim]++ ) {
			const size_t offset = m_voxelstart + getLinearIndex( posInImage ) * m_bpv / 8;
			data::ValuePtr<uint8_t> out_data = m_out.at<uint8_t>( offset, ch.getVolume() );
			cp.ptr = &out_data[0];
			cp.mode = posInImage[data::timeDim];
			ch.foreachVoxel( cp );
			assert( cp.ptr == &out_data[0] + out_data.getLength() );
		}

		return true;
	}

	short unsigned int getTypeId() {return data::ValuePtr<uint8_t>::staticID;}
};


}

ImageFormat_NiftiSa::ImageFormat_NiftiSa()
{
	nifti_type2isis_type[NIFTI_TYPE_INT8 ] = data::ValuePtr< int8_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_INT16] = data::ValuePtr<int16_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_INT32] = data::ValuePtr<int32_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_INT64] = data::ValuePtr<int64_t>::staticID;

	nifti_type2isis_type[NIFTI_TYPE_UINT8 ] = data::ValuePtr< uint8_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_UINT16] = data::ValuePtr<uint16_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_UINT32] = data::ValuePtr<uint32_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_UINT64] = data::ValuePtr<uint64_t>::staticID;

	nifti_type2isis_type[NIFTI_TYPE_FLOAT32] = data::ValuePtr<float>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_FLOAT64] = data::ValuePtr<double>::staticID;

	nifti_type2isis_type[NIFTI_TYPE_RGB24] = data::ValuePtr<util::color24>::staticID;

	nifti_type2isis_type[NIFTI_TYPE_COMPLEX64] = data::ValuePtr<std::complex<float> >::staticID;
	nifti_type2isis_type[NIFTI_TYPE_COMPLEX128] = data::ValuePtr<std::complex<double> >::staticID;

	typedef std::map<short, unsigned short>::const_reference ref_type;
	BOOST_FOREACH( ref_type ref, nifti_type2isis_type ) {
		isis_type2nifti_type[ref.second] = ref.first;
	}

}
std::string ImageFormat_NiftiSa::suffixes( io_modes /*mode*/ )const {return std::string( ".nii" );}

void ImageFormat_NiftiSa::guessSliceOrdering( const data::Image img, char &slice_code, float &slice_duration )
{

	if( img.getChunk( 0, 0, 0, 0, false ).getRelevantDims() == img.getRelevantDims() ) { // seems like there is only one chunk - slice ordering doesnt matter - just choose NIFTI_SLICE_SEQ_INC
		slice_code = NIFTI_SLICE_SEQ_INC;
	} else {
		const util::PropertyValue first = img.getChunk( 0, 0, 0, 0, false ).propertyValue( "acquisitionNumber" ); // acquisitionNumber _must_ chunk-unique - so it is there even without a join
		const util::PropertyValue second = img.getChunk( 0, 0, 1, 0, false ).propertyValue( "acquisitionNumber" );
		const util::PropertyValue middle = img.getChunk( 0, 0, img.getSizeAsVector()[data::sliceDim] / 2 + .5, 0, false ).propertyValue( "acquisitionNumber" );

		if( first->gt( *second ) ) { // second slice has a lower number than the first => decrementing
			if( middle->gt( *second ) ) { // if the middle number is greater than the second its interleaved
				LOG( Runtime, info )
						<< "The \"middle\" acquisitionNumber (" << middle.toString() << ") is greater than the second (" << second.toString()
						<< ") assuming decrementing interleaved slice order";
				slice_code = NIFTI_SLICE_ALT_DEC;
			} else { // assume "normal" otherwise
				LOG( Runtime, info )
						<< "The first acquisitionNumber (" << first.toString() << ") is greater than the second (" << second.toString()
						<< ") assuming decrementing slice order";
				slice_code = NIFTI_SLICE_SEQ_DEC;
			}
		} else { // assume incrementing
			if( middle->lt( *second ) ) { // if the middle number is less than the second ist interleaved
				LOG( Runtime, info )
						<< "The \"middle\" acquisitionNumber (" << middle.toString() << ") is less than the second (" << second.toString()
						<< ") assuming incrementing interleaved slice order";
				slice_code = NIFTI_SLICE_ALT_INC;
			} else { // assume "normal" otherwise
				LOG( Runtime, info )
						<< "The first acquisitionNumber (" << first.toString() << ") is not greater than the second (" << second.toString()
						<< ") assuming incrementing slice order";
				slice_code = NIFTI_SLICE_SEQ_INC;
			}
		}

		slice_duration = fabs( second->as<float>() - second->as<float>() );

		if( slice_code == NIFTI_SLICE_SEQ_INC || slice_code == NIFTI_SLICE_SEQ_DEC ) { // if its interleaved there was another slice between 0 and 1
			slice_duration /= 2;
		}
	}

}

std::list<data::Chunk> ImageFormat_NiftiSa::parseSliceOrdering( const _internal::nifti_1_header *head, data::Chunk current )
{
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
	current.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
	const size_t dims = current.getRelevantDims();
	assert( dims <= 4 ); // more than 4 dimenstions are ... well, not expected

	if( head->slice_code == 0 || head->slice_code == NIFTI_SLICE_SEQ_INC ) {
		if( head->slice_duration == 0 ) { // and there is no slice duration, there is no use in numbering
			return std::list<data::Chunk>( 1, current );
		}
	}

	if( dims < 3 ) { // if there is only one slice, there is no use in numbering
		return std::list<data::Chunk>( 1, current );
	} else {// if there are timesteps we have to get a bit dirty
		std::list< data::Chunk > newChList = ( dims == 4 ? current.autoSplice() : std::list<data::Chunk>( 1, current ) ); // make sure we have a list of 3D-Chunks
		uint32_t offset = 0;

		BOOST_FOREACH( data::Chunk & ch, newChList ) {

			switch( head->slice_code ) { //set sub-property "acquisitionNumber" based on the slice_code and the offset
			case 0:
			case NIFTI_SLICE_SEQ_INC:

				for( uint32_t i = 0; i < ( uint32_t )head->dim[3]; i++ )
					ch.propertyValueAt( "acquisitionNumber", i ) = i + offset;

				break;
			case NIFTI_SLICE_SEQ_DEC:

				for( uint32_t i = 0; i < ( uint32_t )head->dim[3]; i++ )
					ch.propertyValueAt( "acquisitionNumber", head->dim[3] - i - 1 ) = i + offset;

				break;
			case NIFTI_SLICE_ALT_INC: {
				uint32_t i = 0, cnt;

				for( cnt = 0; i < floor( head->dim[3] / 2 + .5 ); i++, cnt += 2 )
					ch.propertyValueAt( "acquisitionNumber", i ) = cnt + offset;

				for( cnt = 1; i < ( uint32_t )head->dim[3]; i++, cnt += 2 )
					ch.propertyValueAt( "acquisitionNumber", i ) = cnt + offset;
			}
			break;
			case NIFTI_SLICE_ALT_DEC: {
				uint32_t i = 0, cnt;

				for( cnt = 0; i < floor( head->dim[3] / 2 + .5 ); i++, cnt += 2 )
					ch.propertyValueAt( "acquisitionNumber", head->dim[3] - i - 1 ) = cnt + offset;

				for( cnt = 1; i < ( uint32_t )head->dim[3]; i++, cnt += 2 )
					ch.propertyValueAt( "acquisitionNumber", head->dim[3] - i - 1 ) = cnt + offset;
			}
			break;
			default:
				LOG( Runtime, error ) << "Unknown slice code " << util::MSubject( head->slice_code );
				break;
			}

			if( head->slice_duration ) {
				for( uint32_t i = 0; i < ( uint32_t )head->dim[3]; i++ ) { // set su-property "acquisitionTime" based of the slice number
					ch.propertyValueAt( "acquisitionTime",  i ) = ch.propertyValueAt( "acquisitionNumber", i )->as<float>() * head->slice_duration * time_fac;
				}
			}

			offset += head->dim[3]; // increase offset by the number of slices per volume
		}
		return newChList;
	}
}


void ImageFormat_NiftiSa::storeDescripForSPM( const util::PropertyMap &props, char desc[] )
{
	std::list<std::string> ret;
	typedef const char *prop_pair[3];
	const prop_pair  pairs[] = {{"TR", "repetitionTime", "ms"}, {"TE", "echoTime", "ms"}, {"FA", "flipAngle", "deg"}, {"timestamp", "sequenceStart", ""}};
	BOOST_FOREACH( const prop_pair & p, pairs ) {
		if( props.hasProperty( p[1] ) ) {
			ret.push_back( std::string( p[0] ) + "=" + props.getPropertyAs<std::string>( p[1] ) + p[2] );
		}
	}
	strncpy( desc, util::listToString( ret.begin(), ret.end(), "/", "", "" ).c_str(), 80 );
}

bool ImageFormat_NiftiSa::parseDescripForSPM( isis::util::PropertyMap &props, const char desc[] )
{
	//check description for tr, te and fa and date which is written by spm8
	boost::regex descriptionRegex(
		".*TR=([[:digit:]]{1,})ms.*TE=([[:digit:]]{1,})ms.*FA=([[:digit:]]{1,})deg\\ *([[:digit:]]{1,2}).([[:word:]]{3}).([[:digit:]]{4})\\ *([[:digit:]]{1,2}):([[:digit:]]{1,2}):([[:digit:]]{1,2}).*"
	);
	boost::cmatch results;

	if ( boost::regex_match( desc, results,  descriptionRegex ) ) {
		props.propertyValue( "repetitionTime" ) = util::Value<uint16_t>( results.str( 1 ) );
		props.propertyValue( "echoTime" ) = util::Value<uint16_t>( results.str( 2 ) );
		props.propertyValue( "flipAngle" ) = util::Value<uint16_t>( results.str( 2 ) );

		const util::Value<int> day = results.str( 4 ), month = results.str( 5 ), year = results.str( 6 );
		const util::Value<uint8_t> hours = boost::lexical_cast<uint8_t>( results.str( 7 ) ), minutes = boost::lexical_cast<uint8_t>( results.str( 8 ) ), seconds = boost::lexical_cast<uint8_t>( results.str( 9 ) );

		boost::posix_time::ptime sequenceStart = boost::posix_time::ptime(
					boost::gregorian::date( ( int )year, ( int )month, ( int )day ),
					boost::posix_time::time_duration( hours, minutes, seconds )
				);
		props.setPropertyAs<boost::posix_time::ptime>( "sequenceStart", sequenceStart );

		LOG( Runtime, info )
				<< "Using Tr=" << props.propertyValue( "repetitionTime" ) << ", Te=" << props.propertyValue( "echoTime" )
				<< ", flipAngle=" << props.propertyValue( "flipAngle" ) << " and sequenceStart=" << props.propertyValue( "sequenceStart" )
				<< " from SPM8 description.";

		return true;
	} else
		return false;
}
void ImageFormat_NiftiSa::storeHeader( const util::PropertyMap &props, _internal::nifti_1_header *head )
{
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
		strncpy( head->descrip, props.getPropertyAs<std::string>( "sequenceDescription" ).c_str(), 80 );

	// store niftis original sform if its there
	if( props.hasProperty( "nifti/sform_code" ) ) {
		head->sform_code = props.getPropertyAs<util::Selection>( "nifti/sform_code" );

		if( props.hasProperty( "nifti/srow_x" ) && props.hasProperty( "nifti/srow_y" ) && props.hasProperty( "nifti/srow_z" ) ) {
			props.getPropertyAs<util::fvector4>( "nifti/srow_x" ).copyTo( head->srow_x );
			props.getPropertyAs<util::fvector4>( "nifti/srow_y" ).copyTo( head->srow_y );
			props.getPropertyAs<util::fvector4>( "nifti/srow_z" ).copyTo( head->srow_z );
		}
	}

	// store niftis original qform if its there
	if( props.hasProperty( "nifti/qform_code" ) ) {
		head->qform_code = props.getPropertyAs<util::Selection>( "nifti/qform_code" );

		if( props.hasProperty( "nifti/quatern_b" ) && props.hasProperty( "nifti/quatern_c" ) && props.hasProperty( "nifti/quatern_d" ) &&
			props.hasProperty( "nifti/qoffset" ) && props.hasProperty( "nifti/qfac" )
		  ) {
			const util::fvector4 offset = props.getPropertyAs<util::fvector4>( "nifti/qoffset" );
			head->quatern_b = props.getPropertyAs<float>( "nifti/quatern_b" );
			head->quatern_c = props.getPropertyAs<float>( "nifti/quatern_c" );
			head->quatern_d = props.getPropertyAs<float>( "nifti/quatern_d" );
			head->pixdim[0] = props.getPropertyAs<float>( "nifti/qfac" );
			head->qoffset_x = offset[0];
			head->qoffset_y = offset[1];
			head->qoffset_z = offset[2];
		}
	}

	//store current orientation (may override values set above)
	if( !storeQForm( props, head ) ) //try to encode as quaternion
		storeSForm( props, head ); //fall back to normal matrix

	strcpy( head->magic, "n+1" );
}
std::list< data::Chunk > ImageFormat_NiftiSa::parseHeader( const isis::image_io::_internal::nifti_1_header *head, isis::data::Chunk props )
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
	case NIFTI_UNITS_USEC:
		time_fac = 1e-3;
		break;
	}


	props.setPropertyAs<uint16_t>( "sequenceNumber", 0 );
	props.setPropertyAs<std::string>( "sequenceDescription", head->descrip );

	if( head->sform_code ) { // get srow if sform_code>0
		props.setPropertyAs( "nifti/sform_code", formCode )->castTo<util::Selection>().set( head->sform_code );
		props.setPropertyAs( "nifti/srow_x", util::fvector4() )->castTo<util::fvector4>().copyFrom( head->srow_x, head->srow_x + 4 );;
		props.setPropertyAs( "nifti/srow_y", util::fvector4() )->castTo<util::fvector4>().copyFrom( head->srow_y, head->srow_y + 4 );;
		props.setPropertyAs( "nifti/srow_z", util::fvector4() )->castTo<util::fvector4>().copyFrom( head->srow_z, head->srow_z + 4 );;
	}

	if( head->qform_code ) { // get the quaternion if qform_code>0
		props.setPropertyAs( "nifti/qform_code", formCode )->castTo<util::Selection>().set( head->qform_code );
		props.setPropertyAs( "nifti/quatern_b", head->quatern_b );
		props.setPropertyAs( "nifti/quatern_c", head->quatern_c );
		props.setPropertyAs( "nifti/quatern_d", head->quatern_d );
		props.setPropertyAs( "nifti/qoffset", util::fvector4( head->qoffset_x, head->qoffset_y, head->qoffset_z, 0 ) );
		props.setPropertyAs( "nifti/qfac", ( head->dim[0] == -1 ) ? : 1 );

		// voxel size
		util::fvector4 v_size;
		v_size.copyFrom( head->pixdim + 1, head->pixdim + std::min<unsigned short>( dims, 3 ) + 1 );
		props.setPropertyAs<util::fvector4>( "nifti/pixdim", v_size * size_fac );
	}

	if( head->sform_code ) { // if sform_code is set, use that regardless of qform
		useSForm( props );
	} else if( head->qform_code ) { // if qform_code is set, but no sform use that (thats the "normal" case)
		useQForm( props );
	} else {
		LOG( Runtime, warning ) << "Neither sform_code nor qform_code are set, using identity matrix for geometry";
		props.setPropertyAs<util::fvector4>( "rowVec",    nifti2isis.getRow( 0 ) ); // we use the transformation from nifti to isis as unity
		props.setPropertyAs<util::fvector4>( "columnVec", nifti2isis.getRow( 1 ) ); // because the image will very likely be in nifti space
		props.setPropertyAs<util::fvector4>( "sliceVec",  nifti2isis.getRow( 2 ) );
		props.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4( head->pixdim[1], head->pixdim[2], head->pixdim[3] ) );
	}

	// set space unit factors
	props.propertyValue( "voxelSize" )->castTo<util::fvector4>() *= size_fac;
	props.propertyValue( "indexOrigin" )->castTo<util::fvector4>() *= size_fac;

	// Tr
	if( head->pixdim[dims] != 0 ) // if pixdim is given for the uppermost dim, assume its repetitionTime
		props.setPropertyAs<uint16_t>( "repetitionTime", head->pixdim[dims]*time_fac );

	// sequenceDescription
	if( !parseDescripForSPM( props, head->descrip ) ) // if descrip dos not hold Te,Tr and stuff (SPM dialect)
		props.setPropertyAs<std::string>( "sequenceDescription", head->descrip ); // use it the usual way

	// TODO: at the moment scaling not supported due to data type changes
	if ( !head->scl_slope == 0 && !( head->scl_slope == 1 || head->scl_inter == 0 ) ) {
		//          throwGenericError( std::string( "Scaling is not supported at the moment. Scale Factor: " ) + util::Value<float>( scale ).toString() );
		LOG( Runtime, error ) << "Scaling is not supported at the moment.";
	}

	return parseSliceOrdering( head, props );
}

std::string ImageFormat_NiftiSa::getName()const {return "Nifti standalone";}

int ImageFormat_NiftiSa::load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & )
{
	data::FilePtr mfile( filename );

	if( !mfile.good() ) {
		if( errno ) {
			throwSystemError( errno, filename + " could not be opened" );
			errno = 0;
		} else
			throwGenericError( filename + " could not be opened" );
	}

	//get the header - we use it directly from the file
	const _internal::nifti_1_header *header = reinterpret_cast<const _internal::nifti_1_header *>( &mfile[0] );

	if( header->intent_code != 0 ) {
		throwGenericError( std::string( "only intent_code==0 is supportet" ) );
	}

	//set up the size - copy dim[0] values from dim[1]..dim[dim[0]]
	util::FixedVector<size_t, 4> size;
	size.fill( 1 );

	size.copyFrom( header->dim + 1, header->dim + 1 + header->dim[0] );
	data::ValuePtrReference data_src;

	if( util::istring( "fsl" ) == dialect.c_str() && header->datatype == NIFTI_TYPE_UINT8 && size[data::timeDim] == 3 ) { //if its fsl-three-volume-color copy the volumes
		LOG( Runtime, notice ) << "The image has 3 timesteps and its type is UINT8, assuming it is an fsl color image.";
		const size_t volume = size.product() / 3;
		data::ValuePtr<util::color24> buff( volume );
		const data::ValuePtr<uint8_t> src = mfile.at<uint8_t>( header->vox_offset );

		for( size_t v = 0; v < volume; v++ ) {
			buff[v].r = src[v];
			buff[v].g = src[v + volume];
			buff[v].b = src[v + volume * 2];
		}

		data_src = buff;
		size[data::timeDim] = 1;
	} else {
		data_src = mfile.atByID( nifti_type2isis_type[header->datatype], header->vox_offset );
		LOG( Runtime, info ) << "Mapping nifti image as " << data_src->getTypeName() << " of length " << data_src->getLength();
		LOG_IF( ( size_t )header->bitpix != data_src->bytesPerElem() * 8, Runtime, warning )
				<< "nifti field bitpix does not fit the bytesize of the given datatype (" << data_src->getTypeName() << "/" << header->bitpix <<  ")";
	}

	std::list<data::Chunk> newChunks = parseHeader( header, data::Chunk( data_src, size[0], size[1], size[2], size[3] ) );
	chunks.insert( chunks.begin(), newChunks.begin(), newChunks.end() );
	return newChunks.size();
}

std::auto_ptr< _internal::WriteOp > ImageFormat_NiftiSa::getWriteOp( const isis::data::Image &src, isis::util::istring dialect )
{
	const size_t bpv = src.getBytesPerVoxel() * 8;
	const unsigned short target_id = src.getMajorTypeID();

	// fsl cannot deal with some types
	if( dialect == "fsl" ) {
		switch( target_id ) {
		case data::ValuePtr<uint16_t>::staticID:
			return std::auto_ptr<_internal::WriteOp>( new _internal::CommonWriteOp( src, typeFallBack<uint16_t>(), bpv, false ) );
			break;
		case data::ValuePtr<uint32_t>::staticID:
			return std::auto_ptr<_internal::WriteOp>( new _internal::CommonWriteOp( src, typeFallBack<uint32_t>(), bpv, false ) );
			break;
		case data::ValuePtr<util::color24>::staticID:

			if( src.getRelevantDims() > 3 ) {
				LOG( Runtime, error ) << "Cannot store color image of size " << src.getSizeAsString() << " using fsl dialect (4th dim is needed for the colors)";
				throwGenericError( "unsupported datatype" );
			} else {
				LOG( Runtime, info ) << data::ValuePtr<util::color24>::staticName() <<  " is not supported by fsl falling back to color encoded in 4th dimension";
				return std::auto_ptr<_internal::WriteOp>( new _internal::FslRgbWriteOp( src ) );
			}

			break;
		}
	}

	return std::auto_ptr<_internal::WriteOp>( new _internal::CommonWriteOp( src, target_id, bpv, ( dialect == "spm" ) ) ); // default case (no fsl dialect or type can be used directly)
}


void ImageFormat_NiftiSa::write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & )
{
	const size_t voxel_offset = 352; // must be >=352 (and multiple of 16)  (http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/vox_offset.html)
	std::auto_ptr< _internal::WriteOp > writer = getWriteOp( image, dialect.c_str() ); // get a fitting writer for the datatype
	const unsigned int target_id = isis_type2nifti_type[writer->getTypeId()]; // get the nifti datatype corresponding to our datatype

	if( target_id ) { // there is a corresponding nifti datatype

		// open/map the new file
		if( !writer->setOutput( filename, voxel_offset ) ) {
			if( errno ) {
				throwSystemError( errno, filename + " could not be opened" );
				errno = 0;
			} else
				throwGenericError( filename + " could not be opened" );
		}


		// get the first 348 bytes as header
		_internal::nifti_1_header *header = writer->getHeader();
		header->datatype = target_id;

		guessSliceOrdering( image, header->slice_code, header->slice_duration );

		if( image.getMajorTypeID() == data::ValuePtr<util::color24>::staticID ) {
			header->cal_min = 0;
			header->cal_max = 255;
		} else {
			const std::pair< float, float > minmax = image.getMinMaxAs<float>();
			header->cal_min = minmax.first;
			header->cal_max = minmax.second;
		}

		storeHeader( image.getChunk( 0, 0 ), header ); // store properties of the "lowest" chunk merged with the image's properties into the header

		if( image.getSizeAsVector()[data::timeDim] > 1 && image.hasProperty( "repetitionTime" ) )
			header->pixdim[data::timeDim + 1] = image.getPropertyAs<float>( "repetitionTime" );

		if( util::istring( dialect.c_str() ) == "spm" ) { // override "normal" description with the "spm-description"
			storeDescripForSPM( image.getChunk( 0, 0 ), header->descrip );
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
	util::dvector4 scale = props.getPropertyAs<util::dvector4>( "voxelSize" ); //used to put the scaling into the transformation
	util::dvector4 offset = props.getPropertyAs<util::dvector4>( "indexOrigin" );

	if( props.hasProperty( "voxelGap" ) ) {
		const util::dvector4 gap = props.getPropertyAs<util::dvector4>( "voxelGap" );
		scale += gap; //nifti does not know about gaps, just add it to the voxel size
	}

	// the direction vectors should be normalized (says the isis-doc) but we get them in a higher precission than usual - so lets re-norm them
	util::dvector4 mat_rows[3];
	const char *row_names[] = {"rowVec", "columnVec", "sliceVec"};

	for( int i = 0; i < 3; i++ ) {
		mat_rows[i] = props.getPropertyAs<util::dvector4>( row_names[i] );
		mat_rows[i].norm();
	}

	util::Matrix4x4<double> image2isis = util::Matrix4x4<double>(
			mat_rows[data::rowDim] * scale[data::rowDim],
			mat_rows[data::columnDim] * scale[data::columnDim],
			mat_rows[data::sliceDim] * scale[data::sliceDim],
			props.getPropertyAs<util::dvector4>( "indexOrigin" )
										 ).transpose();// the columns of the transform matrix are the scaled row-, column-, sliceVec and the offset
	image2isis.elem( 3, 3 ) = 1; // element 4/4 must be "1"

	return nifti2isis.transpose().dot( image2isis ); // apply inverse transform from nifti to isis => return transformation from image to nifti space
}

void ImageFormat_NiftiSa::useSForm( util::PropertyMap &props )
{
	// srow_? is the linear map from image space to nifti space (not isis space)
	// [x] [ nifti/srow_x ]   [i]
	// [y]=[ nifti/srow_y ] * [j]
	// [z] [ nifti/srow_z ]   [k]

	LOG( Debug, info ) << "Using sform (" << props.propertyValue( "nifti/sform_code" ).toString() << ") " << util::MSubject(
						   props.propertyValue( "nifti/srow_x" ).toString() + "-" +
						   props.propertyValue( "nifti/srow_y" ).toString() + "-" +
						   props.propertyValue( "nifti/srow_z" ).toString()
					   ) << " to calc orientation";


	// transform from image space to nifti space
	const util::Matrix4x4<float> image2nifti(
		props.getPropertyAs<util::fvector4>( "nifti/srow_x" ),
		props.getPropertyAs<util::fvector4>( "nifti/srow_y" ),
		props.getPropertyAs<util::fvector4>( "nifti/srow_z" )
	);
	util::Matrix4x4<float> image2isis = nifti2isis.dot( image2nifti ); // add transform to isis-space

	//get position of image-voxel 0,0,0,0 in isis space
	const util::fvector4 origin = image2isis.dot( util::fvector4( 0, 0, 0, 1 ) );
	props.setPropertyAs<util::fvector4>( "indexOrigin", origin )->castTo<util::fvector4>()[data::timeDim] = 0; // timedim is 1 from the matrix calc
	LOG( Debug, info ) << "Computed indexOrigin=" << props.getPropertyAs<util::fvector4>( "indexOrigin" ) << " from sform";

	//remove offset from image2isis
	image2isis = util::Matrix4x4<float>(
					 util::fvector4( 1, 0, 0, -origin[0] ),
					 util::fvector4( 0, 1, 0, -origin[1] ),
					 util::fvector4( 0, 0, 1, -origin[2] )
				 ).dot( image2isis );

	const util::fvector4 voxelSize( // get voxel sizes by transforming othogonal vectors of one voxel from image to isis
		image2isis.dot( util::fvector4( 1, 0, 0 ) ).len(),
		image2isis.dot( util::fvector4( 0, 1, 0 ) ).len(),
		image2isis.dot( util::fvector4( 0, 0, 1 ) ).len()
	);
	props.setPropertyAs<util::fvector4>( "voxelSize", voxelSize )->castTo<util::fvector4>()[data::timeDim] = 0; // timedim is 1 from the matrix calc
	LOG( Debug, info ) << "Computed voxelSize=" << props.getPropertyAs<util::fvector4>( "voxelSize" ) << " from sform";


	//remove scaling from image2isis
	image2isis = image2isis.dot( util::Matrix4x4<float>(
									 util::fvector4( 1 / voxelSize[0], 0, 0 ),
									 util::fvector4( 0, 1 / voxelSize[1], 0 ),
									 util::fvector4( 0, 0, 1 / voxelSize[2] )
								 ) );

	props.setPropertyAs<util::fvector4>( "rowVec", image2isis.transpose().getRow( 0 ) );
	props.setPropertyAs<util::fvector4>( "columnVec", image2isis.transpose().getRow( 1 ) );
	props.setPropertyAs<util::fvector4>( "sliceVec", image2isis.transpose().getRow( 2 ) );

	LOG( Debug, info )
			<< "Computed rowVec=" << props.getPropertyAs<util::fvector4>( "rowVec" ) << ", "
			<< "columnVec=" << props.getPropertyAs<util::fvector4>( "columnVec" ) << " and "
			<< "sliceVec=" << props.getPropertyAs<util::fvector4>( "sliceVec" ) << " from sform";

	props.remove( "nifti/srow_x" );
	props.remove( "nifti/srow_y" );
	props.remove( "nifti/srow_z" );
}
void ImageFormat_NiftiSa::useQForm( util::PropertyMap &props )
{

	// orientation //////////////////////////////////////////////////////////////////////////////////
	//see http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/quatern.html
	//and http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/qformExt.jpg
	double b = props.getPropertyAs<double>( "nifti/quatern_b" );
	double c = props.getPropertyAs<double>( "nifti/quatern_c" );
	double d = props.getPropertyAs<double>( "nifti/quatern_d" );
	double a = sqrt( 1.0 - ( b * b + c * c + d * d ) );

	LOG( Debug, info )
			<< "Using qform (" << props.propertyValue( "nifti/qform_code" ).toString()
			<< ") quaternion=" << util::fvector4( a, b, c, d ) << " with qfac=" << props.propertyValue( "nifti/qfac" ).toString()
			<< ", pixdim=" << props.propertyValue( "nifti/pixdim" ).toString()
			<< " and qoffset= " << props.propertyValue( "nifti/qoffset" ).toString();

	const util::Matrix4x4<double> M(
		util::fvector4( a * a + b * b - c * c - d * d, 2 * b * c - 2 * a * d, 2 * b * d + 2 * a * c ),
		util::fvector4( 2 * b * c + 2 * a * d, a * a + c * c - b * b - d * d, 2 * c * d - 2 * a * b ),
		util::fvector4( 2 * b * d - 2 * a * c, 2 * c * d + 2 * a * b, a * a + d * d - c * c - b * b )
	);
	const util::Matrix4x4<double> image2isis = nifti2isis.dot( M );

	props.setPropertyAs<util::fvector4>( "rowVec", image2isis.transpose().getRow( 0 ) );
	props.setPropertyAs<util::fvector4>( "columnVec", image2isis.transpose().getRow( 1 ) );
	props.setPropertyAs<util::fvector4>( "sliceVec", image2isis.transpose().getRow( 2 ) );

	LOG( Debug, info )
			<< "Computed rowVec=" << props.getPropertyAs<util::fvector4>( "rowVec" ) << ", "
			<< "columnVec=" << props.getPropertyAs<util::fvector4>( "columnVec" ) << " and "
			<< "sliceVec=" << props.getPropertyAs<util::fvector4>( "sliceVec" ) << " from qform";

	props.remove( "nifti/quatern_b" );
	props.remove( "nifti/quatern_c" );
	props.remove( "nifti/quatern_d" );
	props.remove( "nifti/qfac" );

	// indexOrigin //////////////////////////////////////////////////////////////////////////////////
	props.setPropertyAs<util::fvector4>( "indexOrigin", nifti2isis.dot( props.getPropertyAs<util::fvector4>( "nifti/qoffset" ) ) );
	LOG( Debug, info ) << "Computed indexOrigin=" << props.getPropertyAs<util::fvector4>( "indexOrigin" ) << " from qform";
	props.remove( "nifti/qoffset" );

	// voxelSize //////////////////////////////////////////////////////////////////////////////////
	props.transform<util::fvector4>( "nifti/pixdim", "voxelSize" );
	LOG( Debug, info ) << "Computed voxelSize=" << props.getPropertyAs<util::fvector4>( "voxelSize" ) << " from qform";
}
bool ImageFormat_NiftiSa::storeQForm( const util::PropertyMap &props, _internal::nifti_1_header *head )
{

	// take values of the 3x3 matrix == analog to the nifti reference implementation
	const isis::util::Matrix4x4< double > nifti2image = getNiftiMatrix( props ).transpose(); //use the inverse of image2nifti to extract direction vectors easier

	util::fvector4 col[3];

	for( int i = 0; i < 3; i++ ) {
		col[i] = nifti2image.getRow( i ); //nth column in image2nifti
		head->pixdim[i + 1] = col[i].len(); //store voxel size (don't use voxelSize, thats without voxelGap)
		col[i].norm(); // normalize the columns
	}

	// compute the determinant to determine if the transformation is proper
	const float determinant =
		col[0][0] * col[1][1] * col[2][2] - col[0][0] * col[1][2] * col[2][1] - col[0][1] * col[1][0] * col[2][2] +
		col[0][1] * col[1][2] * col[2][0] + col[0][2] * col[1][0] * col[2][1] - col[0][2] * col[1][1] * col[2][0];

	if( determinant > 0 ) {
		head->pixdim[0] = 1;
	} else { // improper => flip 3rd column
		col[2][0] = -col[2][0] ;
		col[2][1] = -col[2][1] ;
		col[2][2] = -col[2][2] ;
		head->pixdim[0] = -1;
	}

	util::Selection tformCode( formCode );
	tformCode.set( "SCANNER_ANAT" );
	head->qform_code = tformCode;
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

	head->qoffset_x = nifti2image.elem( 0, 3 );
	head->qoffset_y = nifti2image.elem( 1, 3 );
	head->qoffset_z = nifti2image.elem( 2, 3 );

	return true;
}
void ImageFormat_NiftiSa::storeSForm( const util::PropertyMap &props, _internal::nifti_1_header *head )
{
	const util::Matrix4x4<double> sform = getNiftiMatrix( props );
	head->sform_code = 1;
	sform.getRow( 0 ).copyTo( head->srow_x );
	sform.getRow( 1 ).copyTo( head->srow_y );
	sform.getRow( 2 ).copyTo( head->srow_z );
}

// The nifti coord system:
// The (x,y,z) coordinates refer to the CENTER of a voxel.
// In methods 2 and 3, the (x,y,z) axes refer to a subject-based coordinate system, with +x = Right  +y = Anterior  +z = Superior.
// So, the transform from nifti to isis is:
const util::Matrix4x4<short> ImageFormat_NiftiSa::nifti2isis(
	util::vector4<short>( -1, 0, 0, 0 ),
	util::vector4<short>( 0, -1, 0, 0 ),
	util::vector4<short>( 0, 0, 1, 0 ),
	util::vector4<short>( 0, 0, 0, 1 )
);

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
