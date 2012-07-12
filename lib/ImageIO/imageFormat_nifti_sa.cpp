#include <DataStorage/fileptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
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
		flip_dim = dummy.mapScannerAxisToImageDimension( data::z );
	}
}
size_t WriteOp::getDataSize()
{
	size_t bitsize = getVolume() * m_bpv;

	if( bitsize % 8 )
		bitsize += 8;

	return bitsize / 8;
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
		getSizeAsVector().copyTo( header->dim + 1 );
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
void WriteOp::applyFlip ( isis::data::ValueArrayReference dat, isis::util::vector4< size_t > chunkSize )
{
	if( m_doFlip ) {
		// wrap the copied part back into a Chunk to flip it
		data::Chunk cp( dat, chunkSize[data::rowDim], chunkSize[data::columnDim], chunkSize[data::sliceDim], chunkSize[data::timeDim] ); // this is a cheap copy
		cp.swapAlong( flip_dim ); // .. so changing its data, will also change the data we just copied
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

	bool doCopy( data::Chunk &ch, util::vector4<size_t> posInImage ) {
		size_t offset = m_voxelstart + getLinearIndex( posInImage ) * m_bpv / 8;
		data::ValueArrayReference out_data = m_out.atByID( m_targetId, offset, ch.getVolume() );
		ch.asValueArrayBase().copyTo( *out_data, m_scale );
		applyFlip( out_data, ch.getSizeAsVector() );
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
		size_t dims[4];
		image.getSizeAsVector().copyTo( dims );
		dims[3] = 3;
		init( dims ); // reset our shape to use 3 timesteps as colors
	}

	bool doCopy( data::Chunk &src, util::vector4<size_t> posInImage ) {
		data::Chunk ch = src;
		ch.convertToType( data::ValueArray<util::color24>::staticID, m_scale );
		VoxelCp cp;
		assert( posInImage[data::timeDim] == 0 );

		for( ; posInImage[data::timeDim] < 3; posInImage[data::timeDim]++ ) { //copy each color/timestep into m_out
			const size_t offset = m_voxelstart + getLinearIndex( posInImage ) * m_bpv / 8;
			data::ValueArray<uint8_t> out_data = m_out.at<uint8_t>( offset, ch.getVolume() );
			cp.ptr = &out_data[0];
			cp.mode = posInImage[data::timeDim];
			ch.foreachVoxel( cp );
			assert( cp.ptr == &out_data[0] + out_data.getLength() );
		}

		return true;
	}

	short unsigned int getTypeId() {return data::ValueArray<uint8_t>::staticID;}
};

class BitWriteOp: public WriteOp
{
public:
	BitWriteOp( const data::Image &image ): WriteOp( image, 1 ) {}

	bool doCopy( data::Chunk &src, util::vector4<size_t> posInImage ) {
		data::ValueArray<bool> in_data = src.asValueArrayBase().as<bool>();
		const size_t offset = m_voxelstart + getLinearIndex( posInImage ) * m_bpv ;

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

	short unsigned int getTypeId() {return data::ValueArray<bool>::staticID;}
};


}

ImageFormat_NiftiSa::ImageFormat_NiftiSa()
{
	nifti_type2isis_type[NIFTI_TYPE_INT8 ] = data::ValueArray< int8_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_INT16] = data::ValueArray<int16_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_INT32] = data::ValueArray<int32_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_INT64] = data::ValueArray<int64_t>::staticID;

	nifti_type2isis_type[NIFTI_TYPE_UINT8 ] = data::ValueArray< uint8_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_UINT16] = data::ValueArray<uint16_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_UINT32] = data::ValueArray<uint32_t>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_UINT64] = data::ValueArray<uint64_t>::staticID;

	nifti_type2isis_type[NIFTI_TYPE_FLOAT32] = data::ValueArray<float>::staticID;
	nifti_type2isis_type[NIFTI_TYPE_FLOAT64] = data::ValueArray<double>::staticID;

	nifti_type2isis_type[NIFTI_TYPE_RGB24] = data::ValueArray<util::color24>::staticID;

	nifti_type2isis_type[NIFTI_TYPE_COMPLEX64] = data::ValueArray<std::complex<float> >::staticID;
	nifti_type2isis_type[NIFTI_TYPE_COMPLEX128] = data::ValueArray<std::complex<double> >::staticID;

	nifti_type2isis_type[NIFTI_TYPE_BINARY] = data::ValueArray<bool>::staticID;

	typedef std::map<short, unsigned short>::const_reference ref_type;
	BOOST_FOREACH( ref_type ref, nifti_type2isis_type ) {
		isis_type2nifti_type[ref.second] = ref.first;
	}

}
util::istring ImageFormat_NiftiSa::suffixes( io_modes /*mode*/ )const {return ".nii";}

