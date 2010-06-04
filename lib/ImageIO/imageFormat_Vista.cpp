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
 * Author: Thomas Pr��ger, proeger@cbs.mpg.de, 2010
 *
 *****************************************************************/

// local includes
#include "imageFormat_Vista.h"

// global includes
#include <list>
#include <sstream>

namespace isis
{

namespace image_io
{

void
ImageFormat_Vista::write( const data::Image &image,
						  const std::string &filename, const std::string &dialect )
throw( std::runtime_error & )
{
	//  All vista images a organized in an attribue list. Let's create an empty one:
	VAttrList attrList = VCreateAttrList();
	//  One or more VImages need to be written to disk.
	VImage *vimages;
    // count number of images that where created with malloc
	int nimages = 0;
	//  get size for each dimension
	util::ivector4 dims = image.sizeToVector();
	//  create a vista image container according to the isis image configuration

	// 4D image data
	//  when we have got a 4D-image, this image provides functional data information
	if( dims[3] > 1 ) {
		vimages = ( VImage * )malloc( sizeof( VImage ) * dims[2] );
		nimages = dims[2];
		// since we need to convert and reorganize the data, we will create a
		// temporary buffer who stores short values.
		// get the first chunk
		data::MemChunk<VShort> mchunk( image.getChunk( 0 ) );

		// see if ALL the data is included in the first chunk.
		// Otherwise, iterate over chunks and include the data.
		if( mchunk.sizeToVector()[3] > 1 ) {
			// reorganize data: (x,y,z,t) -> z images with (x,y,t)
			// in other words: there are z VImages necessary.
			for( int z = 0; z < dims[2]; z++ ) {
				vimages[z] = VCreateImage( dims[3], dims[1], dims[0],
										   VShortRepn );

				// get all data from the first chunk.
				for( int x = 0; x < dims[0]; x++ ) {
					for( int y = 0; y < dims[1]; y++ ) {
						for( int t = 0; t < dims[3]; t++ ) {
							VPixel( vimages[z], t, y, x, VShort )
							= mchunk.voxel<VShort>( x, y, z, t );
						}
					}
				}

				// copy header information
				copyHeaderToVista(image,vimages[z]);

				VAppendAttr( attrList, "image", NULL, VImageRepn, vimages[z] );
			}
		}
		// the 4th dimension ist distributed over the number of chunks
		else {
			// allocate images
			for( int z = 0; z < dims[2]; z++ ) {
				vimages[z] = VCreateImage( dims[3], dims[1], dims[0],
										   VShortRepn );

				// copy header information
				copyHeaderToVista(image,vimages[z]);

				VAppendAttr( attrList, "image", NULL, VImageRepn, vimages[z] );
			}

			// Iterate of set of chunks in the input image
			isis::data::Image::ConstChunkIterator chIter;
			// count current timestep
			int t = 0;

			for( chIter = image.chunksBegin(); chIter != image.chunksEnd(); chIter++ ) {
				// next timestep
				// put the chunk in a mem chunk to get type convertion right
				data::MemChunk<VShort> chunk( *chIter );

				for( int z = 0; z < dims[2]; z++ ) {
					for( int y = 0; y < dims[1]; y++ ) {
						for( int x = 0; x < dims[0]; x++ ) {
							VPixel( vimages[z], t, y, x, VShort )
							= chunk.voxel<VShort>( x, y, z, 0 );
						}
					}
				}

				t++;
			}
		}

		// dims[3] > 1 ?
		// 3D image data
	} else {

		// save 3D data to ONE vista image
		vimages = ( VImage * )malloc( sizeof( VImage ) );
		nimages = 1;

		data::Chunk chunk = image.getChunk(0);
		// choose data type
		switch( chunk.typeID() ) {

		// VBit
		case util::TypePtr<VBit>::staticID:
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VBitRepn );
			for( int z = 0; z < dims[2]; z++ ) {
				for( int y = 0; y < dims[1]; y++ ) {
					for( int x = 0; x < dims[0]; x++ ) {
						VPixel( vimages[0], z, y, x, VBit )
							= chunk.voxel<VBit>( x, y, z, 0 );
					}
				}
			}
			break;

		// VUByte
		case util::TypePtr<VUByte>::staticID:
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VUByteRepn );
			for( int z = 0; z < dims[2]; z++ ) {
				for( int y = 0; y < dims[1]; y++ ) {
					for( int x = 0; x < dims[0]; x++ ) {
						VPixel( vimages[0], z, y, x, VUByte )
						        = chunk.voxel<VUByte>( x, y, z, 0 );
					}
				}
			}
			break;

		// VSByte
		case util::TypePtr<VSByte>::staticID:
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VSByteRepn );
			for( int z = 0; z < dims[2]; z++ ) {
				for( int y = 0; y < dims[1]; y++ ) {
					for( int x = 0; x < dims[0]; x++ ) {
						VPixel( vimages[0], z, y, x, VSByte ) =
								chunk.voxel<VSByte>( x, y, z, 0 );
					}
				}
			}
			break;

