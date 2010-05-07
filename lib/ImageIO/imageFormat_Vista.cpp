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
 * Author: Thomas Pröger, proeger@cbs.mpg.de, 2010
 *
 *****************************************************************/

// local includes
#include "imageFormat_Vista.h"

// global includes
#include <viaio/VImage.h>

namespace isis
{

namespace image_io
{


void ImageFormat_Vista::write( const data::Image &image, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & )
{
	//  All vista images a organized in an attribue list. Let's create an empty one:
	VAttrList attrList = VCreateAttrList();
	//  One or more VImages need to be written to disk.
	VImage *vimages;
	//  get size for each dimension
	util::ivector4 dims = image.sizeToVector();
	//  create a vista image container according to the isis image configuration

	// 4D image data
	//  when we have got a 4D-image, this image provides functional data information
	if( dims[3] > 1 ) {
		vimages = ( VImage * )malloc( sizeof( VImage ) * dims[2] );
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
				vimages[z] = VCreateImage( dims[3], dims[1], dims[0], VShortRepn );

				// get all data from the first chunk.
				for( int x = 0; x < dims[0]; x++ ) {
					for( int y = 0; y < dims[1]; y++ ) {
						for( int t = 0; t < dims[3]; t++ ) {
							VPixel( vimages[z], t, y, x, VShort ) =
								mchunk.voxel<VShort>( x, y, z, t );
						}
					}
				}

				VAppendAttr( attrList, "image", NULL, VImageRepn, vimages[z] );
			}
		}
		// the 4th dimension ist distributed over the number of chunks
		else {
			// allocate images
			for( int z = 0; z < dims[2]; z++ ) {
				vimages[z] = VCreateImage( dims[3], dims[1], dims[0], VShortRepn );
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
							VPixel( vimages[z], t, y, x, VShort ) =
								chunk.voxel<VShort>( x, y, z, 0 );
						}
					}
				}

				t++;
			}
		}

		// 3D image data
		// dims[3] > 1 ?
	} else {
		LOG( image_io::Runtime, error ) << "No support for 3D images, yet";
	}

	//  write to output file
	FILE *f;
	LOG_IF( not ( f = VOpenOutputFile( filename.c_str(), true ) ), ImageIoLog, error )
			<< "Error open file for writing."  << util::MSubject( filename );
	LOG_IF( not VWriteFile( f, attrList ), ImageIoLog, error )
			<< "Error writing image data.";

	//  cleanup
	for( int z = 0; z < dims[2]; z++ )
		VDestroyImage( vimages[z] );

	free( vimages );
	fclose( f );
}

int ImageFormat_Vista::load( data::ChunkList &chunks, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & )
{
}

}
}//namespace image_io isis

isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Vista();
}