void ImageFormat_NiftiSa::guessSliceOrdering( const data::Image img, char &slice_code, float &slice_duration )
{

	if( img.getChunk( 0, 0, 0, 0, false ).getRelevantDims() == img.getRelevantDims() ) { // seems like there is only one chunk - slice ordering doesnt matter - just choose NIFTI_SLICE_SEQ_INC
		slice_code = NIFTI_SLICE_SEQ_INC;
	} else {
		util::PropertyMap::PropPath order = img.getChunk( 0, 0, 0, 0, false ).hasProperty( "acquisitionTime" ) ? "acquisitionTime" : "acquisitionNumber";
		const util::PropertyValue first = img.getChunk( 0, 0, 0, 0, false ).propertyValue( order ); // acquisitionNumber _must_ be chunk-unique - so it is there even without a join
		const util::PropertyValue second = img.getChunk( 0, 0, 1, 0, false ).propertyValue( order );
		const util::PropertyValue middle = img.getChunk( 0, 0, img.getSizeAsVector()[data::sliceDim] / 2 + .5, 0, false ).propertyValue( order );

		if( ( *first ).gt( *second ) ) { // second slice has a lower number than the first => decrementing
			if( ( *middle ).gt( *second ) ) { // if the middle number is greater than the second its interleaved
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
			if( ( *middle ).lt( *second ) ) { // if the middle number is less than the second ist interleaved
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

		slice_duration = fabs( second.as<float>() - second.as<float>() );

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
		// make sure we have a list of 3D-Chunks (acquisitionNumberStride doesn't matter, we will reset it anyway)
		std::list< data::Chunk > newChList = ( dims == 4 ? current.autoSplice(1) : std::list<data::Chunk>( 1, current ) ); 
		
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
					ch.propertyValueAt( "acquisitionTime",  i ) = ch.propertyValueAt( "acquisitionNumber", i ).as<float>() * head->slice_duration * time_fac;
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
		props.propertyValue( "flipAngle" ) = util::Value<uint16_t>( results.str( 3 ) );

		const util::Value<int> day = results.str( 4 ), month = results.str( 5 ), year = results.str( 6 );
		const util::Value<uint8_t> hours = boost::lexical_cast<uint8_t>( results.str( 7 ) ), minutes = boost::lexical_cast<uint8_t>( results.str( 8 ) ), seconds = boost::lexical_cast<uint8_t>( results.str( 9 ) );

		boost::posix_time::ptime sequenceStart = boost::posix_time::ptime(
			boost::gregorian::date( ( int )year, ( int )month, ( int )day ),
			boost::posix_time::time_duration( hours, minutes, seconds )
		);
		props.setPropertyAs<boost::posix_time::ptime>( "sequenceStart", sequenceStart );

		LOG( Runtime, info ) << "Using Tr=" << props.propertyValue( "repetitionTime" ) << ", Te=" << props.propertyValue( "echoTime" )
		<< ", flipAngle=" << props.propertyValue( "flipAngle" ) << " and sequenceStart=" << props.propertyValue( "sequenceStart" )
		<< " from SPM8 description.";

		return true;
	} else
		return false;
}
void ImageFormat_NiftiSa::storeHeader( const util::PropertyMap &props, _internal::nifti_1_header *head )
{
	bool saved = false;

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
		} else
			storeSForm( props, head );

		saved = true;
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
			saved = true;
		} else
			saved = storeQForm( props, head );
	}

	//store current orientation (may override values set above)
	if( !saved && !storeQForm( props, head ) ) //try to encode as quaternion
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
		props.setPropertyAs( "nifti/sform_code", formCode ).castTo<util::Selection>().set( head->sform_code );
		props.setPropertyAs( "nifti/srow_x", util::fvector4() ).castTo<util::fvector4>().copyFrom( head->srow_x, head->srow_x + 4 );;
		props.setPropertyAs( "nifti/srow_y", util::fvector4() ).castTo<util::fvector4>().copyFrom( head->srow_y, head->srow_y + 4 );;
		props.setPropertyAs( "nifti/srow_z", util::fvector4() ).castTo<util::fvector4>().copyFrom( head->srow_z, head->srow_z + 4 );;
	}

	if( head->qform_code ) { // get the quaternion if qform_code>0
		props.setPropertyAs( "nifti/qform_code", formCode ).castTo<util::Selection>().set( head->qform_code );
		props.setPropertyAs( "nifti/quatern_b", head->quatern_b );
		props.setPropertyAs( "nifti/quatern_c", head->quatern_c );
		props.setPropertyAs( "nifti/quatern_d", head->quatern_d );
		props.setPropertyAs( "nifti/qoffset", util::fvector4( head->qoffset_x, head->qoffset_y, head->qoffset_z, 0 ) );
		props.setPropertyAs( "nifti/qfac", ( head->pixdim[0] == -1 ) ? -1 : 1 );

		// voxel size
		util::dlist v_size(dims);
		std::copy(head->pixdim+1,head->pixdim + dims+1,v_size.begin());//@todo implement size_fac
		props.setPropertyAs( "nifti/pixdim", v_size );
	}

	if( head->sform_code ) { // if sform_code is set, use that regardless of qform
		useSForm( props );
	} else if( head->qform_code ) { // if qform_code is set, but no sform use that (thats the "normal" case)
		useQForm( props );
	} else {
		LOG( Runtime, warning ) << "Neither sform_code nor qform_code are set, using identity matrix for geometry";
		const util::fvector4 r[3] = {nifti2isis.getRow( 0 ),nifti2isis.getRow( 1 ),nifti2isis.getRow( 2 )};
		props.setPropertyAs( "rowVec",      util::fvector3( r[0][0],r[0][1],r[0][2] ) ); // we use the transformation from nifti to isis as unity
		props.setPropertyAs( "columnVec",   util::fvector3( r[1][0],r[1][1],r[1][2] ) ); // because the image will very likely be in nifti space
		props.setPropertyAs( "sliceVec",    util::fvector3( r[2][0],r[2][1],r[2][2] ) );
		props.setPropertyAs( "voxelSize",   util::fvector3( head->pixdim[1], head->pixdim[2], head->pixdim[3] ) );
		props.setPropertyAs( "indexOrigin", util::fvector3( 0, 0 ) );
	}

	// set space unit factors
	props.propertyValue( "voxelSize" ).castTo<util::fvector3>() *= size_fac;
	props.propertyValue( "indexOrigin" ).castTo<util::fvector3>() *= size_fac;

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

	if( head->intent_code  ) {
		props.setPropertyAs( "nifti/intent_code", head->intent_code ); // use it the usual way
		LOG( Runtime, warning ) << "Ignoring intent_code " << props.propertyValue( "nifti/intent_code" );
	}


	if( head->cal_max != 0 || head->cal_min != 0 ) { // maybe someone needs that, we dont ...
		props.setPropertyAs( "nifti/cal_max", head->cal_max );
		props.setPropertyAs( "nifti/cal_min", head->cal_min );
	}

	util::fvector3 &vsize = props.propertyValue( "voxelSize" ).castTo<util::fvector3>();

	for( short i = 0; i < std::min<short>( head->dim[0], 3 ); i++ ) {
		if( vsize[i] == 0 ) {
			LOG( Runtime, warning ) << "The voxelSize[" << i << "] is 0. Assuming \"1\"";
			vsize[i] = 1;
		}
	}

	return parseSliceOrdering( head, props );
}

