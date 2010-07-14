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

#ifndef IMAGEFORMAT_VISTA_H_
#define IMAGEFORMAT_VISTA_H_

// global includes
#include <viaio/VImage.h>

// local includes
#include <DataStorage/io_interface.h>

namespace isis
{

namespace image_io
{

class ImageFormat_Vista: public FileFormat
{
public:

	std::string name() { return std::string( "de.mpg.cbs.isis.vista" );}
	std::string suffixes() {return std::string( ".v" );}
	/**
	 * This plugin supports the following dialects:
	 *
	 * <ul>
	 * <li><b>anatomical</b> interpret every subimage as anatomical image. Each subimage will
	 * be saved to chunk while preserving the data type.
	 * <li><b>map</b> interpret the first VFloat subimage found as an overlay map. Overlay maps can be
	 * used to visualize correlated data together with it's anatomical counterpart. That can be statistical
	 * maps like zmaps, tmaps or fmaps as well as any other kind of VFloat data.
	 * <li><b>functional</b> interpret the whole set of subimages as an 4D raw data image. Each subimage must
	 * contain VShort voxel data. Each subimage will be interpretet as slice ((x,y) = (columns,rows)) and the
	 * third dimension (band) is interpretet as time.
	 * </ul>
	 *
	 */
	std::string dialects() {return std::string( "functional map anatomical" );}
	int load( data::ChunkList &chunks, const std::string &filename,
			  const std::string &dialect ) throw( std::runtime_error & );
	void write( const data::Image &image, const std::string &filename,
				const std::string &dialect ) throw( std::runtime_error & );

private:

