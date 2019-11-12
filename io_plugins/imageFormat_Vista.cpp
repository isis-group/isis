/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive
 * and Brain Sciences, Leipzig.
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
 * Author: Thomas Proeger, proeger@cbs.mpg.de, 2010
 *
 *****************************************************************/

// local includes
#include "imageFormat_Vista.hpp"

// global includes
#include <list>
#include <sstream>
#include <algorithm>
#include <viaio/option.h>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_unsigned.hpp>

namespace isis
{

namespace image_io
{

// unfortunately VBit is not clearly defined - so adapt to what the system thinks it is
typedef std::conditional <
boost::is_unsigned<VBit>::type,
	  uint8_t,
	  int8_t
	  >::type vista_bitmask_type;


void
ImageFormat_Vista::write( const data::Image &image,
						  const std::string &filename, const util::istring &/*dialect*/,
						  std::shared_ptr<util::ProgressFeedback> progress )
throw( std::runtime_error & )
{
	LOG( Debug, info ) << "Writing image of size " << image.getSizeAsString() << " and type " << util::getTypeMap()[image.getMajorTypeID()] << " as vista";
	//  All vista images a organized in an attribue list. Let's create an empty one:
	VAttrList attrList = VCreateAttrList();
	//  One or more VImages need to be written to disk.
	VImage *vimages;
	// count number of images that where created with malloc
	int nimages = 0;
	//  get size for each dimension
	util::ivector4 dims = image.getSizeAsVector();

	//  create a vista image container according to the isis image configuration
	// 4D image data
	//  when we have got a 4D-image, this image provides functional data information
	if( dims[3] > 1 ) {
		LOG( Runtime, info ) << "Writing a functional vista image, so falling back to representation short!";
		data::TypedImage<VShort> shortImage( image );
		shortImage.spliceDownTo( data::sliceDim );

		vimages = ( VImage * )malloc( sizeof( VImage ) * dims[2] );
		nimages = dims[2];
		//we have to go through all slices and calculate the offset of the slicetimes
		std::list<float> acquisitionTimeList;

		for ( int z = 0; z < nimages; z++ ) {
			if( shortImage.getChunkAt( z ).hasProperty( "acquisitionTime" ) ) {
				acquisitionTimeList.push_back( shortImage.getChunkAt( z ).getValueAs<float>( "acquisitionTime" ) );
			}
		}

		acquisitionTimeList.sort();
		float sliceTimeOffset = acquisitionTimeList.front();

		if( progress )  progress->show( dims[2], "Creating functional vista image..." );

		for( int z = 0; z < dims[2]; z++ ) {
			vimages[z] = VCreateImage( dims[3], dims[1], dims[0], VShortRepn );

			for( int x = 0; x < dims[0]; x++ ) {
				for( int y = 0; y < dims[1]; y++ ) {
					for( int t = 0; t < dims[3]; t++ ) {
						VPixel( vimages[z], t, y, x, VShort )
						= shortImage.voxel<VShort>( x, y, z, t );
					}
				}
			}

			copyHeaderToVista( shortImage, vimages[z], sliceTimeOffset, true, z );
			VAppendAttr( attrList, "image", NULL, VImageRepn, vimages[z] );

			if( progress ) progress->progress();
		}

		if( progress ) progress->close();

		// dims[3] > 1 ?
		// 3D image data
	} else {
		// save 3D data to ONE vista image
		vimages = ( VImage * )malloc( sizeof( VImage ) );
		nimages = 1;

		// choose data type. The data type should be taken from the image, not
		// from the chunk since the chunks can have different types.
		switch( image.getMajorTypeID() ) {
			// VBit
		case data::ValueArray<bool>::staticID():
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VBitRepn );
			copyImageToVista<vista_bitmask_type>( image, vimages[0] );
			break;
			// VUByte
		case data::ValueArray<VUByte>::staticID():
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VUByteRepn );
			copyImageToVista<VUByte>( image, vimages[0] );
			break;
			// VSByte
		case data::ValueArray<VSByte>::staticID():
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VSByteRepn );
			copyImageToVista<VSByte>( image, vimages[0] );
			break;
			// VShort
		case data::ValueArray<u_int16_t>::staticID():
			LOG( Runtime, info ) << "Vista does not support " << util::Value<u_int16_t>::staticName() << ". Falling back to " << util::Value<VShort>::staticName();
		case data::ValueArray<VShort>::staticID():
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VShortRepn );
			copyImageToVista<VShort>( image, vimages[0] );
			break;
#if defined(_M_X64) || defined(__amd64__) && not defined (__APPLE__)
			// VLong
		case data::ValueArray<VLong>::staticID():
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VLongRepn );
			copyImageToVista<VLong>( image, vimages[0] );
			break;
