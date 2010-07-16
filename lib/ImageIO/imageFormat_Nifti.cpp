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
#include "DataStorage/io_interface.h"
#include "CoreUtils/type.hpp"
#include "common.hpp"
#include "CoreUtils/vector.hpp"
#include "DataStorage/typeptr.hpp"

//SYSTEM INCLUDES
#include <nifti1_io.h>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/regex.hpp>

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

public:
	enum vectordirection {readDir = 0, phaseDir, sliceDir, indexOrigin, voxelSizeVec};

	std::string suffixes() {
		return std::string( ".nii.gz .nii .hdr" );
	}

	std::string dialects() {
		return std::string( "fsl spm" );
	}

	std::string name() {
		//TODO: wahrscheinlich sollten die Namen irgendwie so aussehen "mpg.cbs.nii"?
		return "Nifti";
	}

	bool tainted() {
		return false;
	}//internal plugins are not tainted

	/***********************
	 * load file
	 ************************/
	int load( data::ChunkList &retList, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		//read the file with the function from nifti1_io.h
		nifti_image *ni = nifti_image_read( filename.c_str(), true );

		if ( not ni )
			throwGenericError( "nifti_image_read(" + filename + ") failed" );

		// 0.0 would mean "not in use" - so for better handling use a 1.0
		float scale = ni->scl_slope ? ni->scl_slope : 1.0;

		// TODO: at the moment scaling not supported due to data type changes
		if ( 1.0 != scale )
			throwGenericError( std::string( "Scaling is not supported at the moment. Scale Factor: " ) + util::Type<float>( scale ).toString() );

		LOG( ImageIoDebug, isis::info ) << "datatype to load from nifti " << ni->datatype;
		boost::shared_ptr<data::Chunk> retChunk;
		Deleter del( ni, filename );

		switch ( ni->datatype ) {
		case DT_UINT8:
			retChunk.reset( new _internal::NiftiChunk( static_cast<uint8_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) );
			break;
		case DT_INT8:
			retChunk.reset( new _internal::NiftiChunk( static_cast<int8_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) );
			break;
		case DT_INT16:
			retChunk.reset( new _internal::NiftiChunk( static_cast<int16_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) );
			break;
		case DT_UINT16:
			retChunk.reset( new _internal::NiftiChunk( static_cast<uint16_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) );
			break;
		case DT_UINT32:
			retChunk.reset( new _internal::NiftiChunk( static_cast<uint32_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) );
			break;
		case DT_INT32:
			retChunk.reset( new _internal::NiftiChunk( static_cast<int32_t *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) );
			break;
		case DT_FLOAT32:
			retChunk.reset( new _internal::NiftiChunk( static_cast<float *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) );
			break;
		case DT_FLOAT64:
			retChunk.reset( new _internal::NiftiChunk( static_cast<double *>( ni->data ), del, ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) );
			break;
		default:
			throwGenericError( std::string( "Unsupported datatype " ) + util::Type<int>( ni->datatype ).toString() );
		}

		// don't forget to take the properties with
		copyHeaderFromNifti( *retChunk, *ni );
		//      if ( !strcasecmp( dialect.c_str(), "spm" ) )
		//      {
		//          boost::shared_ptr<data::MemChunk<int16_t> >
		//              dst( new data::MemChunk<int16_t>( ni->dim[1], ni->dim[2], ni->dim[3], ni->dim[4] ) ) ;
		//          retChunk->swapAlong( *dst, 0 );
		//          retList.push_back( *dst );
		//          return 1;
		//      }
		// push the completed NiftiChunk into the list
		retList.push_back( *retChunk );
		return retChunk ? 1 : 0;
	}


	/***********************
	 * write file
	 ************************/
	void write( const data::Image &imageOrig, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & ) {
		LOG( ImageIoDebug, isis::info ) << "Write Nifti.";
		boost::filesystem::path boostFilename( filename );
		//copy of our image due to changing it by transformCoords
		isis::data::Image image( imageOrig );
		//orientation in isis LPS - but in nifti everything relative to RAS - so let's change read/phase direction and sign of indexOrigin
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
		//default init for nifti image
		nifti_image ni;
		memset( &ni, 0, sizeof( nifti_image ) ); //set everything to zero - default value for "not used"
		ni.nu = ni.nv = ni.nw = 1;
		ni.datatype = DT_UNKNOWN;
		ni.data = NULL;
		ni.fname = const_cast<char *>( filename.c_str() );
		// get dim info from image
		util::fvector4 dimensions = image.sizeToVector();
		//set the props from the image to the nifti file
		copyHeaderToNifti( image, ni );
		// set filename for resulting image(s) due to Analyze vs. Nifti
		ni.fname = const_cast<char *>( filename.c_str() ); // header name
		boost::filesystem::path imgname;

		if ( "hdr" == extension( boostFilename ) ) {
			ni.nifti_type = 0; // that's ANALYZE ID
			imgname  =  change_extension( boostFilename, ".img" );
			ni.iname = const_cast<char *>( imgname.string().c_str() );
		} else {
			ni.nifti_type = 1; // that's NIFTI ID
			ni.iname = const_cast<char *>( filename.c_str() );
		}

		// FSL compatibility
		if ( strcasecmp( dialect.c_str(), "fsl" ) ) {
			//  stuffFslCompatibility(image, ni);
		}

		// copy the data to the nifti image
		LOG( ImageIoLog, isis::info ) << image.getChunk( 0, 0, 0, 0 ).typeName();

		switch ( image.getChunk( 0, 0, 0, 0 ).typeID() ) {
		case data::TypePtr<int8_t>::staticID:

			if ( 0 == strcasecmp( dialect.c_str(), "fsl" ) ) { // fsl not compatible with int8, convert to uint8
				data::MemImage<u_int8_t> fslCopy( image );
				ni.datatype = DT_UINT8;
				copyDataToNifti<u_int8_t>( fslCopy, ni );
				break;
			}

			ni.datatype = DT_INT8;
			copyDataToNifti<int8_t>( image, ni );
			break;
		case data::TypePtr<u_int8_t>::staticID:
			ni.datatype = DT_UINT8;
			copyDataToNifti<u_int8_t>( image, ni );
			break;
		case data::TypePtr<int16_t>::staticID:
			ni.datatype = DT_INT16;
			copyDataToNifti<int16_t>( image, ni );
			break;
		case data::TypePtr<u_int16_t>::staticID:

			if ( 0 == strcasecmp( dialect.c_str(), "fsl" ) ) {
				image.print( std::cout );
				data::MemImage<int16_t> fslCopy( image );
				fslCopy.print( std::cout << "Copy (" << fslCopy.chunksBegin()->typeName() << ")" << std::endl );
				ni.datatype = DT_INT16;
				copyDataToNifti<int16_t>( fslCopy, ni );
				break;
			}

			ni.datatype = DT_UINT16;
			copyDataToNifti<u_int16_t>( image, ni );
			break;
		case data::TypePtr<int32_t>::staticID:
			ni.datatype = DT_INT32;
			copyDataToNifti<int32_t>( image, ni );
			break;
		case data::TypePtr<u_int32_t>::staticID:

			if ( 0 == strcasecmp( dialect.c_str(), "fsl" ) ) {
				data::MemImage<int32_t> fslCopy( image );
				ni.datatype = DT_INT32;
				copyDataToNifti<int32_t>( image, ni );
				break;
			}

			ni.datatype = DT_UINT32;
			copyDataToNifti<u_int16_t>( image, ni );
			break;
		case data::TypePtr<float>::staticID:
			ni.datatype = DT_FLOAT32;
			copyDataToNifti<float>( image, ni );
			break;
		case data::TypePtr<double>::staticID:
			ni.datatype = DT_FLOAT64;
			copyDataToNifti<double>( image, ni );
			break;
		default:
			throwGenericError( "Datatype " + data::TypePtr<float>::staticName() + " cannot be written!" );
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

	void copyHeaderFromNifti( data::Chunk &retChunk, const nifti_image &ni ) {
		util::fvector4 dimensions( ni.dim[1], ni.ndim >= 2 ? ni.dim[2] : 1,
								   ni.ndim >= 3 ? ni.dim[3] : 1, ni.ndim >= 4 ? ni.dim[4] : 1 );
		LOG( ImageIoLog, info ) << "size of chunk " << dimensions << "/" << ni.ndim;

		for ( int t = 0; t < dimensions[3]; t++ ) {
			util::fvector4 offsets( ni.qoffset_x, ni.qoffset_y, ni.qoffset_z, 0 );
			//retChunk.setProperty("acquisitionTime", );
			retChunk.setProperty( "acquisitionNumber", t );
			retChunk.setProperty<u_int16_t>( "sequenceNumber", 1 );
			// in nifti everything should be relative to RAS, in isis we use LPS coordinates - normally change read/phase dir and sign of indexOrigin
			//TODO: has to be tested with different niftis - don't trust them!!!!!!!!!
			retChunk.setProperty( "indexOrigin", util::fvector4( ni.qoffset_x, ni.qoffset_y, ni.qoffset_z, 0 ) );
			retChunk.setProperty( "readVec",  getVector( ni, readDir ) );
			retChunk.setProperty( "phaseVec", getVector( ni, phaseDir ) );
			retChunk.setProperty( "sliceVec", getVector( ni, sliceDir ) );
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
			retChunk.setProperty( "voxelSize", getVector( ni, voxelSizeVec ) );
			retChunk.setProperty( "sequenceDescription", std::string( ni.descrip ) );
			retChunk.setProperty( "StudyDescription", std::string( ni.intent_name ) );

			if ( ( 2 == ni.freq_dim ) and ( 1 == ni.phase_dim ) ) {
				retChunk.setProperty<std::string>( "InPlanePhaseEncodingDirection", "ROW" );
			} else if ( ( 1 == ni.freq_dim ) and ( 2 == ni.phase_dim ) ) {
				retChunk.setProperty<std::string>( "InPlanePhaseEncodingDirection", "COL" );
			}

			retChunk.setProperty( "voxelGap", util::fvector4() ); // not extra included in Nifti, so set to zero
			//just some LOGS
			LOG( ImageIoLog, info ) << "dims at all " << dimensions;
			LOG( ImageIoLog, info ) << "Offset values from nifti" << offsets;
			LOG( ImageIoLog, info ) << "FOV read/phase/slice/voxelsize:"
									<< getVector( ni, readDir )
									<< " / " << getVector( ni, phaseDir )
									<< " / " << getVector( ni, sliceDir )
									<< getVector( ni, voxelSizeVec );
		}

		//check repetition time
		//check description for TR=[[:digit:]]ms.
		boost::regex descriptionRegex( ".*TR=([[:digit:]]{1,})ms.*" );
		boost::cmatch results;
		u_int16_t tr = 0;
		std::string description = ni.descrip;

		if ( boost::regex_match( ni.descrip, results,  descriptionRegex ) ) {
			tr = boost::lexical_cast<u_int16_t>( results.str( 1 ) );
		}

		//if "TR=" was found in description and differs from pixdim[dim]
		if( tr && ( !tr == ni.pixdim[ni.ndim] ) ) {
			LOG( ImageIoLog, warning ) << "TR=" << tr << " was found in description but differs from pixdim[4]= "
									   << ni.pixdim[ni.ndim] << ". This seems to be a nifti coming from SPM. Setting repetitionTime to " << tr
									   << " ms. During conversion this value can be changed by using the parameter -tr";
			retChunk.setProperty<u_int16_t>( "repetitionTime", tr );
		}

		//if "TR=" was not found in description pixdimg[dim] == 0 a warning calls attention to use parameter -tr to change repetitionTime.
		if( tr == 0 && ni.pixdim[ni.ndim] == 0 ) {
			LOG( ImageIoLog, warning ) << "Repetition time seems to be invalid. To set the repetition time during conversion use the parameter -tr ";
			retChunk.setProperty<u_int16_t>( "repetitionTime", 0 );
		}

		if( !tr && ni.pixdim[ni.ndim] ) {
			retChunk.setProperty<u_int16_t>( "repetitionTime", ni.pixdim[ni.ndim] );
		}
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
		ni.data = malloc( image.bytes_per_voxel() * image.volume() );
		T *refNii = ( T * ) ni.data;
		const util::FixedVector<size_t, 4> csize = image.getChunk( 0, 0 ).sizeToVector();
		const util::FixedVector<size_t, 4> isize = image.sizeToVector();

		for ( size_t t = 0; t < isize[3]; t += csize[3] ) {
			for ( size_t z = 0; z < isize[2]; z += csize[2] ) {
				for ( size_t y = 0; y < isize[1]; y += csize[1] ) {
					for ( size_t x = 0; x < isize[0]; x += csize[0] ) {
						const size_t dim[] = {x, y, z, t};
						const data::Chunk ch = image.getChunk( x, y, z, t );
						T *target = refNii + image.dim2Index( dim );
						ch.getTypePtr<T>().copyToMem( 0, ch.volume() - 1, target );
					}
				}
			}
		}

		// data dependent information added
		ni.nbyper = image.bytes_per_voxel();
		image.getMinMax( ni.cal_min, ni.cal_max );
	}

	void copyHeaderToNifti( const data::Image &image, nifti_image &ni ) {
		//all the other information for the nifti header
		BOOST_ASSERT( data::Image::n_dims == 4 );
		ni.scl_slope = 1.0;
		ni.scl_inter = 0.0;// TODO: ? http://209.85.135.104/search?q=cache:AxBp5gn9GzoJ:nifti.nimh.nih.gov/board/read.php%3Ff%3D1%26i%3D57%26t%3D57+nifti-1+scl_slope&hl=en&ct=clnk&cd=1&client=iceweasel-a

		if ( image.hasProperty( "InPlanePhaseEncodingDirection" ) ) {
			std::string phaseEncoding = ( image.getProperty<std::string>( "InPlanePhaseEncodingDirection" ) );

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
		util::fvector4 dimensions = image.sizeToVector();
		LOG( ImageIoLog, info ) << dimensions;
		ni.ndim = ni.dim[0] = image.relevantDims();
		ni.nx = ni.dim[1] = dimensions[0];
		ni.ny = ni.dim[2] = dimensions[1];
		ni.nz = ni.dim[3] = dimensions[2];
		ni.nt = ni.dim[4] = dimensions[3];
		ni.nvox = image.volume();
		util::fvector4 readVec = image.getProperty<util::fvector4>( "readVec" );
		util::fvector4 phaseVec = image.getProperty<util::fvector4>( "phaseVec" );
		util::fvector4 sliceVec = image.getProperty<util::fvector4>( "sliceVec" );
		util::fvector4 indexOrigin = image.getProperty<util::fvector4>( "indexOrigin" );
		// don't switch the z-AXIS!!!
		//indexOrigin[2] = -indexOrigin[2];
		LOG( ImageIoLog, info ) << indexOrigin;
		util::fvector4 voxelSizeVector = image.getProperty<util::fvector4>( "voxelSize" );
		util::fvector4 voxelGap = image.getProperty<util::fvector4>( "voxelGap" );
		ni.dx = ni.pixdim[1] = voxelSizeVector[0] + voxelGap[0];
		ni.dy = ni.pixdim[2] = voxelSizeVector[1] + voxelGap[1];
		ni.dz = ni.pixdim[3] = voxelSizeVector[2] + voxelGap[2];
		ni.dt = ni.pixdim[4] = voxelSizeVector[3];

		if ( true == image.hasProperty( "sequenceDescription" ) ) {
			std::string descrip = ( image.getProperty<std::string>( "sequenceDescription" ) );
			snprintf( ni.descrip, 80, "%s", descrip.c_str() );
		}

		if ( true == image.hasProperty( "StudyDescription" ) ) {
			std::string descrip = ( image.getProperty<std::string>( "StudyDescription" ) );
			snprintf( ni.intent_name, 16, "%s", descrip.c_str() );
		}

		if ( image.hasProperty( "repetitionTime" ) ) {
			LOG( ImageIoLog, info ) << "Setting pixdim[" << ni.ndim << "] to " << image.getProperty<u_int16_t>( "repetitionTime" );
			ni.dt = ni.pixdim[ni.ndim+1] = image.getProperty<u_int16_t>( "repetitionTime" );
		}

		//the rotation matrix
		//create space tranformation matrices - transforms the space when reading _NOT_ the data
		// thus ni.qto_xyz describes the orientation of the scanner space in the image space, not the orientation of image in the scanner space
		ni.sform_code = ni.qform_code = NIFTI_XFORM_SCANNER_ANAT;//set scanner aligned space from nifti1.h
		LOG( ImageIoLog, info ) << "ReadVec " << readVec << "  phaseVec" << phaseVec << "sliceVec" << sliceVec;

		for ( int y = 0; y < 3; y++ ) {
			ni.qto_xyz.m[y][0] = readVec[y];
			ni.qto_xyz.m[y][1] = phaseVec[y];
			ni.qto_xyz.m[y][2] = sliceVec[y];
			ni.qto_xyz.m[y][3] = indexOrigin[y];
		}

		memcpy( ni.sto_xyz.m, ni.qto_xyz.m, sizeof( ni.sto_xyz.m ) );

		//add scaling to the sform
		for ( int y = 0; y < 3; y++ ) {
			ni.sto_xyz.m[0][y] *= voxelSizeVector[y] + voxelGap[y];
			ni.sto_xyz.m[1][y] *= voxelSizeVector[y] + voxelGap[y];
			ni.sto_xyz.m[2][y] *= voxelSizeVector[y] + voxelGap[y];
		}

		//generate matching quaternions
		nifti_mat44_to_quatern(
			ni.qto_xyz,
			&ni.quatern_b, &ni.quatern_c, &ni.quatern_d,
			&ni.qoffset_x, &ni.qoffset_y, &ni.qoffset_z,
			NULL, NULL, NULL,
			&ni.qfac );
	}



};//end class definition
}
}//namespace image_io isis


isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Nifti();
}