	template <typename TYPE> class VistaChunk : public data::Chunk
	{

	private:

		/**
		 * The vista IO plugin maps VImage objects to isis::Chunks. The VImage
		 * memory space must be freed afterwards.
		 */
		struct VImageDeleter {

			VImage m_image;

			VImageDeleter( VImage image ) {
				m_image = image;
			}

			void operator()( void *p ) {
				LOG( Debug, info ) << "Freeing VImage pointer";
				VDestroyImage( m_image );
			}
		};

		/**
		 * This function copies all metadata from Vista image header attributes to
		 * the corresponding fields in the target Vista image.
		 * @param image The target chunk where all data will be copied to.
		 * @oaram chunk The source image that provides the Vista metadata attributes.
		 */
		void copyHeaderFromVista( const VImage &image, data::Chunk &chunk ) {
			// traverse through attribute list and set metadata
			VAttrList attributes = VImageAttrList( image );
			VAttrListPosn posn;

			for( VFirstAttr( attributes, &posn ); VAttrExists( &posn ); VNextAttr( &posn ) ) {
				const char *name = VGetAttrName( &posn );
				VPointer val;

				// MANDATORY: voxel --> voxelSize
				// it's a vector with 3 elements
				if( strcmp( name, "voxel" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					std::list<float> flist = util::string2list<float>( std::string( ( char * )val ) );
					std::list<float>::const_iterator iter = flist.begin();
					float x = *iter++, y = *iter++, z = *iter;
					chunk.setProperty( "voxelSize", util::fvector4( x, y, z, 0 ) );
					continue;
				}

				// set all vista specific properties in a extra group.
				std::string propname = std::string( "Vista/" ) + name;

				// MANDATORY: orientation --> readVector, phaseVector, sliceVector
				// create default read, phase, slice vector values according to attribute
				// "orientation" in vista header. This should only be done if the vectors
				// weren't defined otherwise.
				if( ( strcmp( name, "orientation" ) == 0 ) and ( not chunk.hasProperty( "readVec" ) ) ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					//TODO remove "orientation" in Vista group
					chunk.setProperty<std::string>( propname, std::string( ( VString )val ) );

					// axial is the reference
					if( strcmp( ( const char * )val, "axial" ) == 0 ) {
						chunk.setProperty( "readVec", util::fvector4( 1, 0, 0, 0 ) );
						chunk.setProperty( "phaseVec", util::fvector4( 0, 1, 0, 0 ) );
						chunk.setProperty( "sliceVec", util::fvector4( 0, 0, 1, 0 ) );
						continue;
					}

					if( strcmp( ( const char * )val, "sagittal" ) == 0 ) {
						chunk.setProperty( "readVec", util::fvector4( 0, 1, 0, 0 ) );
						chunk.setProperty( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
						chunk.setProperty( "sliceVec", util::fvector4( 1, 0, 0, 0 ) );
						continue;
					}

					if( strcmp( ( const char * )val, "coronal" ) == 0 ) {
						chunk.setProperty( "readVec", util::fvector4( 1, 0, 0, 0 ) );
						chunk.setProperty( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
						chunk.setProperty( "sliceVec", util::fvector4( 0, -1, 0, 0 ) );
						continue;
					}
				}

				// OPTIONAL: "repetition_time" or "repetition" -> repetitionTime
				if( ( strcmp ( name, "repetition" ) == 0 ) || ( strcmp( name, "repetition_time" ) == 0 ) ) {
					VGetAttrValue( &posn, NULL, VShortRepn, val );
					chunk.setProperty<unsigned short>( "repetitionTime", *( ( unsigned short * )val ) );
					continue;
				}

				// OPTIONAL: columnVec -> readVec, overwrite old values
				if( strcmp( name, "columnVec" ) == 0 ) {
					util::fvector4 readVec;
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					const std::list<float> tokens = util::string2list<float>( ( const char * )val, ' ' );
					readVec.copyFrom<std::list<float>::const_iterator>( tokens.begin(), tokens.end() );
					chunk.setProperty<util::fvector4>( "readVec", readVec );
					continue;
				}

				// OPTIONAL: rowVec -> phaseVec, overwrite old values
				if( strcmp( name, "rowVec" ) == 0 ) {
					util::fvector4 phaseVec;
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					const std::list<float> tokens = util::string2list<float>( ( const char * )val, ' ' );
					phaseVec.copyFrom<std::list<float>::const_iterator>( tokens.begin(), tokens.end() );
					chunk.setProperty<util::fvector4>( "phaseVec", phaseVec );
					continue;
				}

				// OPTIONAL: sliceVec -> sliceVec, overwrite old values
				if( strcmp( name, "sliceVec" ) == 0 ) {
					util::fvector4 sliceVec;
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					const std::list<float> tokens = util::string2list<float>( ( const char * )val, ' ' );
					sliceVec.copyFrom<std::list<float>::const_iterator>( tokens.begin(), tokens.end() );
					chunk.setProperty<util::fvector4>( "sliceVec", sliceVec );
					continue;
				}

				// OPTIONAL: indexOrigin -> indexOrigin
				if( strcmp( name, "indexOrigin" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					std::list<float> flist = util::string2list<float>( std::string( ( char * )val ) );
					std::list<float>::const_iterator iter = flist.begin();
					float x = *iter++, y = *iter++, z = *iter;
					chunk.setProperty( "indexOrigin", util::fvector4( x, y, z, 0 ) );
					continue;
				}

				// traverse through attributes
				switch( VGetAttrRepn( &posn ) ) {
				case VBitRepn:
					VGetAttrValue( &posn, NULL, VBitRepn, val );
					break;
				case VUByteRepn:
					VGetAttrValue( &posn, NULL, VUByteRepn, val );
					break;
				case VSByteRepn:
					VGetAttrValue( &posn, NULL, VSByteRepn, val );
					break;
				case VShortRepn:
					VGetAttrValue( &posn, NULL, VShortRepn, val );
					break;
				case VLongRepn:
					VGetAttrValue( &posn, NULL, VLongRepn, val );
					break;
				case VFloatRepn:
					VGetAttrValue( &posn, NULL, VFloatRepn, val );
					break;
				case VDoubleRepn:
					VGetAttrValue( &posn, NULL, VDoubleRepn, val );
					break;
				case VStringRepn:
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					chunk.setProperty<std::string>( propname, std::string( ( VString )val ) );
					break;
				default:
					std::cout << "unknown attribute representation found" << std::endl;
				}
			}

			// AFTERMATH
			// set missing values according to default rules

			//if not set yet, set read, phase and slice vector.
			// DEFAULT: axial
			if( not chunk.hasProperty( "readVec" ) ) {
				chunk.setProperty( "readVec", util::fvector4( 1, 0, 0, 0 ) );
				chunk.setProperty( "phaseVec", util::fvector4( 0, 1, 0, 0 ) );
				chunk.setProperty( "sliceVec", util::fvector4( 0, 0, 1, 0 ) );
			}

			// set default index origin according to the image geometry
			if( not chunk.hasProperty( "indexOrigin" ) ) {
				util::fvector4 dims = chunk.sizeToVector();
				util::fvector4 voxels = chunk.getProperty<util::fvector4>( "voxelSize" );
				// calculate index origin according to axial
				util::fvector4 ioTmp(
					-( ( dims[0] - 1 )*voxels[0] ) / 2,
					-( ( dims[1] - 1 )*voxels[1] ) / 2,
					-( ( dims[2] - 1 )*voxels[2] ) / 2,
					0 );
				util::fvector4 readV = chunk.getProperty<util::fvector4>( "readVec" );
				util::fvector4 phaseV = chunk.getProperty<util::fvector4>( "phaseVec" );
				util::fvector4 sliceV = chunk.getProperty<util::fvector4>( "sliceVec" );
				// multiply indexOrigin with read, phase and slice vector
				util::fvector4 iOrig(
					readV[0] * ioTmp[0] + phaseV[0] * ioTmp[1] + sliceV[0] * ioTmp[2],
					readV[1] * ioTmp[0] + phaseV[1] * ioTmp[1] + sliceV[1] * ioTmp[2],
					readV[2] * ioTmp[0] + phaseV[2] * ioTmp[1] + sliceV[2] * ioTmp[2],
					0 );
				chunk.setProperty( "indexOrigin", iOrig );
			}

			// set acquisitionNumber. This values is always missing
			chunk.setProperty( "acquisitionNumber", 0 );
		}

	public:

		/**
		 * Default constructor. Create a VistaChunk out of a vista image.
		 */
		VistaChunk( VImage image ):
			data::Chunk( static_cast<TYPE *>( image->data ), VImageDeleter( image ),
						 VImageNColumns( image ), VImageNRows( image ), VImageNBands( image ) ) {
			copyHeaderFromVista( image, *this );
		}
	};

	/**
	 * This function copies all chunk header informations to the appropriate
	 * vista image attribute values.
	 *
	 * @param chunk A reference to chunk that provides the metadata.
	 * @param image The target image. Alle metadata will be copied to the
	 * corresponding header attributes.
	 */
	void copyHeaderToVista( const data::Image &image, VImage &vimage );

	/**
	 * Copies the whole itk image into a given vista image. This function is
	 * templated over the pixel data type of the target vista image. Therefore
	 * correct data types should be vista data types like VShort, VUByte, etc. .
	 */
	template <typename T> bool copyImageToVista( const data::Image &image, VImage &vimage );

	/**
	 * This function creates a MemChunk with the correct type and adds it to the
	 * end of the Chunk list.
	 */
	template <typename TInput> void addChunk( data::ChunkList &chunks, VImage image );
};

}
}//namespace image_io isis

#endif /* IMAGEFORMAT_VISTA_H_ */