std::string ImageFormat_NiftiSa::getName()const {return "Nifti standalone";}

isis::data::ValueArray< bool > ImageFormat_NiftiSa::bitRead( data::ValueArray< uint8_t > src, size_t size )
{
	assert( size );

	if( src.getLength() * 8 < size ) {
		std::string err( "unexpected end of file (missing " );
		err += boost::lexical_cast<std::string>( size - src.getLength() * 8 ) + " bytes)";
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

bool ImageFormat_NiftiSa::checkSwapEndian ( _internal::nifti_1_header *header )
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


int ImageFormat_NiftiSa::load ( std::list<data::Chunk> &chunks, const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> /*progress*/ )  throw( std::runtime_error & )
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
	_internal::nifti_1_header *header = reinterpret_cast<_internal::nifti_1_header *>( &mfile[0] );
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

	//set up the size - copy dim[0] values from dim[1]..dim[dim[0]]
	util::vector4<size_t> size;
	size.fill( 1 );

	size.copyFrom( header->dim + 1, header->dim + 1 + header->dim[0] );
	data::ValueArrayReference data_src;

	if( header->datatype == NIFTI_TYPE_BINARY ) { // image is binary encoded - needs special decoding
		data_src = bitRead( mfile.at<uint8_t>( header->vox_offset ), size.product() );
	} else if( util::istring( "fsl" ) == dialect.c_str() && header->datatype == NIFTI_TYPE_UINT8 && size[data::timeDim] == 3 ) { //if its fsl-three-volume-color copy the volumes
		LOG( Runtime, notice ) << "The image has 3 timesteps and its type is UINT8, assuming it is an fsl color image.";
		const size_t volume = size.product() / 3;
		data::ValueArray<util::color24> buff( volume );
		const data::ValueArray<uint8_t> src = mfile.at<uint8_t>( header->vox_offset, volume * 3 );
		LOG( Runtime, info ) << "Mapping nifti image as FSL RBG set of 3*" << volume << " elements";

		for( size_t v = 0; v < volume; v++ ) {
			buff[v].r = src[v];
			buff[v].g = src[v + volume];
			buff[v].b = src[v + volume * 2];
		}

		data_src = buff;
		size[data::timeDim] = 1;
	} else if( util::istring( "fsl" ) == dialect.c_str() && header->datatype == NIFTI_TYPE_FLOAT32 && size[data::timeDim] == 3 ) { //if its fsl-three-volume-vector copy the volumes
		LOG( Runtime, notice ) << "The image has 3 timesteps and its type is FLOAT32, assuming it is an fsl vector image.";
		const size_t volume = size.product() / 3;
		data::ValueArray<util::fvector3> buff( volume );
		const data::ValueArray<float> src = mfile.at<float>( header->vox_offset, size.product(), swap_endian );

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
			data_src = mfile.atByID( type, header->vox_offset, size.product(), swap_endian );

			if( swap_endian ) {
				LOG( Runtime, info ) << "Opened nifti image as endianess swapped " << data_src->getTypeName() << " of " << data_src->getLength()
				<< " elements (" << data_src->bytesPerElem()*data_src->getLength() << ")";
			} else {
				LOG( Runtime, info ) << "Mapped nifti image natively as " << data_src->getTypeName() << " of " << data_src->getLength()
				<< " elements (" << data_src->bytesPerElem()*data_src->getLength() << ")";
			}

			LOG_IF( ( size_t )header->bitpix != data_src->bytesPerElem() * 8, Runtime, warning )
					<< "nifti field bitpix does not fit the bytesize of the given datatype (" << data_src->getTypeName() << "/" << header->bitpix <<  ")";

		} else {
			LOG( Runtime, error ) << "Sorry, the nifti datatype " << header->datatype << " is not (yet) supported";
			throwGenericError( "unsupported datatype" );
		}
	}

	std::list<data::Chunk> newChunks = parseHeader( header, data::Chunk( data_src, size[0], size[1], size[2], size[3] ) );
	chunks.insert( chunks.begin(), newChunks.begin(), newChunks.end() );
	return newChunks.size();
}