		// VShort
		case util::TypePtr<VShort>::staticID:
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VShortRepn );
			for( int z = 0; z < dims[2]; z++ ) {
				for( int y = 0; y < dims[1]; y++ ) {
					for( int x = 0; x < dims[0]; x++ ) {
						VPixel( vimages[0], z, y, x, VShort ) =
								chunk.voxel<VShort>( x, y, z, 0 );
					}
				}
			}
			break;

		// VLong
		case util::TypePtr<VLong>::staticID:
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VLongRepn );
			for( int z = 0; z < dims[2]; z++ ) {
				for( int y = 0; y < dims[1]; y++ ) {
					for( int x = 0; x < dims[0]; x++ ) {
						VPixel( vimages[0], z, y, x, VLong )
								= chunk.voxel<VLong>( x, y, z, 0 );
					}
				}
			}
			break;

		// VFloat
		case util::TypePtr<VFloat>::staticID:
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VFloatRepn );
			for( int z = 0; z < dims[2]; z++ ) {
				for( int y = 0; y < dims[1]; y++ ) {
					for( int x = 0; x < dims[0]; x++ ) {
						VPixel( vimages[0], z, y, x, VFloat )
								= chunk.voxel<VFloat>( x, y, z, 0 );
					}
				}
			}
			break;

		// VDouble
		case util::TypePtr<VDouble>::staticID:
			vimages[0] = VCreateImage( dims[2], dims[1], dims[0], VDoubleRepn);
			for( int z = 0; z < dims[2]; z++ ) {
				for( int y = 0; y < dims[1]; y++ ) {
					for( int x = 0; x < dims[0]; x++ ) {
						VPixel( vimages[0], z, y, x, VDouble)
								= chunk.voxel<VDouble>( x, y, z, 0 );
					}
				}
			}
			break;

		// default error
		default:
			LOG(image_io::Runtime,error)
				<< "Can't map image type to vista type. Abort ["
				<< chunk.typeID() << "/" << util::Type<VFloat>::staticID << "]" ;
			return;
		}

		// copy header information
		copyHeaderToVista(image, vimages[0]);

		VAppendAttr( attrList, "image", NULL, VImageRepn, vimages[0] );

	}

	//  write to output file
	FILE *f;
	LOG_IF( !( f = VOpenOutputFile( filename.c_str(), true ) ), ImageIoLog, error )
			<< "Error open file for writing." << util::MSubject( filename );
	LOG_IF( !VWriteFile( f, attrList ), ImageIoLog, error )
			<< "Error writing image data.";

	//  cleanup
	for( int z = 0; z < nimages; z++ )
		VDestroyImage( vimages[z] );

	free( vimages );
	fclose( f );
}


int ImageFormat_Vista::load( data::ChunkList &chunks, const std::string &filename,
		const std::string &dialect ) throw ( std::runtime_error & )
{
	// open input file
	FILE *ip;

	if( !( ip = fopen( filename.c_str(), "r" ) ) ) {
		std::string s;
		s = "Error opening file " + filename + "for reading.";
		throwGenericError( s );
	}

	VAttrList list;
	VImage *images;
	int nimages;

	// read images from file stream
	if( ( nimages = VReadImages( ip, &list, &images ) ) == 0 ) {
		std::string s = "Error reading images from file " + filename;
		throwGenericError( s );
	}

	image_io::enable_log<util::DefaultMsgPrint>( warning );
	LOG(image_io::Runtime,info) << "found " << nimages << " images.";

	// found one image -> write it right into one chunk
	if ( nimages == 1 ) {
		switch( VPixelRepn( images[0] ) ) {
		case VBitRepn:
			addChunk<VBit>(chunks,images[0]);
			break;
		case VUByteRepn:
			addChunk<VUByte>(chunks,images[0]);
			break;
		case VSByteRepn:
			addChunk<VSByte>(chunks,images[0]);
			break;
		case VShortRepn:
			addChunk<VShort>(chunks,images[0]);
			break;
		case VLongRepn:
			addChunk<VLong>(chunks,images[0]);
			break;
		case VFloatRepn:
			addChunk<VFloat>(chunks,images[0]);
			break;
		case VDoubleRepn:
			addChunk<VDouble>(chunks,images[0]);
		default:
			LOG( image_io::Runtime, error )
				<< "No Vista datatype found. Input image is broken. Abort.";
		}// switch(VPixelRepn(images[0]))

	}
	// found serveral images -> assume that this is functional data
	else {
		LOG( image_io::Runtime, error ) << "No support for 4D input, yet";
	}

	return nimages;
}