#endif
			// VFloat
		case data::ValueArray<VFloat>::staticID():
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VFloatRepn );
			copyImageToVista<VFloat>( image, vimages[0] );
			break;
			// VDouble
		case data::ValueArray<VDouble>::staticID():
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VDoubleRepn );
			copyImageToVista<VDouble>( image, vimages[0] );
			break;
			// default error
		default:
			LOG( image_io::Runtime, error )
					<< "Can't map image type " << image.getChunk( 0 ).getTypeName() << "(" << image.getChunk( 0 ).getTypeID() << ") to vista type. Aborting" ;
			return;
		}

		// copy header information
		copyHeaderToVista( image, vimages[0], 0 , false );
		VAppendAttr( attrList, "image", NULL, VImageRepn, vimages[0] );
	}

	/*****************************************
	 * write history information if available
	 *****************************************/
	util::PropertyMap::KeyList keyset = image.getKeys();
	// count number of history entries
	size_t hcount = 0;
	// if history list prefix is available increase counter.
	for( util::PropertyMap::KeyList::key_type key :  keyset ) {
		if (  std::string( key.c_str() ).find( histPrefix ) != std::string::npos ) {
			hcount++;
		}
	}

	if( hcount > 0 ) {
		VAttrList hlist = VCreateAttrList();

		for( unsigned i = 1; i <= hcount; i++ ) {
			std::stringstream name;
			name << histPrefix << i;
			std::string val = image.getValueAs<std::string>( util::istring( name.str().c_str() ) );
			// split key token from history property string
			size_t x = val.find( ":" );
			VAppendAttr( hlist, val.substr( 0, x ).c_str(), NULL, VStringRepn,
						 val.substr( x + 1, val.size() ).c_str() );
		}

		// Prepend history list to attrlist
		VPrependAttr( attrList, "history", NULL, VAttrListRepn, hlist );
	}

	//  write to output file
	FILE *f;
	f = VOpenOutputFile( filename.c_str(), true );
	LOG_IF( ! f, ImageIoLog, error )
			<< "Error open file for writing." << util::MSubject( filename );
	bool written = VWriteFile( f, attrList );
	LOG_IF( !written, ImageIoLog, error )
			<< "Error writing image data.";

	//  cleanup
	for( int z = 0; z < nimages; z++ )
		VDestroyImage( vimages[z] );

	free( vimages );
	fclose( f );
}