std::auto_ptr< _internal::WriteOp > ImageFormat_NiftiSa::getWriteOp( const isis::data::Image &src, isis::util::istring dialect )
{
	const size_t bpv = src.getMaxBytesPerVoxel() * 8;
	unsigned short target_id = src.getMajorTypeID(); //default to major type of the image

	//bitmap is not supportet by spm and fsl
	if( target_id == data::ValueArray<bool>::staticID ) {
		if( dialect == "fsl" || dialect == "spm" ) {
			target_id = typeFallBack<bool, uint8_t>( dialect.c_str() );// fall back to uint8_t and use normal writer for that
		} else {
			return std::auto_ptr<_internal::WriteOp>( new _internal::BitWriteOp( src ) ); // use special writer for bit
		}
	}

	// fsl cannot deal with some types
	if( dialect == "fsl" ) {
		switch( target_id ) {
		case data::ValueArray<uint16_t>::staticID:
			target_id = typeFallBack<uint16_t, int16_t>( "fsl" );
			break;
		case data::ValueArray<uint32_t>::staticID:
			target_id = typeFallBack<uint32_t, int32_t>( "fsl" );
			break;
		case data::ValueArray<util::color24>::staticID:

			if( src.getRelevantDims() > 3 ) {
				LOG( Runtime, error ) << "Cannot store color image of size " << src.getSizeAsString() << " using fsl dialect (4th dim is needed for the colors)";
				throwGenericError( "unsupported datatype" );
			} else {
				LOG( Runtime, info ) << data::ValueArray<util::color24>::staticName() <<  " is not supported by fsl falling back to color encoded in 4th dimension";
				return std::auto_ptr<_internal::WriteOp>( new _internal::FslRgbWriteOp( src ) );
			}

			break;
		}
	}

	// generic case (use generic scalar writer for the target_id)
	return std::auto_ptr<_internal::WriteOp>( new _internal::CommonWriteOp( src, target_id, bpv, ( dialect == "spm" ) ) );
}


