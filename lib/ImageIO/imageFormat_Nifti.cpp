/****************************************************************
 *
 * <Copyright information>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Lydia Hellrung, hellrung@cbs.mpg.de, 2009
 *
 * ImageFormatNii.cpp
 *
 * Description:
 *
 *  Created on: Aug 12, 2009
 *      Author: hellrung
 ******************************************************************/

//LOCAL INCLUDES
#include <DataStorage/io_interface.h>
#include <CoreUtils/type.hpp>
#include <DataStorage/common.hpp>
#include <CoreUtils/vector.hpp>
#include <DataStorage/typeptr.hpp>

//SYSTEM INCLUDES
#include <nifti1_io.h>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/regex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace isis
{
namespace image_io
{

/*************************
 * creating an own chunk for better Nifti handling (whole file handling)
 **************************/
namespace _internal
{

class NiftiChunk : public data::Chunk
{
public:
	template<typename T, typename D> NiftiChunk( T *src, D del, size_t width, size_t height, size_t slices, size_t timesteps ) :
		data::Chunk( src, del, width, height, slices, timesteps ) {
		LOG( ImageIoDebug, info ) << "create NiftiChunk";
	}

	//TODO: This is really weird staff - the gcc4.2 on MAC and RedHat is complaining about
	// the private copy constructor because he cannot resolve the template constructor when creating NiftiChunk
	// e.g. in retList.push_back( _internal::NiftiChunk::makeNiftiChunk( static_cast<uint8_t *> (ni->data), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ? ni->dim[4] : 1 )  );
	// That's completely crazy but this workaround is a first solution - we hope to find a better one
	template<typename T, typename D> static data::Chunk makeNiftiChunk( T *src, D del, size_t width, size_t height, size_t slices, size_t timesteps )  {
		return NiftiChunk( src, del, width, height, slices, timesteps );
	}

private:
	NiftiChunk( const NiftiChunk & ); // no standard copy constructor
	NiftiChunk &operator=( const NiftiChunk & ); // no copy operator
};// class NiftiChunk
}//namespace _internal


/*************************
 * now, the real plugin for Nifti conversion
 **************************/
class ImageFormat_Nifti : public FileFormat
{
	// own chunk creations, like NiftiChunk, usually having it's own deleter
	struct Deleter {
		//members
		nifti_image *m_pNiImage;
		std::string m_filename;
		//constructor
		Deleter( nifti_image *ni, const std::string &filename ) :
			m_pNiImage( ni ), m_filename( filename ) {}

		//the most important operator
		void operator ()( void *at ) {
			LOG_IF( NULL == m_pNiImage, ImageIoLog, error )
					<<  "Trying to close non-existing nifti file: " << util::MSubject( m_filename );
			LOG( ImageIoDebug, info ) << "Closing Nifti-Chunk file " << util::MSubject( m_filename );
			//clean up with the function from nifti1_io.h
			nifti_image_free( m_pNiImage );
		}
	};
protected:
	std::string suffixes()const {
		return std::string( ".nii.gz .nii .hdr" );
	}
public:
	enum vectordirection {readDir = 0, phaseDir, sliceDir, indexOrigin, voxelSizeVec};


	std::string dialects( const std::string &filename )const {
		return std::string( "fsl spm" );
	}

	std::string getName()const {
		//TODO: wahrscheinlich sollten die Namen irgendwie so aussehen "mpg.cbs.nii"?
		return "Nifti";
	}

	bool tainted()const {
		return false;
	}//internal plugins are not tainted

	/***********************
	 * load file
	 ************************/
	int load( std::list<data::Chunk> &retList, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		//read the file with the function from nifti1_io.h
		nifti_image *ni = nifti_image_read( filename.c_str(), true );

		if ( not ni )
			throwGenericError( "nifti_image_read() failed" );

		// 0.0 would mean "not in use" - so for better handling use a 1.0
		float scale = ni->scl_slope ? ni->scl_slope : 1.0;

		// TODO: at the moment scaling not supported due to data type changes
		if ( 1.0 != scale ) {
			//          throwGenericError( std::string( "Scaling is not supported at the moment. Scale Factor: " ) + util::Value<float>( scale ).toString() );
			LOG( ImageIoDebug, warning ) << "Scaling is not supported at the moment. Scale Factor: "  + util::Value<float>( scale ).toString();
		}

		LOG( ImageIoDebug, isis::info ) << "datatype to load from nifti " << ni->datatype;
		Deleter del( ni, filename );

		switch ( ni->datatype ) {
		case DT_UINT8:
			retList.push_back( _internal::NiftiChunk::makeNiftiChunk( static_cast<uint8_t *> ( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ? ni->dim[4] : 1 )  );
			break;
		case DT_INT8:
			retList.push_back( _internal::NiftiChunk::makeNiftiChunk( static_cast<int8_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ? ni->dim[4] : 1 ) );
			break;
		case DT_INT16:
			retList.push_back( _internal::NiftiChunk::makeNiftiChunk( static_cast<int16_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ? ni->dim[4] : 1 ) );
			break;
		case DT_UINT16:
			retList.push_back( _internal::NiftiChunk::makeNiftiChunk( static_cast<uint16_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ? ni->dim[4] : 1 ) );
			break;
		case DT_UINT32:
			retList.push_back( _internal::NiftiChunk::makeNiftiChunk( static_cast<uint32_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ? ni->dim[4] : 1 ) );
			break;
		case DT_INT32:
			retList.push_back( _internal::NiftiChunk::makeNiftiChunk( static_cast<int32_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ? ni->dim[4] : 1 ) );
			break;
		case DT_FLOAT32:
			retList.push_back( _internal::NiftiChunk::makeNiftiChunk( static_cast<float *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ? ni->dim[4] : 1 ) );
			break;
		case DT_FLOAT64:
			retList.push_back( _internal::NiftiChunk::makeNiftiChunk( static_cast<double *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ? ni->dim[4] : 1 ) );
			break;
		default:
			throwGenericError( std::string( "Unsupported datatype " ) + util::Value<int>( ni->datatype ).toString() );
		}

		// don't forget to take the properties with
		copyHeaderFromNifti( retList.back(), *ni );
		//      if ( dialect == "spm" )
		//      {
		//          boost::shared_ptr<data::MemChunk<int16_t> >
		//              dst( new data::MemChunk<int16_t>( ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) ) ;
		//          retChunk->swapAlong( *dst, 0 );
		//          retList.push_back( *dst );
		//          return 1;
		//      }
		//

		return 1; // if there was an error, we wouldn't get here
	}


	/***********************
	 * write file
	 ************************/
	void write( const data::Image &imageOrig, const std::string &filename, const std::string &sdialect ) throw( std::runtime_error & ) {
		const util::istring dialect( sdialect.begin(), sdialect.end() );
		LOG( Debug, info ) << "Writing image of size " << imageOrig.getSizeAsString() << " and type " << imageOrig.getMajorTypeName() << " as nifti";

		boost::filesystem::path boostFilename( filename );
		//copy of our image due to changing it by transformCoords
		isis::data::Image image( imageOrig );
		//default init for nifti image
		nifti_image ni;
		memset( &ni, 0, sizeof( nifti_image ) ); //set everything to zero - default value for "not used"
		ni.nu = ni.nv = ni.nw = 1;
		ni.datatype = DT_UNKNOWN;
		ni.data = NULL;
		ni.fname = const_cast<char *>( filename.c_str() );
		//orientation in isis LPS - but in nifti everything relative to RAS
		// - so let's change row/column direction and sign of indexOrigin
		//now we try to transform
		boost::numeric::ublas::matrix<float> matrix( 3, 3 );
		matrix( 0, 0 ) = -1;
		matrix( 0, 1 ) = 0;
		matrix( 0, 2 ) = 0;
		matrix( 1, 0 ) = 0;
		matrix( 1, 1 ) = -1;
		matrix( 1, 2 ) = 0;
		matrix( 2, 0 ) = 0;
		matrix( 2, 1 ) = 0;
		matrix( 2, 2 ) = +1;
		image.transformCoords( matrix );
		//set the props from the image to the nifti file
		copyHeaderToNifti( image, ni );
		// set filename for resulting image(s) due to Analyze vs. Nifti
		ni.fname = const_cast<char *>( filename.c_str() ); // header name
		boost::filesystem::path imgname;

		if ( "hdr" == extension( boostFilename ) ) {
			ni.nifti_type = 0; // that's ANALYZE ID
			imgname  =  change_extension( boostFilename, ".img" );
			ni.iname = const_cast<char *>( imgname.file_string().c_str() );
		} else {
			ni.nifti_type = 1; // that's NIFTI ID
			ni.iname = const_cast<char *>( filename.c_str() );
		}

		// FSL compatibility
		if ( dialect != "fsl" ) { //@todo wtf ? shouldn't that be ==
			//  stuffFslCompatibility(image, ni);
		}

		// copy the data to the nifti image
		LOG( ImageIoLog, isis::info ) << "image typeid: " << image.getMajorTypeID();
		LOG( ImageIoLog, isis::info ) << "image typename: " << image.getMajorTypeName();

		switch ( image.getMajorTypeID() ) {
		case data::ValuePtr<int8_t>::staticID:

			if ( dialect == "fsl" ) { // fsl not compatible with int8, convert to uint8
				data::TypedImage<uint8_t> fslCopy( image );
				ni.datatype = DT_UINT8;
				copyDataToNifti<uint8_t>( fslCopy, ni );
				break;
			}

			ni.datatype = DT_INT8;
			copyDataToNifti<int8_t>( image, ni );
			break;
		case data::ValuePtr<uint8_t>::staticID:
			ni.datatype = DT_UINT8;
			copyDataToNifti<uint8_t>( image, ni );
			break;
		case data::ValuePtr<int16_t>::staticID:
			ni.datatype = DT_INT16;
			copyDataToNifti<int16_t>( image, ni );
			break;
		case data::ValuePtr<uint16_t>::staticID:

			if ( dialect == "fsl" ) {
				//              image.print( std::cout );
				data::TypedImage<int16_t> fslCopy( image );
				ni.datatype = DT_INT16;
				copyDataToNifti<int16_t>( fslCopy, ni );
			} else {
				ni.datatype = DT_UINT16;
				copyDataToNifti<uint16_t>( image, ni );
			}

			break;
		case data::ValuePtr<int32_t>::staticID:
			ni.datatype = DT_INT32;
			copyDataToNifti<int32_t>( image, ni );
			break;
		case data::ValuePtr<uint32_t>::staticID:

			if ( dialect == "fsl" ) {
				data::TypedImage<int32_t> fslCopy( image );
				ni.datatype = DT_INT32;
				copyDataToNifti<int32_t>( image, ni );
				break;
			}

			ni.datatype = DT_UINT32;
			copyDataToNifti<uint16_t>( image, ni );
			break;
		case data::ValuePtr<float>::staticID:
			ni.datatype = DT_FLOAT32;
			copyDataToNifti<float>( image, ni );
			break;
		case data::ValuePtr<double>::staticID:
			ni.datatype = DT_FLOAT64;
			copyDataToNifti<double>( image, ni );
			break;
		default:
			throwGenericError( "Datatype " + image.getMajorTypeName() + " cannot be written!" );
		}

		//now really write the nifti file with the function from nifti1_io.h
		errno = 0; //reset errno
		nifti_image_write( &ni ); //write the image - in case of a failure errno should be set

		if ( errno )
			throwSystemError( errno );
	}

	/****************************************
	 * PRIVATE
	 ****************************************/
private:

	void geometryFromNifti( util::PropertyValue &row, util::PropertyValue &column, util::PropertyValue &slice, const mat44 &geo, const util::fvector4 &div ) {
		row->castTo<util::fvector4>() = util::fvector4( geo.m[0][0], geo.m[1][0], geo.m[2][0], geo.m[3][0] ) / div[0];
		column->castTo<util::fvector4>() = util::fvector4( geo.m[0][1], geo.m[1][1], geo.m[2][1], geo.m[3][1] ) / div[1];
		slice->castTo<util::fvector4>() = util::fvector4( geo.m[0][2], geo.m[1][2], geo.m[2][2], geo.m[3][2] ) / div[2];
	}

	void copyHeaderFromNifti( data::Chunk &retChunk, const nifti_image &ni ) {
		util::fvector4 dimensions( ni.dim[1], ni.ndim >= 2 ? ni.dim[2] : 1,
								   ni.ndim >= 3 ? ni.dim[3] : 1, ni.ndim >= 4 ? ni.dim[4] : 1 );
		LOG( ImageIoLog, info ) << "size of chunk " << dimensions << "/" << ni.ndim;

		for ( int t = 0; t < dimensions[3]; t++ ) {
			util::fvector4 offsets( ni.qoffset_x, ni.qoffset_y, ni.qoffset_z, 0 );
			//retChunk.setPropertyAs("acquisitionTime", );
			retChunk.setPropertyAs<uint32_t>( "acquisitionNumber", t );
			retChunk.setPropertyAs<uint16_t>( "sequenceNumber", 1 );
			// in nifti everything should be relative to RAS, in isis we use LPS coordinates - normally change row/column dir and sign of indexOrigin
			//TODO: has to be tested with different niftis - don't trust them!!!!!!!!!
			retChunk.setPropertyAs( "indexOrigin", util::fvector4( ni.qoffset_x, ni.qoffset_y, ni.qoffset_z, 0 ) );

			retChunk.setPropertyAs( "nifti/qform_code", ni.qform_code );
			retChunk.setPropertyAs( "nifti/sform_code", ni.sform_code );
			const util::fvector4 &voxel_size = retChunk.setPropertyAs( "voxelSize", getVector( ni, voxelSizeVec ) )->castToType<util::fvector4>();

			if( ni.sform_code ) {
				geometryFromNifti(
					retChunk.setPropertyAs( "rowVec", util::fvector4() ),
					retChunk.setPropertyAs( "columnVec", util::fvector4() ),
					retChunk.setPropertyAs( "sliceVec", util::fvector4() ),
					ni.sto_xyz, voxel_size
				);
				LOG( Debug, info ) << "Using sto_xyz as primary orientation "
								   << retChunk.propertyValue( "rowVec" ) << " " << retChunk.propertyValue( "columnVec" ) << retChunk.propertyValue( "sliceVec" );

				if( ni.qform_code ) {
					geometryFromNifti(
						retChunk.setPropertyAs( "nifti/qrowVec", util::fvector4() ),
						retChunk.setPropertyAs( "nifti/qcolumnVec", util::fvector4() ),
						retChunk.setPropertyAs( "nifti/qsliceVec", util::fvector4() ),
						ni.qto_xyz, voxel_size
					);
					LOG( Debug, info ) << "Using qto_xyz as secondary orientation "
									   << retChunk.propertyValue( "nifti/qrowVec" ) << " " << retChunk.propertyValue( "nifti/qcolumnVec" ) << retChunk.propertyValue( "nifti/qsliceVec" );
				}
			} else if( ni.qform_code ) {
				geometryFromNifti(
					retChunk.setPropertyAs( "rowVec", util::fvector4() ),
					retChunk.setPropertyAs( "columnVec", util::fvector4() ),
					retChunk.setPropertyAs( "sliceVec", util::fvector4() ),
					ni.qto_xyz, voxel_size
				);
				LOG( Debug, info ) << "Using qto_xyz as primary orientation "
								   << retChunk.propertyValue( "rowVec" ) << " " << retChunk.propertyValue( "columnVec" ) << retChunk.propertyValue( "sliceVec" );
			} else {
				LOG( Runtime, warning ) << "Neigther sform_code nor qform_code are set, using identity matrix for geometry";
				retChunk.setPropertyAs( "rowVec",  util::fvector4( 1, 0, 0 ) );
				retChunk.setPropertyAs( "columnVec", util::fvector4( 0, 1, 0 ) );
				retChunk.setPropertyAs( "sliceVec", util::fvector4( 0, 0, 1 ) );
			}

			//now we try to transform
			boost::numeric::ublas::matrix<float> matrix( 3, 3 );
			matrix( 0, 0 ) = -1;
			matrix( 0, 1 ) = 0;
			matrix( 0, 2 ) = 0;
			matrix( 1, 0 ) = 0;
			matrix( 1, 1 ) = -1;
			matrix( 1, 2 ) = 0;
			matrix( 2, 0 ) = 0;
			matrix( 2, 1 ) = 0;
			matrix( 2, 2 ) = +1;
			retChunk.transformCoords( matrix );
			retChunk.setPropertyAs( "sequenceDescription", std::string( ni.descrip ) );
			retChunk.setPropertyAs( "studyDescription", std::string( ni.intent_name ) );

			if ( ( 2 == ni.freq_dim ) and ( 1 == ni.phase_dim ) ) {
				retChunk.setPropertyAs<std::string>( "inplanePhaseEncodingDirection", "ROW" );
			} else if ( ( 1 == ni.freq_dim ) and ( 2 == ni.phase_dim ) ) {
				retChunk.setPropertyAs<std::string>( "inplanePhaseEncodingDirection", "COL" );
			}

			retChunk.setPropertyAs( "voxelGap", util::fvector4() ); // not extra included in Nifti, so set to zero
			//just some LOGS
			LOG( ImageIoLog, info ) << "dims at all " << dimensions;
			LOG( ImageIoLog, info ) << "Offset values from nifti" << offsets;
			LOG( ImageIoLog, info ) << "FOV row/column/slice/voxelsize:"
									<< retChunk.propertyValue( "rowVec" ).toString( false )
									<< " / " << retChunk.propertyValue( "columnVec" ).toString( false )
									<< " / " << retChunk.propertyValue( "sliceVec" ).toString( false )
									<< " / " << voxel_size;
		}

		//check description for tr, te and fa and date which is written by spm8
		boost::regex descriptionRegex(
			".*TR=([[:digit:]]{1,})ms.*TE=([[:digit:]]{1,})ms.*FA=([[:digit:]]{1,})deg\\ *([[:digit:]]{1,2}).([[:word:]]{3}).([[:digit:]]{4})\\ *([[:digit:]]{1,2}):([[:digit:]]{1,2}):([[:digit:]]{1,2}).*" );
		boost::cmatch results;
		uint16_t tr = 0;
		uint16_t te = 0;
		uint16_t fa = 0;
		std::string day, month, year;
		size_t hours, minutes, seconds;
		boost::gregorian::date isisDate;
		boost::posix_time::time_duration isisTimeDuration;

		if ( boost::regex_match( ni.descrip, results,  descriptionRegex ) ) {
			tr = boost::lexical_cast<uint16_t>( results.str( 1 ) );
			te = boost::lexical_cast<uint16_t>( results.str( 2 ) );
			fa = boost::lexical_cast<uint16_t>( results.str( 3 ) );
			day = boost::lexical_cast<std::string>( results.str( 4 ) );
			month = boost::lexical_cast<std::string>( results.str( 5 ) );
			year = boost::lexical_cast<std::string>( results.str( 6 ) );
			hours = boost::lexical_cast<size_t>( results.str( 7 ) );
			minutes = boost::lexical_cast<size_t>( results.str( 8 ) );
			seconds = boost::lexical_cast<size_t>( results.str( 9 ) );

			if( day.size() == 1 ) {
				day.insert( 0, std::string( "0" ) );
			}

			isisTimeDuration = boost::posix_time::time_duration( hours, minutes, seconds );
			std::string strDate = year + "-" + month + "-" + day;
			isisDate = boost::gregorian::from_simple_string( strDate );
			boost::posix_time::ptime isisTime( isisDate, isisTimeDuration );
			LOG( ImageIoLog, info ) << "SPM8 description found.";
			retChunk.setPropertyAs<boost::posix_time::ptime>( "sequenceStart", isisTime );
			retChunk.setPropertyAs<uint16_t>( "flipAngle", fa );
			retChunk.setPropertyAs<uint16_t>( "echoTime", te );
			retChunk.setPropertyAs<uint16_t>( "repetitionTime", tr );
		}

		//if "TR=" was not found in description and pixdim[dim] == 0 a warning calls attention to use parameter -tr to change repetitionTime.
		if( tr == 0 && ni.pixdim[ni.ndim] == 0 ) {
			LOG( ImageIoLog, warning ) << "Repetition time seems to be invalid. To set the repetition time during conversion use the parameter -tr "; //@todo thet shouldn't be here
			retChunk.setPropertyAs<u_int16_t>( "repetitionTime", 0 );
		}

		if( !tr && ni.pixdim[ni.ndim] ) {
			retChunk.setPropertyAs<uint16_t>( "repetitionTime", ni.pixdim[ni.ndim] * 1000 );
		}

		util::fvector4 newVoxelSize = retChunk.getPropertyAs<util::fvector4>( "voxelSize" );
		newVoxelSize[3] = 0;
		retChunk.setPropertyAs<util::fvector4>( "voxelSize", newVoxelSize );
	}

	util::fvector4 getVector( const nifti_image &ni, const enum vectordirection &dir ) {
		util::fvector4 retVec( 0, 0, 0, 0 );
		float units; // conversion-factor to mm

		switch ( ni.xyz_units ) {
		case NIFTI_UNITS_METER:
			units = 1.0e3;
			break;
		case NIFTI_UNITS_MICRON:
			units = 1.0e-3;
			break;
		default:
			units = 1;
		}

		util::FixedVector<float, 4> voxel_size( ni.pixdim + 1 );
		voxel_size = voxel_size * units;

		if ( dir == voxelSizeVec )
			return voxel_size;

		if ( ni.nifti_type > 0 ) { // only for non-ANALYZE
			//      RotMatrix scaleMat;//method 2
			util::fvector4 qto, sto;

			//get qto
			for ( int i = 0; i < 3; i++ ) {
				qto[i] = ni.qto_xyz.m[i][dir] / voxel_size[dir];
			}

			LOG( ImageIoDebug, info ) << "Orientation red from qto_xyz:" << dir + 1 << " is " << qto;

			//get sto
			for ( int i = 0; i < 3; i++ ) {
				sto[i] = ni.sto_xyz.m[i][dir] / voxel_size[dir];
			}

			LOG( ImageIoDebug, info ) << "Orientation red from sto_xyz:" << dir + 1 << " is " << sto;

			//use one of them
			if ( ni.qform_code > 0 ) {// just tranform to the nominal space of the scanner
				retVec = qto;
			} else if ( ni.sform_code > 0 ) { // method 3
				retVec = sto;
			} else if ( ni.sform_code == 0 and ni.qform_code == 0 ) { // it seems to be an ANALYZE7.5 file
				LOG( ImageIoLog, info ) << "sform_code and qform_code are 0. Trying to use qto_xyz info!";
				retVec = qto;
			} else {
				LOG( ImageIoLog, error )
						<< "can't read orientation Vector for direction: " << dir + 1;
			}
		}

		return retVec;
	}

	template<typename T>
	void copyDataToNifti( const data::Image &image, nifti_image &ni ) {
		ni.data = malloc( image.getBytesPerVoxel() * image.getVolume() );
		T *refNii = ( T * ) ni.data;
		const util::FixedVector<size_t, 4> csize = image.getChunk( 0, 0 ).getSizeAsVector();
		const util::FixedVector<size_t, 4> isize = image.getSizeAsVector();
		const data::scaling_pair scale = image.getScalingTo( data::ValuePtr<T>::staticID );

		for ( size_t t = 0; t < isize[3]; t += csize[3] ) {
			for ( size_t z = 0; z < isize[2]; z += csize[2] ) {
				for ( size_t y = 0; y < isize[1]; y += csize[1] ) {
					for ( size_t x = 0; x < isize[0]; x += csize[0] ) {
						const size_t dim[] = {x, y, z, t};
						const data::Chunk ch = image.getChunkAs<T>( scale, x, y, z, t );
						T *target = refNii + image.getLinearIndex( dim );
						ch.getValuePtr<T>().copyToMem( 0, ch.getVolume() - 1, target );
					}
				}
			}
		}

		// data dependent information added
		ni.nbyper = image.getBytesPerVoxel();
		std::pair<double, double> minmax = image.getMinMaxAs<double>();
		ni.cal_min = minmax.first;
		ni.cal_max = minmax.second;
	}

	void geometryToNifti( const util::fvector4 &row, const util::fvector4 &column, const util::fvector4 &slice, const util::fvector4 &offset, mat44 &geo, const util::fvector4 &factor ) {
		for ( int y = 0; y < 3; y++ ) {
			geo.m[y][0] = row[y] * factor[0];
			geo.m[y][1] = column[y] * factor[1];
			geo.m[y][2] = slice[y] * factor[2];
			geo.m[y][3] = offset[y];
		}

	}
	void copyHeaderToNifti( const data::Image &image, nifti_image &ni ) {
		//all the other information for the nifti header
		BOOST_ASSERT( data::Image::dims == 4 );
		ni.scl_slope = 1.0;
		ni.scl_inter = 0.0;// TODO: ? http://209.85.135.104/search?q=cache:AxBp5gn9GzoJ:nifti.nimh.nih.gov/board/read.php%3Ff%3D1%26i%3D57%26t%3D57+nifti-1+scl_slope&hl=en&ct=clnk&cd=1&client=iceweasel-a

		if ( image.hasProperty( "InPlanePhaseEncodingDirection" ) ) {
			std::string phaseEncoding = ( image.getPropertyAs<std::string>( "InPlanePhaseEncodingDirection" ) );

			if ( phaseEncoding == "ROW" ) {
				ni.freq_dim = 2;
				ni.phase_dim = 1;
			}

			if ( phaseEncoding == "COL" ) {
				ni.freq_dim = 1;
				ni.phase_dim = 2;
			} else {
				ni.freq_dim = 0;
				ni.phase_dim = 0;
			}
		} else {
			ni.freq_dim = 1;
			ni.phase_dim = 2;
			ni.slice_dim = 3;
		}

		ni.xyz_units = NIFTI_UNITS_MM;
		ni.time_units = NIFTI_UNITS_MSEC;
		util::fvector4 dimensions = image.getSizeAsVector();
		LOG( ImageIoLog, info ) << dimensions;
		ni.ndim = ni.dim[0] = image.getRelevantDims();
		ni.nx = ni.dim[1] = dimensions[0];
		ni.ny = ni.dim[2] = dimensions[1];
		ni.nz = ni.dim[3] = dimensions[2];
		ni.nt = ni.dim[4] = dimensions[3];
		ni.nvox = image.getVolume();
		util::fvector4 rowVec = image.getPropertyAs<util::fvector4>( "rowVec" );
		util::fvector4 columnVec = image.getPropertyAs<util::fvector4>( "columnVec" );
		util::fvector4 sliceVec = image.getPropertyAs<util::fvector4>( "sliceVec" );
		util::fvector4 indexOrigin = image.getPropertyAs<util::fvector4>( "indexOrigin" );

		if( image.hasProperty( "nifti/qform_code" ) )
			ni.qform_code =  image.getPropertyAs<int>( "nifti/qform_code" );

		if( image.hasProperty( "nifti/sform_code" ) )
			ni.sform_code = image.getPropertyAs<int>( "nifti/sform_code" );

		// don't switch the z-AXIS!!!
		//indexOrigin[2] = -indexOrigin[2];
		LOG( ImageIoLog, info ) << indexOrigin;
		util::fvector4 voxelSizeVector = image.getPropertyAs<util::fvector4>( "voxelSize" );
		util::fvector4 voxelGap;

		if( image.hasProperty( "voxelGap" ) ) {
			voxelGap = image.getPropertyAs<util::fvector4>( "voxelGap" );
		}

		ni.dx = ni.pixdim[1] = voxelSizeVector[0] + voxelGap[0];
		ni.dy = ni.pixdim[2] = voxelSizeVector[1] + voxelGap[1];
		ni.dz = ni.pixdim[3] = voxelSizeVector[2] + voxelGap[2];
		ni.dt = ni.pixdim[4] = voxelSizeVector[3];

		if ( true == image.hasProperty( "sequenceDescription" ) ) {
			std::string descrip = ( image.getPropertyAs<std::string>( "sequenceDescription" ) );
			snprintf( ni.descrip, 80, "%s", descrip.c_str() );
		}

		if ( true == image.hasProperty( "StudyDescription" ) ) {
			std::string descrip = ( image.getPropertyAs<std::string>( "StudyDescription" ) );
			snprintf( ni.intent_name, 16, "%s", descrip.c_str() );
		}

		if ( image.hasProperty( "repetitionTime" ) ) {
			LOG( ImageIoLog, info ) << "Setting pixdim[" << ni.ndim << "] to " << image.getPropertyAs<uint16_t>( "repetitionTime" );
			ni.dt = ni.pixdim[ni.ndim+1] = ( float ) image.getPropertyAs<uint16_t>( "repetitionTime" ) / 1000; //nifti saves repTime s
		}

		//the rotation matrix
		//create space tranformation matrices - transforms the space when reading _NOT_ the data
		// thus ni.qto_xyz describes the orientation of the scanner space in the image space, not the orientation of image in the scanner space
		//      ni.sform_code = ni.qform_code = NIFTI_XFORM_SCANNER_ANAT;//set scanner aligned space from nifti1.h
		LOG( Debug, info ) << "rowVec " << rowVec << "  columnVec" << columnVec << "sliceVec" << sliceVec;


		if( ni.sform_code ) { // if sform_code was set - sform was used as geometry
			geometryToNifti( rowVec, columnVec, sliceVec, indexOrigin, ni.sto_xyz, voxelSizeVector + voxelGap );

			if( ni.qform_code ) { // if both was set - qform would be stored in "nifti/"
				geometryToNifti(
					image.getPropertyAs<util::fvector4>( "nifti/qrowVec" ),
					image.getPropertyAs<util::fvector4>( "nifti/qcolumnVec" ),
					image.getPropertyAs<util::fvector4>( "nifti/qsliceVec" ),
					indexOrigin, ni.qto_xyz, util::fvector4( 1, 1, 1 )
				);
			}
		} else { // if only qform code was set qform was used as geometry
			if( !ni.qform_code ) //if none was set, default qform to NIFTI_XFORM_SCANNER_ANAT
				ni.qform_code = NIFTI_XFORM_SCANNER_ANAT;

			geometryToNifti( rowVec, columnVec, sliceVec, indexOrigin, ni.qto_xyz, util::fvector4( 1, 1, 1 ) );
		}

		if( ni.qform_code ) {
			//generate matching quaternions
			nifti_mat44_to_quatern(
				ni.qto_xyz,
				&ni.quatern_b, &ni.quatern_c, &ni.quatern_d,
				&ni.qoffset_x, &ni.qoffset_y, &ni.qoffset_z,
				NULL, NULL, NULL,
				&ni.qfac );

			LOG( Debug, info ) << "ni.qto_xyz:" << util::listToString( ni.qto_xyz.m[0], ni.qto_xyz.m[0] + 3 );
			LOG( Debug, info ) << "ni.qto_xyz:" << util::listToString( ni.qto_xyz.m[1], ni.qto_xyz.m[1] + 3 );
			LOG( Debug, info ) << "ni.qto_xyz:" << util::listToString( ni.qto_xyz.m[2], ni.qto_xyz.m[2] + 3 );
			LOG( Debug, info ) << "ni.qfac:" << ni.qfac;
		}
	}



};//end class definition
}
}//namespace image_io isis


isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Nifti();
}