std::list<data::Chunk> ImageFormat_Vista::load( const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> progress )
{
	// open input file
	FILE *ip;
	util::istring myDialect = dialect;

	if( !( ip = fopen( filename.c_str(), "r" ) ) ) {
		std::string s;
		s = "Error opening file " + filename + "for reading.";
		throwGenericError( s );
	}

	VAttrList list;
	VAttrListPosn hposn;
	VImage *images;
	// number of images (images loaded into a VistaChunk)
	unsigned nimages = 0;
	// number of VistaChunks loaded. Since every VImage is saved into a VistaChunk
	// nloaded gives the number of slices loaded so far.
	unsigned nloaded = 0;

	// read images from file stream
	if( ( nimages = VReadImages( ip, &list, &images ) ) == 0 ) {
		std::string s = "Error reading images from file " + filename;
		throwGenericError( s );
	}

	// enable "info" log level
	// image_io::enable_log<util::DefaultMsgPrint>( info );
	if( myDialect == "onlyfirst" ) {
		nimages = 1;
	}

	LOG( image_io::Runtime, info ) << "found " << nimages << " images.";
	/*****************************************
	 * Save vista file history if available
	 *****************************************/
	// Create an empty PropertyMap to store vista history related properties.
	// This map should be appended to every chunk in the output.
	util::PropertyMap hMap;
	VAttrList hist_list = VReadHistory( &list );

	if ( hist_list != NULL ) {
		unsigned int hcount = 0;
		VPointer val;

		for( VFirstAttr( hist_list, &hposn ); VAttrExists( &hposn ); VNextAttr( &hposn ) ) {
			// The vista file history will be saved in the Vista/HistoryLineXX elements
			// with XX as the index of the corresponding entry in the vista history list.
			if( VGetAttrValue( &hposn, NULL, VStringRepn, &val ) ) {
				const VString attrName = VGetAttrName( &hposn );
				std::stringstream key, value;
				key << histPrefix << ++hcount;
				value << attrName << ":" << std::string( ( VString ) val );
				hMap.setValueAs<std::string>( util::istring( key.str().c_str() ), value.str() );
			}
		}
	}

	/* interpred the image data structure according to the given dialects:
	 *
	 * FUNCTIONAL: The vista image contains functional data. In this case there
	 *             are a number of VShort images which represents the slices of
	 *             the functional image data. This dialect will create a 4-D image.
	 * MAP:    The vista image contains data which represents a statistical
	 *             map. In this case the first 3D VFloat images will be loaded
	 *             which represents the mapping layer.
	 * ANATOMICAL: The vista images contains presumably a number of anatomical
	 *             images. Each image will be saved in a seperate isis image
	 *             with the according data type.
	 */
	//if we have a vista image with functional data and one or more anatomical scans, we
	//can not reject the anatomical images. So we store them in a vector and handle them later.
	std::vector<VImage> residualVImages;
	std::vector<VImage> emptySlices;

	if( myDialect.empty() ) {
		if( nimages > 1 ) {
			//test for functional data
			size_t nShortRepn = 0;
			size_t nOtherRepn = 0;
			std::set<std::string> voxelSet;
			std::set<int> columnsSet;
			std::set<int> rowsSet;

			//if we have more than 1 short image with the same voxelsize, columnsize and rowsize
			//we assume a functional image
			for ( size_t k = 0; k < nimages; k++ ) {
				if( VPixelRepn( images[k] ) == VShortRepn ) {
					nShortRepn++;
					columnsSet.insert( VImageNColumns( images[k] ) );
					rowsSet.insert( VImageNRows( images[k] ) );

					//hack to detect images of size 1x1
					if( VImageNColumns( images[k] ) == 1 && VImageNRows( images[k] ) == 1 )  {
						emptySlices.push_back( images[k] );
					}

					VAttrList attributes = VImageAttrList( images[k] );
					VAttrListPosn posn;

					for( VFirstAttr( attributes, &posn ); VAttrExists( &posn ); VNextAttr( &posn ) ) {
						const char *name = VGetAttrName( &posn );
						VPointer val;

						if( strcmp( name, "voxel" ) == 0 ) {
							VGetAttrValue( &posn, NULL, VStringRepn, &val );
							voxelSet.insert( std::string( ( char * )val ) );
						}
					}
				} else {
					nOtherRepn++;
				}
			}

			if ( ( nShortRepn > 1 && voxelSet.size() == 1 ) && ( ( rowsSet.size() == 1 && columnsSet.size() == 1 ) || emptySlices.size() ) ) {
				LOG( isis::DataDebug, info ) << "Autodetect Dialect: Multiple VShort images found. Assuming a functional vista image";
				myDialect = "functional";
			} else {
				LOG( isis::DataDebug, info ) << "Autodetect Dialect: Multiple images found. Assuming a set of anatomical images";
			}
		} else {
			if( VPixelRepn( images[0] ) == VFloatRepn ) {
				LOG( isis::DataDebug, info ) << "Autodetect Dialect: VFloat image found. Assuming a statistical vista image";
				myDialect = "map";
			} else {
				LOG( isis::DataDebug, info ) << "Autodetect Dialect: Assuming an anatomical vista image";
			}
		}
	}

	// FUNCTIONAL -> copy every subimage into one chunk, splice the chunk
	// along the z-direction -> add all resulting chunks to the chunk list.
	if( myDialect == "functional" ) {
		char orient[100], voxelstr[100];
		orient[0] = '\0';
		voxelstr[0] = '\0';
		util::FixedVector<float, 3> v3;
		VPointer val;
		// index origin
		util::fvector3 indexOrigin;
		// traverse images and collect all VShort images.
		std::vector<VImage> vImageVector;

		for( unsigned int k = 0; k < nimages; k++ ) {
			if( VPixelRepn( images[k] ) != VShortRepn ) {
				residualVImages.push_back( images[k] );
			} else {
				vImageVector.push_back( images[k] );
			}
		}

		//############################# hack to support vista images that consist of slices with size   1x1x1 (to be honest....the whole plugin is a hack ;-)

		util::ivector4 dims( 0, 0, 0, 0 );

		for( unsigned int k = 0; k < vImageVector.size(); k++ ) {
			if( VImageNColumns( vImageVector[k] )  > 1 && VImageNRows( vImageVector[k] ) ) {
				dims[0] = VImageNColumns( vImageVector[k] );
				dims[1] = VImageNRows( vImageVector[k] );
				dims[2] = vImageVector.size();
				dims[3] = VImageNBands( vImageVector[k] );
				break;
			}
		}

		//remove the images with size 1x1 from the imageVector
		VAttrList refList = VImageAttrList( vImageVector.front() );
		for( std::vector< VImage >::const_reference oneVoxelImage :  emptySlices ) {
			std::vector< VImage >::iterator iter = std::find( vImageVector.begin(), vImageVector.end(), oneVoxelImage );
			vImageVector.erase( iter );
			VImage image = VCreateImage( dims[3], dims[1], dims[0], VShortRepn );
			VAttrList list = VImageAttrList( image );
			VAttrListPosn pos;

			for( VFirstAttr( refList, &pos ); VAttrExists( &pos ); VNextAttr( &pos ) ) {
				const char *name = VGetAttrName( &pos );
				VPointer val;

				if( ( strcmp( name, "orientation" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "orientation", NULL, VStringRepn, VString( val ) );
				}

				if( ( strcmp( name, "voxel" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "voxel", NULL, VStringRepn, VString( val ) );
				}

				if( ( strcmp( name, "repetition_time" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "repetition_time", NULL, VStringRepn, VString( val ) );
				}

				if( ( strcmp( name, "indexOrigin" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "indexOrigin", NULL, VStringRepn, VString( val ) );
				}

				if( ( strcmp( name, "rowVec" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "rowVec", NULL, VStringRepn, VString( val ) );
				}

				if( ( strcmp( name, "columnVec" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "columnVec", NULL, VStringRepn, VString( val ) );
				}

				if( ( strcmp( name, "sliceVec" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "sliceVec", NULL, VStringRepn, VString( val ) );
				}

				if( ( strcmp( name, "slice_time" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "slice_time", NULL, VStringRepn, VString( val ) );
				}

				if( ( strcmp( name, "ca" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "ca", NULL, VStringRepn, VString( val ) );
				}

				if( ( strcmp( name, "cp" ) == 0 ) ) {
					VGetAttrValue( &pos, NULL, VStringRepn, &val );
					VAppendAttr( list, "cp", NULL, VStringRepn, VString( val ) );
				}
			}

			//          VFillImage( image, 0, 0 );
			vImageVector.insert( iter, image );
		}

		std::list<VistaChunk<VShort> > vistaChunkList;
		//if we have no repetitionTime we have to calculate it with the help of the biggest slicetime
		uint16_t biggest_slice_time = 0;
		// the geometrical dimension of the 3D image according to the slice geometry
		// and the number of slices.


		std::set<util::fvector3, data::_internal::SortedChunkList::posCompare> originCheckSet;
		//first we have to create a vista chunkList so we can get the number of slices
		for( std::vector<VImage>::reference sliceRef :  vImageVector ) {
			VistaChunk<VShort> vchunk( sliceRef, true );
			vistaChunkList.push_back( vchunk );

			if( vchunk.hasProperty( "indexOrigin" ) ) {
				originCheckSet.insert( vchunk.getValueAs<util::fvector3>( "indexOrigin" ) );
			}

			if( vchunk.hasProperty( "acquisitionTime" ) && !vchunk.hasProperty( "repetition_time" ) ) {
				float currentSliceTime = vchunk.getValueAs<float>( "acquisitionTime" );

				if ( currentSliceTime > biggest_slice_time ) {
					float diff = currentSliceTime - biggest_slice_time;
					biggest_slice_time = currentSliceTime + diff;
				}
			}
		}

		if( progress ) progress->show( vistaChunkList.size() );

		for( std::vector<VistaChunk<VShort> >::reference sliceRef :  vistaChunkList ) {
			// increase slice counter
			nloaded++;
			uint16_t repetitionTime = 0;
			util::fvector3 ioprob;

			if( !sliceRef.hasProperty( "repetitionTime" ) && biggest_slice_time ) {
				sliceRef.setValueAs<uint16_t>( "repetitionTime", biggest_slice_time );
			}

			if( sliceRef.hasProperty( "repetitionTime" ) ) {
				repetitionTime = sliceRef.getValueAs<uint16_t>( "repetitionTime" );
			}

			// since functional data will be read first the sequence number
			// is 0.
			sliceRef.setValueAs<uint16_t>( "sequenceNumber", 0 );
			/********************* INDEX ORIGIN *********************
			 * Step 1: check if indexOrigin present.
			 * Step 2: Calculate indexOrigin according to the slice number
			 * (value of nloaded).
			 */

			// check if indexOrigin already present AND differs from slice to slice
			if( sliceRef.hasProperty( "indexOrigin" ) and ( originCheckSet.size() == vImageVector.size() ) ) {
				ioprob = sliceRef.getValueAs<util::fvector3>( "indexOrigin" );
			}
			// no indexOrigin present or does not differ from slice to slice -> Calculate index origin
			else {
				/******************** SET index origin ********************/
				// the index origin of each slice depends on the slice orientation
				// and voxel resolution. All chunks in the list splices are supposed
				// to have the same index origin since they are from the same slice.
				// get slice orientation of image
				VAttrList attributes = VImageAttrList( vImageVector[nloaded - 1] );
				VAttrListPosn posn;
				val = NULL;

				// Get orientation information
				for ( VFirstAttr( attributes, &posn ); VAttrExists( &posn ); VNextAttr( &posn ) ) {
					const VString name = VGetAttrName( &posn );

					if( strcmp( name, "orientation" ) == 0 ) {
						VGetAttrValue( &posn, NULL, VStringRepn, &val );
						break;
					}
				}

				// unusual error: there is no 'orientation' information in the vista image.
				if( val == NULL ) {
					throwGenericError( "Missing orientation information in functional data." );
				}

				// compare new orientation with old. Just to make sure that all subimages
				// have the same slice orientation.
				if( orient[0] == '\0' ) {
					strcpy( orient, ( char * )val );
				} else {
					// orientation string differs from previous value;
					if( strcmp( orient, ( char * )val ) != 0 )
						throwGenericError( "Inconsistent orienation information in functional data." );
				}

				// get voxel resolution
				val = NULL;

				for ( VFirstAttr( attributes, &posn ); VAttrExists( &posn ); VNextAttr( &posn ) ) {
					const VString name = VGetAttrName( &posn );

					if( strcmp( name, "voxel" ) == 0 ) {
						VGetAttrValue( &posn, NULL, VStringRepn, &val );
						break;
					}
				}

				// unusual error: there is no 'voxel' information in the vista image.
				if( val == NULL )
					throwGenericError( "Missing voxel information in functional data." );

				// compare new voxel resolution with old. Just to make sure that all subimages
				// have the same slice voxel resolution.
				if( voxelstr[0] == '\0' ) {
					strcpy( voxelstr, ( char * )val );
					std::list<float> buff = util::stringToList<float>( std::string( voxelstr ), ' ' );
					v3.copyFrom( buff.begin(), buff.end() );
				} else {
					// voxel string differs from previous value;
					if( strcmp( voxelstr, ( char * )val ) != 0 )
						throwGenericError( "Inconsistent voxel information in functional data." );
				}

				// set index origin to the coordinates of the n'th slice according to
				// the slice orientation. n is the index of the current subimage.
				// It's defined by the current value of nloaded.
				// Get indexOrigin from whole image with respect of the orientation
				// information. In general this should be the indexOrigin from the
				// (0,0,0,0) voxel.
				if ( !sliceRef.hasProperty( "indexOrigin" ) ) {
					ioprob = calculateIndexOrigin( sliceRef, dims );
				} else {
					ioprob = sliceRef.getValueAs<util::fvector3>( "indexOrigin" );
				}

				// correct the index origin according to the slice number and voxel
				// resolution
				// sagittal (x,y,z) -> (z,x,y)
				if( strcmp( orient, "sagittal" ) == 0 ) {
					LOG( DataLog, verbose_info ) << "computing ioprop with sagittal";
					ioprob[0] -= ( nloaded - 1 ) * v3[2];
				}
				// coronal (x,y,z) -> (x,-z,y)
				else if( strcmp( orient, "coronal" ) == 0 ) {
					LOG( DataLog, verbose_info ) << "computing ioprop with coronal";
					ioprob[1] -= ( nloaded - 1 ) * v3[2];
				}
				// axial (x,y,z) -> (x,y,z)
				else {
					LOG( DataLog, verbose_info ) << "computing ioprop with axial: += " <<  ( nloaded - 1 ) * v3[2];
					ioprob[2] -= ( nloaded - 1 ) * v3[2];
				}
			}

			// Set indexOrigin. This should be done before splicing.
			sliceRef.setValueAs( "indexOrigin", ioprob );
			/********************* SPLICE VistaChunk *********************
			 * With functional data the VistaChunk has the dimensions
			 * columns x rows x 1 x time. We splice the Chunk along the
			 * time axise to get time * (column x row x 1 x 1) chunks.
			 */
			// splice VistaChunk
			std::list<data::Chunk> splices = sliceRef.splice( data::sliceDim );
			/******************** SET acquisitionTime ********************/
			size_t timestep = 0;
			for( data::Chunk & spliceRef :  splices ) {
				uint32_t acqusitionNumber = ( nloaded - 1 ) + vImageVector.size() * timestep;
				spliceRef.setValueAs<uint32_t>( "acquisitionNumber", acqusitionNumber );

				if ( repetitionTime && sliceRef.hasProperty( "acquisitionTime" ) ) {
					float acquisitionTimeSplice = sliceRef.getValueAs<float>( "acquisitionTime" ) + ( repetitionTime * timestep );
					spliceRef.setValueAs<float>( "acquisitionTime", acquisitionTimeSplice );
				}

				// add history information
				spliceRef.join( hMap, true );
				timestep++;
			}
			LOG( DataLog, verbose_info ) << "adding " << splices.size() << " chunks to the output";
			/******************** add chunks to output ********************/
			std::back_insert_iterator<std::list<data::Chunk> > dest_iter ( chunks );
			std::copy( splices.begin(), splices.end(), dest_iter );

			if( progress ) progress->progress();
		} // END foreach vistaChunkList

		if ( progress ) progress->close();

		//handle the residual images
		uint16_t sequenceNumber = 0;
		for( std::vector<VImage>::reference vImageRef :  residualVImages ) {
			if( switchHandle( vImageRef, chunks ) ) {
				chunks.back().setValueAs<uint16_t>( "sequenceNumber", ++sequenceNumber );
				// add history information
				chunks.back().join( hMap, true );
				nloaded++;
			}
		}
	} // END if myDialect == "functional"

	// MAP -> the vista image should contain a single 3D VFloat image. Hence the
	// first image found will be saved in a float MemChunk and added to the output.
	else if( myDialect == "map" ) {
		// print a warning message when there are more than one image.
		LOG_IF( nimages >= 1, image_io::Runtime, warning )
				<< "Multiple images found. Will use the first VFloat image I can find.";

		// have a look for the first float image -> destroy the other images
		for( unsigned k = 0; k < nimages; k++ ) {
			if( ( VPixelRepn( images[k] ) != VFloatRepn ) || ( nloaded > 0 ) )
				VDestroyImage( images[k] );
			else {
				addChunk<VFloat>( chunks, images[k] );

				// check indexOrigin -> calculate default value if necessary
				if ( ! chunks.back().hasProperty( "indexOrigin" ) ) {
					util::ivector4 dims = chunks.back().getSizeAsVector();
					chunks.back().setValueAs<util::fvector3>( "indexOrigin",
							calculateIndexOrigin( ( chunks.back() ), dims ) );
				}

				// add history informations
				chunks.back().join( hMap, true );
				nloaded++;
			}
		}
	}
	// default: ANATOMICAL -> copy every image into a separate isis image with
	// the corresponding data type.
	else {
		for( unsigned k = 0; k < nimages; k++ ) {
			if( switchHandle( images[k], chunks ) ) {
				chunks.back().setValueAs<uint16_t>( "sequenceNumber", nloaded );

				// check indexOrigin -> calculate default value if necessary
				if ( ! chunks.back().hasProperty( "indexOrigin" ) ) {
					util::ivector4 dims = chunks.back().getSizeAsVector();
					chunks.back().setValueAs<util::fvector3>( "indexOrigin",
							calculateIndexOrigin( ( chunks.back() ), dims ) );
				}

				// add history information
				chunks.back().join( hMap, true );
				nloaded++;
			}
		}
	} // END else

	//  cleanup, close file handle
	fclose( ip );

	// ERROR: throw exception if there is no new chunk in the list
	if( !nloaded )
		throwGenericError ( "No images loaded" );

	LOG( Debug , info ) << nloaded << " images loaded.";
	return nloaded;
}

bool ImageFormat_Vista::switchHandle( VImage &image, std::list<data::Chunk> &chunks )
{
	switch( VPixelRepn( image ) ) {
	case VBitRepn:
		addChunk<vista_bitmask_type>( chunks, image );
		return true;
		break;
	case VUByteRepn:
		addChunk<VUByte>( chunks, image );
		return true;
		break;
	case VSByteRepn:
		addChunk<VSByte>( chunks, image );
		return true;
		break;
	case VShortRepn:
		addChunk<VShort>( chunks, image );
		return true;
		break;
#if defined(_M_X64) || defined(__amd64__) && not defined (__APPLE__)
	case VLongRepn:
		addChunk<VLong>( chunks, image );
		return true;
		break;
#endif
	case VFloatRepn:
		addChunk<VFloat>( chunks, image );
		return true;
		break;
	case VDoubleRepn:
		addChunk<VDouble>( chunks, image );
		return true;
		break;
	default:
		// discard images with unknown data type
		VDestroyImage( image );
		return false;
	}

	return false;
}

void ImageFormat_Vista::copyHeaderToVista( const data::Image &image, VImage &vimage, const float &sliceTimeOffset, const bool functional, size_t slice )
{
	// get attribute list from image
	VAttrList list = VImageAttrList( vimage );
	// ********** MANDATORY attributes **********
	// POLICY: copy all mandatory attributes
	// get voxel
	util::fvector3 voxels = image.getValueAs<util::fvector3>( "voxelSize" );
	util::fvector3 vGap = image.getValueAs<util::fvector3>( "voxelGap" );
	const float inf = std::numeric_limits<float>::infinity();

	// if vGap is valid the add it to the voxel resultion.
	if( vGap[0] != inf && vGap[1] != inf && vGap[2] && vGap[3] != inf ) {
		voxels = voxels + vGap;
	}

	std::stringstream vstr;
	vstr << voxels[0] << " " << voxels[1] << " " << voxels[2];
	VAppendAttr( list, "voxel", NULL, VStringRepn, vstr.str().c_str() );
	// copy orientation vectors
	util::fvector3 rowVec = image.getValueAs<util::fvector3>( "rowVec" );
	util::fvector3 columnVec = image.getValueAs<util::fvector3>( "columnVec" );
	util::fvector3 sliceVec = image.getValueAs<util::fvector3>( "sliceVec" );
	// set rowVec -> columnVec
	vstr.str( "" );
	vstr << rowVec[0] << " " << rowVec[1] << " " << rowVec[2];
	VAppendAttr( list, "columnVec", NULL, VStringRepn, vstr.str().c_str() );
	// set column -> rowVec
	vstr.str( "" );
	vstr << columnVec[0] << " " << columnVec[1] << " " << columnVec[2];
	VAppendAttr( list, "rowVec", NULL, VStringRepn, vstr.str().c_str() );
	// set sliceVec -> sliceVec
	vstr.str( "" );
	vstr << sliceVec[0] << " " << sliceVec[1] << " " << sliceVec[2];
	VAppendAttr( list, "sliceVec", NULL, VStringRepn, vstr.str().c_str() );
	// index origin
	util::fvector3 indexOrigin;

	if( functional ) {
		indexOrigin = image.getChunk( 0, 0, slice, 0 ).getValueAs<util::fvector3>( "indexOrigin" );
	} else {
		indexOrigin = image.getValueAs<util::fvector3>( "indexOrigin" );
	}

	vstr.str( "" );
	vstr << indexOrigin[0] << " " << indexOrigin[1] << " " << indexOrigin[2];
	VAppendAttr( list, "indexOrigin", NULL, VStringRepn, vstr.str().c_str() );

	// set slice orientation according to the image orientation
	switch( image.getMainOrientation() ) {
	case data::Image::axial:
	case data::Image::reversed_axial:
		VAppendAttr( list, "orientation", NULL, VStringRepn, "axial" );
		break;
	case data::Image::sagittal:
	case data::Image::reversed_sagittal:
		VAppendAttr( list, "orientation", NULL, VStringRepn, "sagittal" );
		break;
	case data::Image::coronal:
	case data::Image::reversed_coronal:
		VAppendAttr( list, "orientation", NULL, VStringRepn, "coronal" );
		break;
	}

	// ********** OPTIONAL **********
	// POLICY copy optional attributes

	// repetition time
	if( image.hasProperty( "repetitionTime" ) ) {
		VAppendAttr( list, "repetition_time", NULL, VShortRepn,
					 image.getValueAs<uint16_t>( "repetitionTime" ) );
	}

	//subject name
	if ( image.hasProperty( "subjectName" ) ) {
		VAppendAttr( list, "patient", NULL, VStringRepn,
					 ( VString ) image.getValueAs<std::string>( "subjectName" ).c_str() );
	}

	if ( image.hasProperty( "DICOM/ManufacturersModelName" ) ) {
		VAppendAttr( list, "device", NULL, VStringRepn,
					 ( VString ) image.getValueAs<std::string>( "DICOM/ManufacturersModelName" ).c_str() );
	}

	//  if( map.hasProperty( "acquisitionTime" ) && functional ) {
	//      VAppendAttr( list, "slice_time", NULL, VShortRepn,
	//                   map.getValueAs<int16_t>( "acquisitionTime" ) );
	//  }
	if ( functional ) {
		// Deriving slice time from acquisition time. This is only ok if we have
		// timing information for every slice. Hence the cunks should be split to
		// 2-D slices.

		// Check if the current chunk encodes more than one slice. This
		// is only valid if there is more than one slice in the isis image.
		if( image.getChunk( slice ).getSizeAsVector()[2] > 1 ) {
			LOG( data::Runtime, error ) << "Chunk contains more than one slice."
										<< "Interpolation of slice time is not possible.";
		} else {
			// See if there is an acquisition time available
			if ( image.getChunkAt( slice ).hasProperty( "acquisitionTime" ) ) {
				std::stringstream sstream;
				float stime;
				stime = image.getChunkAt( slice ).getValueAs<float>( "acquisitionTime" );
				stime -= sliceTimeOffset;
				sstream << stime;
				VAppendAttr ( list, "slice_time", NULL, VStringRepn, sstream.str().c_str() );
			}
			// It's not safe to guess the slice order. If there is no acquisition time
			// then there is no slice_time attribute in vista image.
			else {
				LOG( data::Runtime, info ) << "Missing acquisition time. "
										   << "Interpolation of slice time is not supported.";
			}
		}
	}

	if ( image.hasProperty( "subjectGender" ) ) {
		util::Selection genderSelection = image.getValueAs<util::Selection>( "subjectGender" );
		std::string gender = genderSelection;
		VAppendAttr( list, "sex", NULL, VStringRepn, ( VString ) gender.c_str() );
	}

	if ( image.hasProperty( "echoTime" ) ) {
		VAppendAttr( list, "echoTime", NULL, VFloatRepn, ( VFloat ) image.getValueAs<float>( "echoTime" ) );
	}

	if ( image.hasProperty( "flipAngle" ) ) {
		VAppendAttr( list, "flipAngle", NULL, VShortRepn, ( VShort ) image.getValueAs<uint16_t>( "flipAngle" ) );
	}

	if ( image.hasProperty( "transmitCoil" ) ) {
		VAppendAttr( list, "transmitCoil", NULL, VStringRepn, ( VString ) image.getValueAs<std::string>( "transmitCoil" ).c_str() );
	}

	if ( image.hasProperty( "sequenceStart" ) ) {
		boost::posix_time::ptime isisTime = image.getValueAs<boost::posix_time::ptime>( "sequenceStart" );
		boost::gregorian::date isisDate = isisTime.date();
		boost::posix_time::time_duration isisTimeDuration = isisTime.time_of_day();
		VAppendAttr( list, "date", NULL, VStringRepn, boost::gregorian::to_simple_string( isisDate ).c_str() );
		VAppendAttr( list, "time", NULL, VStringRepn, ( VString ) boost::posix_time::to_simple_string( isisTimeDuration ).c_str() );
	}

	if ( image.hasProperty( "subjectAge" ) ) {
		// age in days
		uint16_t age = image.getValueAs<uint16_t>( "subjectAge" );
		age = ( ( age / 365.2425 ) - floor( age / 365.2425 ) ) < 0.5 ?
			  floor( age / 365.2425 ) : ceil( age / 365.2425 );
		VAppendAttr( list, "age", NULL, VShortRepn, age );
	}

	// ********** Vista group **********
	// POLICY: Append ALL properties from the 'Vista' Propmap to the end of the
	// vista attribute list.
	// EXCEPTION: ignore history attributes with the form "Vista/HistoryLineXX,
	// with XX is the index number of the history entry in the vista file

	if( image.hasBranch( "Vista" ) ) {
		util::PropertyMap vista_branch = image.branch( "Vista" );
		// convert it to a property map
		util::PropertyMap::KeyList klist = vista_branch.getKeys();
		util::PropertyMap::KeyList::const_iterator kiter;
		// prefix of history entries
		std::string hpref = "HistoryLine";

		for( kiter = klist.begin(); kiter != klist.end(); kiter++ ) {
			// skip entry from vista image history
			if( (  std::string ( ( *kiter ).c_str() ) ).find( hpref ) != std::string::npos ) {
				continue;
			}

			// get property value
			util::PropertyValue pv = vista_branch.property( *kiter );
			// VBit -> VBit (char *)
			static_assert( sizeof( char ) == sizeof( uint8_t ) );

			if( pv.is<uint8_t>() ) {
				VAppendAttr( list, ( *kiter ).c_str(), NULL, VBitRepn,
							 ( VBit )pv.castTo<uint8_t>() );
				continue;
			}

			// VUByte -> VUByte (char *)
			if( pv.is<VUByte>() ) {
				VAppendAttr( list, ( *kiter ).c_str(), NULL, VUByteRepn,
							 pv.castTo<VUByte>() );
				continue;
			}

			// VSByte -> VSByte (char *)
			if( pv.is<VSByte>() ) {
				VAppendAttr( list, ( *kiter ).c_str(), NULL, VSByteRepn,
							 pv.castTo<VSByte>() );
				continue;
			}

			// VShort -> VShort (char *)
			if( pv.is<VShort>() ) {
				VAppendAttr( list, ( *kiter ).c_str(), NULL, VShortRepn,
							 pv.castTo<VShort>() );
				continue;
			}

#if defined(_M_X64) || defined(__amd64__) && not defined(__APPLE__)

			// VLong -> VLong (char *)
			if( pv.is<VLong>() ) {
				VAppendAttr( list, ( *kiter ).c_str(), NULL, VLongRepn,
							 pv.castTo<VLong>() );
				continue;
			}

#endif

			// VFloat -> VFloat (char *)
			if( pv.is<VFloat>() ) {
				VAppendAttr( list, ( *kiter ).c_str(), NULL, VFloatRepn,
							 pv.castTo<VFloat>() );
				continue;
			}

			// VDouble -> VDouble (char *)
			if( pv.is<VDouble>() ) {
				VAppendAttr( list, ( *kiter ).c_str(), NULL, VDoubleRepn,
							 pv.castTo<VDouble>() );
				continue;
			}

			// VString -> std::string
			if( pv.is<std::string>() ) {
				VAppendAttr( list, ( *kiter ).c_str(), NULL, VStringRepn,
							 pv.castTo<std::string>().c_str() );
				continue;
			}
		}
	}
}

template <typename TInput> void ImageFormat_Vista::addChunk( std::list< isis::data::Chunk >& chunks, VImage image )
{
	static uint16_t sNum = 0;
	chunks.push_back( VistaChunk<TInput>( image, false ) );
	chunks.back().setValueAs( "sequenceNumber", sNum++ );// some voodoo to fake a sequenceNumber
}

template <typename T> bool ImageFormat_Vista::copyImageToVista( const data::Image &image, VImage &vimage )
{
	const util::vector4<size_t> csize = image.getChunk( 0, 0 ).getSizeAsVector();
	const util::vector4<size_t> isize = image.getSizeAsVector();
	LOG_IF( isize[3] > 1, Debug, error ) << "Vista cannot store 4D-Data in one VImage.";
	const data::scaling_pair scale = image.getScalingTo( data::ValueArray<T>::staticID() );

	for ( size_t z = 0; z < isize[2]; z += csize[2] ) {
		for ( size_t y = 0; y < isize[1]; y += csize[1] ) {
			for ( size_t x = 0; x < isize[0]; x += csize[0] ) {
				data::Chunk ch = image.getChunkAs<T>( scale, x, y, z, 0 );
				ch.getValueArray<T>().copyToMem( &VPixel( vimage, z, y, x, T ), csize.product() );
			}
		}
	}

	return true;
}

util::fvector3 ImageFormat_Vista::calculateIndexOrigin( isis::data::Chunk &chunk, isis::util::ivector4 &dims )
{
	// IMPORTANT: We don't use the dims from the chunks since we are not sure if
	// if the 3rd dimension contains geometrical or time information. Hence it's
	// neccessary to provide image dimensional informations via a function
	// parameter.
	util::fvector3 voxels = chunk.getValueAs<util::fvector3>( "voxelSize" );
	// calculate index origin according to axial
	util::fvector3 ioTmp(
		-( ( dims[0] - 1 )*voxels[0] ) / 2,
		-( ( dims[1] - 1 )*voxels[1] ) / 2,
		-( ( dims[2] - 1 )*voxels[2] ) / 2 );
	util::fvector3 readV = chunk.getValueAs<util::fvector3>( "rowVec" );
	util::fvector3 phaseV = chunk.getValueAs<util::fvector3>( "columnVec" );
	util::fvector3 sliceV = chunk.getValueAs<util::fvector3>( "sliceVec" );
	// multiply indexOrigin with read, column and slice vector
	util::fvector3 iOrig(
		readV[0] * ioTmp[0] + phaseV[0] * ioTmp[1] + sliceV[0] * ioTmp[2],
		readV[1] * ioTmp[0] + phaseV[1] * ioTmp[1] + sliceV[1] * ioTmp[2],
		readV[2] * ioTmp[0] + phaseV[2] * ioTmp[1] + sliceV[2] * ioTmp[2] );
	return iOrig;
}

}//namespace image_io
}//namespace isis

isis::image_io::FileFormat *
factory()
{
	return new isis::image_io::ImageFormat_Vista();
}