void ImageFormat_NiftiSa::write( const data::Image &image, const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> /*progress*/ )  throw( std::runtime_error & )
{
	const size_t voxel_offset = 352; // must be >=352 (and multiple of 16)  (http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/vox_offset.html)
	std::auto_ptr< _internal::WriteOp > writer = getWriteOp( image, dialect.c_str() ); // get a fitting writer for the datatype
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


		// get the first 348 bytes as header
		_internal::nifti_1_header *header = writer->getHeader();
		header->datatype = nifti_id;

		guessSliceOrdering( image, header->slice_code, header->slice_duration );

		if( image.getMajorTypeID() == data::ValueArray<util::color24>::staticID ) {
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
	const util::dvector4 offset = props.getPropertyAs<util::dvector4>( "indexOrigin" );

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
		offset
	).transpose();// the columns of the transform matrix are the scaled row-, column-, sliceVec and the offset
	image2isis.elem( 3, 3 ) = 1; // element 4/4 must be "1"

	return nifti2isis.transpose().dot( image2isis ); // apply inverse transform from nifti to isis => return transformation from image to nifti space
}

void ImageFormat_NiftiSa::useSForm( util::PropertyMap &props )
{
	// srow_? is the linear map from image space to nifti space (not isis space)
	// [x_nii] [ nifti/srow_x ]   [i]
	// [x_nii]=[ nifti/srow_y ] * [j]
	// [x_nii] [ nifti/srow_z ]   [k]

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
	props.setPropertyAs( "indexOrigin", util::fvector3( origin[0], origin[1], origin[2] ) );
	LOG( Debug, info ) << "Computed indexOrigin=" << props.propertyValue( "indexOrigin" ) << " from sform";

	//remove offset from image2isis
	image2isis = util::Matrix4x4<float>(
		util::fvector4( 1, 0, 0, -origin[0] ),
		util::fvector4( 0, 1, 0, -origin[1] ),
		util::fvector4( 0, 0, 1, -origin[2] )
	).dot( image2isis );

	const util::fvector3 voxelSize( // get voxel sizes by transforming othogonal vectors of one voxel from image to isis
		image2isis.dot( util::fvector4( 1, 0, 0 ) ).len(),
		image2isis.dot( util::fvector4( 0, 1, 0 ) ).len(),
		image2isis.dot( util::fvector4( 0, 0, 1 ) ).len()
	);

	props.setPropertyAs( "voxelSize", voxelSize );
	LOG( Debug, info ) << "Computed voxelSize=" << props.propertyValue("voxelSize" ) << " from sform";


	//remove scaling from image2isis
	image2isis = image2isis.dot( util::Matrix4x4<float>(
		util::fvector4( 1 / voxelSize[0], 0, 0 ),
		util::fvector4( 0, 1 / voxelSize[1], 0 ),
		util::fvector4( 0, 0, 1 / voxelSize[2] )
	) );

	const util::fvector4 r[3] = {image2isis.transpose().getRow( 0 ),image2isis.transpose().getRow( 1 ),image2isis.transpose().getRow( 2 )};
	props.setPropertyAs( "rowVec",      util::fvector3( r[0][0],r[0][1],r[0][2] ) ); 
	props.setPropertyAs( "columnVec",   util::fvector3( r[1][0],r[1][1],r[1][2] ) ); 
	props.setPropertyAs( "sliceVec",    util::fvector3( r[2][0],r[2][1],r[2][2] ) );

	LOG( Debug, info ) << "Computed rowVec=" << props.propertyValue( "rowVec" ) << ", "
	<< "columnVec=" << props.propertyValue("columnVec" ) << " and "
	<< "sliceVec=" << props.propertyValue( "sliceVec" ) << " from sform";

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
	util::dvector4 quaternion(
		0,//a
		props.getPropertyAs<double>( "nifti/quatern_b" ),
		props.getPropertyAs<double>( "nifti/quatern_c" ),
		props.getPropertyAs<double>( "nifti/quatern_d" )
	);

	double &a = quaternion[0], &b = quaternion[1], &c = quaternion[2], &d = quaternion[3];

	if( 1 - quaternion.sqlen() < 1.e-7 ) { //if the quaternion is to "long"
		quaternion.norm();      //normalize it and leave the angle as 0
	} else {
		a = sqrt( 1 - quaternion.sqlen() );                 /* angle = 2*arccos(a) */
	}

	LOG( Debug, info ) << "Using qform (" << props.propertyValue( "nifti/qform_code" ).toString()
	<< ") quaternion=" << util::fvector4( a, b, c, d ) << " with qfac=" << props.propertyValue( "nifti/qfac" ).toString()
	<< ", pixdim=" << props.propertyValue( "nifti/pixdim" ).toString()
	<< " and qoffset= " << props.propertyValue( "nifti/qoffset" ).toString();

	const double a2 = a * a, b2 = b * b, c2 = c * c, d2 = d * d;
	const double _2ab = 2 * a * b, _2ac = 2 * a * c, _2ad = 2 * a * d;
	const double _2bc = 2 * b * c, _2bd = 2 * b * d;
	const double _2cd = 2 * c * d;

	const double r_11 = a2 + b2 - c2 - d2, r_12 = _2bc - _2ad,  r_13 = _2bd + _2ac;
	const double r_21 = _2bc + _2ad,  r_22 = a2 - b2 + c2 - d2, r_23 = _2cd - _2ab;
	const double r_31 = _2bd - _2ac,  r_32 = _2cd + _2ab,  r_33 = a2 - b2 - c2 + d2;
	const int qfac = props.getPropertyAs<float>( "nifti/qfac" );

	const util::Matrix4x4<double> image2nifti(
		util::fvector4( r_11, r_12, r_13 * qfac ),
		util::fvector4( r_21, r_22, r_23 * qfac ),
		util::fvector4( r_31, r_32, r_33 * qfac )
	);

	LOG( Debug, info ) << "The matrix made from the qform is "
	<< util::fvector3( r_11, r_12, r_13 * qfac ) << "-"
	<< util::fvector3( r_21, r_22, r_23 * qfac ) << "-"
	<< util::fvector3( r_31, r_32, r_33 * qfac );

	const util::Matrix4x4<double> image2isis = nifti2isis.dot( image2nifti );

	const util::fvector4 r[3] = {image2isis.transpose().getRow( 0 ),image2isis.transpose().getRow( 1 ),image2isis.transpose().getRow( 2 )};
	props.setPropertyAs( "rowVec",      util::fvector3( r[0][0],r[0][1],r[0][2] ) ); 
	props.setPropertyAs( "columnVec",   util::fvector3( r[1][0],r[1][1],r[1][2] ) ); 
	props.setPropertyAs( "sliceVec",    util::fvector3( r[2][0],r[2][1],r[2][2] ) );

	LOG( Debug, info ) << "Computed rowVec=" << props.propertyValue( "rowVec" ) << ", "
	<< "columnVec=" << props.propertyValue( "columnVec" ) << " and "
	<< "sliceVec=" << props.propertyValue( "sliceVec" ) << " from qform";

	props.remove( "nifti/quatern_b" );
	props.remove( "nifti/quatern_c" );
	props.remove( "nifti/quatern_d" );
	props.remove( "nifti/qfac" );

	// indexOrigin //////////////////////////////////////////////////////////////////////////////////
	const util::fvector4 origin= nifti2isis.dot( props.getPropertyAs<util::fvector4>( "nifti/qoffset" ) );
	props.setPropertyAs( "indexOrigin", util::fvector3( origin[0], origin[1], origin[2] ) );
	LOG( Debug, info ) << "Computed indexOrigin=" << props.propertyValue( "indexOrigin" ) << " from qform";
	props.remove( "nifti/qoffset" );

	// voxelSize //////////////////////////////////////////////////////////////////////////////////
	const util::dlist::iterator pixdimStart = props.propertyValue("nifti/pixdim").castTo<util::dlist>().begin();
	util::dlist::iterator pixdimEnd = pixdimStart; std::advance(pixdimEnd,3);
	props.setPropertyAs("voxelSize",util::fvector3()).castTo<util::fvector3>().copyFrom(pixdimStart,pixdimEnd); //@todo is conversion dlist > fvector3 available
	props.remove("nifti/pixdim");
	LOG( Debug, info ) << "Computed voxelSize=" << props.propertyValue( "voxelSize" ) << " from qform";
}
bool ImageFormat_NiftiSa::storeQForm( const util::PropertyMap &props, _internal::nifti_1_header *head )
{

	// take values of the 3x3 matrix == analog to the nifti reference implementation
	const isis::util::Matrix4x4< double > nifti2image = getNiftiMatrix( props ).transpose(); //use the inverse of image2nifti to extract direction vectors easier

	util::fvector3 col[3];

	for( int i = 0; i < 3; i++ ) {
		const util::dvector4 buff = nifti2image.getRow( i );
		col[i].copyFrom(buff.begin(),buff.begin()+3); //nth column in image2nifti
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

	head->qoffset_x = nifti2image.elem( 0, 3 );
	head->qoffset_y = nifti2image.elem( 1, 3 );
	head->qoffset_z = nifti2image.elem( 2, 3 );

	return true;
}
void ImageFormat_NiftiSa::storeSForm( const util::PropertyMap &props, _internal::nifti_1_header *head )
{
	const util::Matrix4x4<double> sform = getNiftiMatrix( props );

	if( !head->sform_code )head->sform_code = 1; // default to 1 if not set till now

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