void ImageFormat_Vista::copyHeaderToVista( const data::Image &image, VImage& vimage )
{
	// get attribute list from image
	VAttrList list = VImageAttrList(vimage);

	// ********** MANDATORY attributes **********

	// get voxel
    util::fvector4 voxels = image.getProperty<util::fvector4>("voxelSize");
    std::stringstream vstr;
    vstr << voxels[0] << " " << voxels[1] << " " << voxels[2];
    VAppendAttr(list, "voxel",NULL,VStringRepn,vstr.str().c_str());

    // copy orientation vectors
    util::fvector4 readVec = image.getProperty<util::fvector4>("readVec");
    util::fvector4 phaseVec = image.getProperty<util::fvector4>("phaseVec");
    util::fvector4 sliceVec = image.getProperty<util::fvector4>("sliceVec");

    // set readVec -> columnVec
    vstr.str("");vstr << readVec[0] << " " << readVec[1] << " " << readVec[2];
    VAppendAttr(list, "columnVec", NULL, VStringRepn, vstr.str().c_str());

    // set phase -> rowVec
    vstr.str("");vstr << phaseVec[0] << " " << phaseVec[1] << " " << phaseVec[2];
    VAppendAttr(list, "rowVec", NULL, VStringRepn, vstr.str().c_str());

    // set sliceVec -> sliceVec
    vstr.str("");vstr << sliceVec[0] << " " << sliceVec[1] << " " << sliceVec[2];
    VAppendAttr(list, "sliceVec", NULL, VStringRepn, vstr.str().c_str());

    // index origin
    util::fvector4 indexOrigin = image.getProperty<util::fvector4>("indexOrigin");
    vstr.str("");vstr << indexOrigin[0] << " " << indexOrigin[1] << " " << indexOrigin[2];
    VAppendAttr(list, "indexOrigin", NULL, VStringRepn, vstr.str().c_str());

    // set slice orientation according to the image orientation
    switch(image.getMainOrientation()) {
    case data::Image::axial:
    case data::Image::reversed_axial:
    	VAppendAttr(list,"orientation", NULL, VStringRepn, "axial");
    	break;
    case data::Image::sagittal:
    case data::Image::reversed_sagittal:
    	VAppendAttr(list,"orientation", NULL, VStringRepn, "sagittal");
    	break;
    case data::Image::coronal:
    case data::Image::reversed_coronal:
    	VAppendAttr(list,"orientation", NULL, VStringRepn, "coronal");
    	break;
    }

    // ********** Vista group **********
    // POLICY: Append ALL properties from the 'Vista' Propmap to the end of the
    // attribute list.
    util::PropertyValue pval = image.getPropertyValue("Vista");
    if((not pval.empty()) and pval->is<util::PropMap>()) {
    	// convert it to a property map
    	util::PropMap &pmap = pval->cast_to_Type<util::PropMap>();
    	util::PropMap::key_list klist = pmap.keys();
    	util::PropMap::key_list::const_iterator kiter;
    	for( kiter = klist.begin(); kiter != klist.end(); kiter++) {

    		util::PropertyValue pv = pmap.getPropertyValue(*kiter);

    		// VBit -> VBit (char *)
    		if(pv->is<VBit>()) {
    			VAppendAttr(list, (*kiter).c_str(), NULL, VBitRepn,
    					(VBit)pv->cast_to_Type<VBit>());
    			continue;
    		}

    		// VUByte -> VUByte (char *)
			if(pv->is<VUByte>()) {
				VAppendAttr(list, (*kiter).c_str(), NULL, VUByteRepn,
						(VUByte)pv->cast_to_Type<VUByte>());
				continue;
			}

    		// VSByte -> VSByte (char *)
			if(pv->is<VSByte>()) {
				VAppendAttr(list, (*kiter).c_str(), NULL, VSByteRepn,
						(VSByte)pv->cast_to_Type<VSByte>());
				continue;
			}

    		// VShort -> VShort (char *)
			if(pv->is<VShort>()) {
				VAppendAttr(list, (*kiter).c_str(), NULL, VShortRepn,
						(VShort)pv->cast_to_Type<VShort>());
				continue;
			}

    		// VLong -> VLong (char *)
			if(pv->is<VLong>()) {
				VAppendAttr(list, (*kiter).c_str(), NULL, VLongRepn,
						(VLong)pv->cast_to_Type<VLong>());
				continue;
			}

    		// VFloat -> VFloat (char *)
			if(pv->is<VFloat>()) {
				VAppendAttr(list, (*kiter).c_str(), NULL, VFloatRepn,
						(VFloat)pv->cast_to_Type<VFloat>());
				continue;
			}

    		// VDouble -> VDouble (char *)
			if(pv->is<VDouble>()) {
				VAppendAttr(list, (*kiter).c_str(), NULL, VDoubleRepn,
						(VDouble)pv->cast_to_Type<VDouble>());
				continue;
			}

    		// VString -> std::string
    		if(pv->is<std::string>()) {
    			VAppendAttr(list, (*kiter).c_str(), NULL, VStringRepn,
    					((std::string)pv->cast_to_Type<std::string>()).c_str());
    			continue;
    		}
    	}
    }
}

void ImageFormat_Vista::copyHeaderFromVista( const VImage &image, data::Chunk &chunk )
{
	// traverse through attribute list and set metadata
	VAttrList attributes = VImageAttrList(image);
	VAttrListPosn posn;

	for(VFirstAttr(attributes,&posn);VAttrExists(&posn);VNextAttr(&posn)){
		const char* name = VGetAttrName(&posn);
		VPointer val;

		// MANDATORY: voxel --> voxelSize
		// it's a vector with 3 elements
		if(strcmp(name, "voxel") == 0){
			VGetAttrValue(&posn,NULL,VStringRepn,&val);
			std::list<float> flist = util::string2list<float>(std::string((char *)val));
			std::list<float>::const_iterator iter = flist.begin();
			float x = *iter++, y = *iter++, z = *iter;
			chunk.setProperty("voxelSize",util::fvector4(x, y, z, 0));
			continue;
		}

		// set all vista specific properties in a extra group.
		std::string propname = std::string("Vista/") + name;

		// MANDATORY: orientation --> readVector, phaseVector, sliceVector
		// create default read, phase, slice vector values according to attribute
		// "orientation" in vista header. This should only be done if the vectors
		// weren't defined otherwise.
		if((strcmp(name,"orientation") == 0) and (not chunk.hasProperty("readVec")) ){
			VGetAttrValue(&posn,NULL,VStringRepn,&val);

			//TODO remove "orientation" in Vista group
			chunk.setProperty<std::string>(propname,std::string((VString)val));

			// axial is the reference
			if(strcmp((const char*)val,"axial") == 0) {
				chunk.setProperty("readVec", util::fvector4(-1,0,0,0));
				chunk.setProperty("phaseVec", util::fvector4(0,-1,0,0));
				chunk.setProperty("sliceVec", util::fvector4(0,0,1,0));
				continue;
			}
			if(strcmp((const char*)val,"sagittal") == 0) {
				chunk.setProperty("readVec", util::fvector4(0,-1,0,0));
				chunk.setProperty("phaseVec", util::fvector4(0,0,-1,0));
				chunk.setProperty("sliceVec", util::fvector4(1,0,0,0));
				continue;
			}
			if(strcmp((const char*)val,"coronal") == 0) {
				chunk.setProperty("readVec", util::fvector4(-1,0,0,0));
				chunk.setProperty("phaseVec", util::fvector4(0,0,-1,0));
				chunk.setProperty("sliceVec", util::fvector4(0,-1,0,0));
				continue;
			}
		}

		// OPTIONAL: columnVec -> readVec, overwrite old values
		if(strcmp(name, "columnVec") == 0) {
			util::fvector4 readVec;
			VGetAttrValue(&posn,NULL,VStringRepn,&val);
			const std::list<float> tokens = util::string2list<float>((const char*)val,' ');
			readVec.copyFrom<std::list<float>::const_iterator>(tokens.begin(),tokens.end());
			chunk.setProperty<util::fvector4>("readVec",readVec);
			continue;
		}

		// OPTIONAL: rowVec -> phaseVec, overwrite old values
		if(strcmp(name, "rowVec") == 0) {
			util::fvector4 phaseVec;
			VGetAttrValue(&posn,NULL,VStringRepn,&val);
			const std::list<float> tokens = util::string2list<float>((const char*)val,' ');
			phaseVec.copyFrom<std::list<float>::const_iterator>(tokens.begin(),tokens.end());
			chunk.setProperty<util::fvector4>("phaseVec",phaseVec);
			continue;
		}

		// OPTIONAL: sliceVec -> sliceVec, overwrite old values
		if(strcmp(name, "sliceVec") == 0) {
			util::fvector4 sliceVec;
			VGetAttrValue(&posn,NULL,VStringRepn,&val);
			const std::list<float> tokens = util::string2list<float>((const char*)val,' ');
			sliceVec.copyFrom<std::list<float>::const_iterator>(tokens.begin(),tokens.end());
			chunk.setProperty<util::fvector4>("sliceVec",sliceVec);
			continue;
		}

		// OPTIONAL: indexOrigina -> indexOrigin
		if(strcmp(name, "indexOrigin") == 0){
			VGetAttrValue(&posn,NULL,VStringRepn,&val);
			std::list<float> flist = util::string2list<float>(std::string((char *)val));
			std::list<float>::const_iterator iter = flist.begin();
			float x = *iter++, y = *iter++, z = *iter;
			chunk.setProperty("indexOrigin",util::fvector4(x, y, z, 0));
			continue;
		}

		// traverse through attributes
		switch(VGetAttrRepn(&posn)) {

		case VBitRepn:
			VGetAttrValue(&posn,NULL,VBitRepn,val);
			break;
		case VUByteRepn:
			VGetAttrValue(&posn,NULL,VUByteRepn,val);
			break;
		case VSByteRepn:
			VGetAttrValue(&posn,NULL,VSByteRepn,val);
			break;
		case VShortRepn:
			VGetAttrValue(&posn,NULL,VShortRepn,val);
			break;
		case VLongRepn:
			VGetAttrValue(&posn,NULL,VLongRepn,val);
			break;
		case VFloatRepn:
			VGetAttrValue(&posn,NULL,VFloatRepn,val);
			break;
		case VDoubleRepn:
			VGetAttrValue(&posn,NULL,VDoubleRepn,val);
			break;
		case VStringRepn:
			VGetAttrValue(&posn,NULL,VStringRepn,&val);
			chunk.setProperty<std::string>(propname,std::string((VString)val));
			break;
		default:
			std::cout << "unknown attribute representation found" << std::endl;
		}
	}

	// AFTERMATH
	// set missing values according to default rules

	//if not set yet, set read, phase and slice vector.
	// DEFAULT: axial
	if(not chunk.hasProperty("readVec")) {
		chunk.setProperty("readVec", util::fvector4(1,0,0,0));
		chunk.setProperty("phaseVec", util::fvector4(0,1,0,0));
		chunk.setProperty("sliceVec", util::fvector4(0,0,1,0));
	}

	// set default index origin according to the image geometry
	if(not chunk.hasProperty("indexOrigin")) {
		util::fvector4 dims = chunk.sizeToVector();
		util::fvector4 voxels = chunk.getProperty<util::fvector4>("voxelSize");
		// calculate index origin according to axial
		util::fvector4 ioTmp(
				((dims[0]-1)*voxels[0])/2,
				((dims[1]-1)*voxels[1])/2,
				-((dims[2]-1)*voxels[2])/2,
				0);

		// multiply indexOrigin with read, phase and slice vector
		util::fvector4 iOrig(
				((util::fvector4)chunk.getProperty<util::fvector4>("readVec")).dot(ioTmp),
				((util::fvector4)chunk.getProperty<util::fvector4>("phaseVec")).dot(ioTmp),
				((util::fvector4)chunk.getProperty<util::fvector4>("sliceVec")).dot(ioTmp),
				0);
		chunk.setProperty("indexOrigin",iOrig);
	}

	// set acquisitionNumber. This values is always missing
	chunk.setProperty("acquisitionNumber",0);
}

template <typename TInput> void ImageFormat_Vista::addChunk(data::ChunkList &chunks, VImage image) {

	boost::shared_ptr<data::MemChunk<TInput> > chunk_sp(
			new data::MemChunk<TInput>( static_cast<TInput*>( image->data),
					VImageNColumns( image ),
					VImageNRows( image ),
					VImageNBands( image ) ) );

	// copy header information
	copyHeaderFromVista( image, *chunk_sp );
	// add chunk to chunk list
	chunks.push_back( *chunk_sp );
}

}//namespace image_io
}//namespace isis

isis::image_io::FileFormat *
factory()
{
	return new isis::image_io::ImageFormat_Vista();
}
