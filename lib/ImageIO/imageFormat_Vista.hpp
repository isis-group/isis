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

#ifndef IMAGEFORMAT_VISTA_H_
#define IMAGEFORMAT_VISTA_H_

// global includes
#include <viaio/VImage.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <list>
#include <boost/foreach.hpp>

// local includes
#include <DataStorage/io_interface.h>
#include <CoreUtils/istring.hpp>

namespace isis
{

namespace image_io
{

class ImageFormat_Vista: public FileFormat
{
protected:
	std::string suffixes()const {return std::string( ".v" );}
public:
	std::string name()const { return std::string( "Vista" );}
	bool tainted()const {return false;}//internal plugins are not tainted

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
	std::string dialects( const std::string &filename )const {return std::string( "functional map anatomical" );}
	int load( data::ChunkList &chunks, const std::string &filename,
			  const std::string &dialect ) throw( std::runtime_error & );
	void write( const data::Image &image, const std::string &filename,
				const std::string &dialect ) throw( std::runtime_error & );

	/**
	 * Default constructor. Needed to initialize some private member variables.
	 */
	ImageFormat_Vista() : FileFormat(),
		histPrefix( "Vista/HistoryLine" ) {}

private:

	/**
	 * The prefix string for the isis metadata properties holding the vista
	 * history informations.
	 */
	const std::string histPrefix;

	struct DateDecoding {
		std::string dateRegex, delimiter;
		size_t first, second, third;
		DateDecoding( std::string regex, std::string d, size_t f, size_t s, size_t t )
			: dateRegex( regex ), delimiter( d ) , first( f ), second( s ), third( t ) {}
	};

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
		void copyHeaderFromVista( const VImage &image, data::Chunk &chunk, bool functional ) {
			// traverse through attribute list and set metadata
			VAttrList attributes = VImageAttrList( image );
			VAttrListPosn posn;
			LOG( DataLog, verbose_info ) << "copying Header from Vista";
			std::string time, date;

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
					chunk.setProperty<util::fvector4>( "voxelSize", util::fvector4( x, y, z, 1 ) );
					continue;
				}

				// set all vista specific properties in a extra group.
				util::istring propname = ( std::string( "Vista/" ) + name ).c_str();

				// MANDATORY: orientation --> readVector, phaseVector, sliceVector
				// create default read, phase, slice vector values according to attribute
				// "orientation" in vista header. This should only be done if the vectors
				// weren't defined otherwise.
				if( ( strcmp( name, "orientation" ) == 0 ) && ( ! chunk.hasProperty( "readVec" ) ) ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					//TODO remove "orientation" in Vista group
					chunk.setProperty<std::string>( propname, std::string( ( VString )val ) );

					if( functional ) {
						// axial is the reference
						if( strcmp( ( const char * )val, "axial" ) == 0 ) {
							chunk.setProperty<util::fvector4>( "readVec", util::fvector4( 1, 0, 0, 0 ) );
							chunk.setProperty<util::fvector4>( "phaseVec", util::fvector4( 0, 1, 0, 0 ) );
							chunk.setProperty<util::fvector4>( "sliceVec", util::fvector4( 0, 0, 1, 0 ) );
							continue;
						}

						if( strcmp( ( const char * )val, "sagittal" ) == 0 ) {
							chunk.setProperty<util::fvector4>( "readVec", util::fvector4( 0, 1, 0, 0 ) );
							chunk.setProperty<util::fvector4>( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
							chunk.setProperty<util::fvector4>( "sliceVec", util::fvector4( 1, 0, 0, 0 ) );
							continue;
						}

						if( strcmp( ( const char * )val, "coronal" ) == 0 ) {
							chunk.setProperty<util::fvector4>( "readVec", util::fvector4( 1, 0, 0, 0 ) );
							chunk.setProperty<util::fvector4>( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
							chunk.setProperty<util::fvector4>( "sliceVec", util::fvector4( 0, -1, 0, 0 ) );
							continue;
						}
					} else {
						if( strcmp( ( const char * )val, "axial" ) == 0 ) {
							chunk.setProperty<util::fvector4>( "readVec", util::fvector4( -1, 0, 0, 0 ) );
							chunk.setProperty<util::fvector4>( "phaseVec", util::fvector4( 0, 1, 0, 0 ) );
							chunk.setProperty<util::fvector4>( "sliceVec", util::fvector4( 0, 0, -1, 0 ) );
							continue;
						}

						if( strcmp( ( const char * )val, "sagittal" ) == 0 ) {
							chunk.setProperty<util::fvector4>( "readVec", util::fvector4( 0, 1, 0, 0 ) );
							chunk.setProperty<util::fvector4>( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
							chunk.setProperty<util::fvector4>( "sliceVec", util::fvector4( 1, 0, 0, 0 ) );
							continue;
						}

						if( strcmp( ( const char * )val, "coronal" ) == 0 ) {
							chunk.setProperty<util::fvector4>( "readVec", util::fvector4( 1, 0, 0, 0 ) );
							chunk.setProperty<util::fvector4>( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
							chunk.setProperty<util::fvector4>( "sliceVec", util::fvector4( 0, -1, 0, 0 ) );
							continue;
						}
					}
				}

				// OPTIONAL: "repetition_time" or "repetition" -> repetitionTime
				if( ( strcmp ( name, "repetition" ) == 0 ) || ( strcmp( name, "repetition_time" ) == 0 ) ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					std::string repTime = std::string( VString( val ) );

					if ( static_cast<signed int>( repTime.find( "." ) ) != -1 ) {
						repTime.erase( repTime.begin() + repTime.find( "." ), repTime.end() );
					}

					std::stringstream sstr( repTime );
					u_int16_t repTimeInt;
					sstr >> repTimeInt;
					chunk.setProperty<u_int16_t>( "repetitionTime", repTimeInt );
					continue;
				}

				if ( strcmp ( name , "slice_time" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					std::string slice_time = std::string( VString ( val ) );
					std::stringstream sstr( slice_time );
					float sliceTimeFloat;
					sstr >> sliceTimeFloat;
					chunk.setProperty<float>( "acquisitionTime",  sliceTimeFloat  );
					continue;
				}

				if ( strcmp ( name, "sex" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					util::Selection genderSelection( "female,male,other" );

					if ( std::string( ( VString ) val ) == "female" ) {
						genderSelection.set( "female" );
					}

					if ( std::string( ( VString ) val ) == std::string( "male" ) ) {
						genderSelection.set( "male" );
					}

					if ( std::string( ( VString ) val ) == "other" ) {
						genderSelection.set( "other" );
					}

					chunk.setProperty<util::Selection>( "subjectGender", genderSelection );
					continue;
				}

				if ( strcmp ( name, "patient" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					std::string subjectName = std::string ( ( VString ) val );
					subjectName.resize( 4 );
					chunk.setProperty<std::string>( "subjectName", subjectName );
					continue;
				}

				// OPTIONAL: columnVec -> readVec, overwrite old values
				if( strcmp( name, "columnVec" ) == 0 ) {
					util::fvector4 readVec;
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					const std::list<float> tokens = util::string2list<float>( std::string( ( const char * )val ), ' ' );
					readVec.copyFrom<std::list<float>::const_iterator>( tokens.begin(), tokens.end() );
					chunk.setProperty<util::fvector4>( "readVec", readVec );
					continue;
				}

				// OPTIONAL: rowVec -> phaseVec, overwrite old values
				if( strcmp( name, "rowVec" ) == 0 ) {
					util::fvector4 phaseVec;
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					const std::list<float> tokens = util::string2list<float>( std::string( ( const char * )val ), ' ' );
					phaseVec.copyFrom<std::list<float>::const_iterator>( tokens.begin(), tokens.end() );
					chunk.setProperty<util::fvector4>( "phaseVec", phaseVec );
					continue;
				}

				// OPTIONAL: sliceVec -> sliceVec, overwrite old values
				if( strcmp( name, "sliceVec" ) == 0 ) {
					util::fvector4 sliceVec;
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					const std::list<float> tokens = util::string2list<float>( std::string( ( const char * )val ), ' ' );
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
					chunk.setProperty<util::fvector4>( "indexOrigin", util::fvector4( x, y, z, 0 ) );
					continue;
				}

				if( strcmp( name, "date" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					date = std::string( ( VString ) val );
					continue;
				}

				if( strcmp( name, "time" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					time = std::string( ( VString ) val );
					continue;
				}

				if( strcmp( name, "echoTime" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					std::string echoTimeStr = std::string( VString ( val ) );
					std::stringstream sstr( echoTimeStr );
					float echoTime;
					sstr >> echoTime;
					chunk.setProperty<float>( "echoTime", echoTime );
					continue;
				}

				if( strcmp( name, "flipAngle" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					std::string flipAngleStr = std::string( VString ( val ) );
					std::stringstream sstr( flipAngleStr );
					u_int16_t flipAngle;
					sstr >> flipAngle;
					chunk.setProperty<float>( "flipAngle", flipAngle );
					continue;
				}

				if( strcmp( name, "transmitCoil" ) == 0 ) {
					VGetAttrValue( &posn, NULL, VStringRepn, &val );
					chunk.setProperty<std::string>( "transmitCoil", std::string( ( VString )val ) );
					continue;
				}

				// read the age in years (vista) and save it in PatientAge in days (isis)
				if( strcmp( name, "age" ) == 0 ) {
					uint16_t age;
					VGetAttrValue( &posn, NULL, VShortRepn, &age );
					// rounding: floor or ceil
					age = ( ( age * 365.2425 ) - floor( age * 365.2425 ) ) < 0.5 ?
						  floor( age * 365.2425 ) : ceil( age * 365.2425 );
					chunk.setProperty<uint16_t>( "subjectAge", age );
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
			} // END iterate over attributes

			// AFTERMATH
			// set missing values according to default rules

			if ( date.size() ) {
				std::list<DateDecoding> m_dateDecodingList;
				std::list<DateDecoding> m_timeDecodingList;
				//e.g. 12 May 2007
				m_dateDecodingList.push_back( DateDecoding( std::string( "^([[:digit:]]{1,2})\\ {1}([[:word:]]{3})\\ {1}([[:digit:]]{4}).*" ), "-", 1, 2, 3 ) );
				//e.g. 12.03.2007
				m_dateDecodingList.push_back( DateDecoding( std::string( "^([[:digit:]]{2})\\.([[:digit:]]{2})\\.([[:digit:]]{4})$" ), "-", 1, 2, 3 ) );
				//e.g. 2010-Feb-12
				m_dateDecodingList.push_back( DateDecoding( std::string( "^([[:digit:]]{4})\\-([[:word:]]{3})\\-([[:digit:]]{2})$" ), "-", 3, 2, 1 ) );
				//e.g. 20081210
				m_dateDecodingList.push_back( DateDecoding( std::string( "^([[:digit:]]{4})([[:digit:]]{2})([[:digit:]]{2})$" ), "-", 3, 2, 1 ) );
				//e.g. 11:15:49 5 May 2002 (date)
				m_dateDecodingList.push_back( DateDecoding( std::string( "^([[:digit:]]{2})\\:([[:digit:]]{2})\\:([[:digit:]]{2})\\ +([[:digit:]]{1,2})\\ +([[:word:]]{3})\\ +([[:digit:]]{4}).*" ), "-", 4, 5, 6 ) );
				//e.g. 11:15:49
				m_timeDecodingList.push_back( DateDecoding( std::string( "^([[:digit:]]{2})\\:([[:digit:]]{2})\\:([[:digit:]]{2}).*" ), "not_needed", 1, 2, 3 ) );
				//e.g. 11:15:49 5 May 2002 (time)
				m_timeDecodingList.push_back( DateDecoding( std::string( "^([[:digit:]]{2})\\:([[:digit:]]{2})\\:([[:digit:]]{2})\\ ?([[:digit:]]{1,2})\\ ?([[:word:]{3})\\ ?([[:digit:]]{4}).*" ), "not_needed", 1, 2, 3 ) );
				std::string day, month, year;
				boost::gregorian::date isisDate;
				BOOST_FOREACH( std::list<DateDecoding>::const_reference dateRef, m_dateDecodingList ) {
					boost::regex dateRegex( dateRef.dateRegex );
					boost::cmatch dateResults;

					if ( boost::regex_match( date.c_str(), dateResults, dateRegex ) ) {
						day = boost::lexical_cast<std::string>( dateResults.str( dateRef.first ) );

						if( day.size() == 1 ) {
							day.insert( 0, std::string( "0" ) );
						}

						month = boost::lexical_cast<std::string>( dateResults.str( dateRef.second ) );
						year = boost::lexical_cast<std::string>( dateResults.str( dateRef.third ) );
						std::string strDate = year + dateRef.delimiter + month + dateRef.delimiter + day;
						isisDate = boost::gregorian::from_simple_string( strDate );
					}
				}

				//if no attribute time was found search in date for the time
				if ( ! time.size() ) {
					time = date;
				}

				size_t hours, minutes, seconds;
				boost::posix_time::time_duration isisTimeDuration;
				BOOST_FOREACH( std::list<DateDecoding>::const_reference timeRef, m_timeDecodingList ) {
					boost::regex timeRegex( timeRef.dateRegex );
					boost::cmatch timeResults;

					if ( boost::regex_match( time.c_str(), timeResults, timeRegex ) ) {
						hours = boost::lexical_cast<size_t>( timeResults[timeRef.first] );
						minutes = boost::lexical_cast<size_t>( timeResults[timeRef.second] );
						seconds = boost::lexical_cast<size_t>( timeResults[timeRef.third] );
						isisTimeDuration = boost::posix_time::time_duration( hours, minutes, seconds );
					}
				}
				boost::posix_time::ptime isisTime( isisDate, isisTimeDuration );
				chunk.setProperty<boost::posix_time::ptime>( "sequenceStart", isisTime );
			}

			//if not set yet, set read, phase and slice vector.
			// DEFAULT: axial
			if( not chunk.hasProperty( "readVec" ) ) {
				chunk.setProperty<util::fvector4>( "readVec", util::fvector4( 1, 0, 0, 0 ) );
				chunk.setProperty<util::fvector4>( "phaseVec", util::fvector4( 0, 1, 0, 0 ) );
				chunk.setProperty<util::fvector4>( "sliceVec", util::fvector4( 0, 0, 1, 0 ) );
			}

			// set voxel gap tp (0,0,0,0) since there is no gap information available
			// in vista images.
			chunk.setProperty<util::fvector4>( "voxelGap", util::fvector4( 0, 0, 0, 0 ) );

			// set acquisitionNumber. This values is always missing
			if( !chunk.hasProperty( "acquisitionNumber" ) )
				chunk.setProperty<uint32_t>( "acquisitionNumber", 0 );

			LOG( DataLog, verbose_info ) << "finished copying header";
		}

	public:

		/**
		 * Default constructor. Create a VistaChunk out of a vista image.
		 */

		VistaChunk( VImage image, const bool functional, size_t nslices = 0 ):
			data::Chunk( static_cast<TYPE *>( image->data ), VImageDeleter( image ),
						 VImageNColumns( image ), VImageNRows( image ), functional ? 1 : VImageNBands( image ), functional ? VImageNBands( image ) : 1 ) {
			copyHeaderFromVista( image, *this, functional );
		}
	};

	//member function which switch handles the loaded images
	bool switchHandle( VImage &, data::ChunkList & );

	/**
	 * This function copies all chunk header informations to the appropriate
	 * vista image attribute values.
	 *
	 * @param image A reference to image that provides the metadata.
	 * @param vimage The target Vista image. Alle metadata will be copied to the
	 * corresponding header attributes.
	 * @param functional flag to indicate that the image contains functional data.
	 * @param slice the index of the slice.
	 */
	void copyHeaderToVista( const data::Image &image, VImage &vimage,  const float &sliceTimeOffset , const bool functional, size_t slice = 0 );

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

	/**
	 * This function calculates the index origin of a given chunk according to its
	 * slice orientation information and voxel resolution. The resulting indexOrigin
	 * will point to the physical coordinate of the voxel at position (0,0,0).
	 * Since there are no additional informations about the correct position of
	 * the voxel space relative to the scanner space available, the routine assumes
	 * that the voxel space is placed with the center near the scanner iso center.
	 */
	util::fvector4 calculateIndexOrigin( data::Chunk &chunk, util::ivector4 &dims );
};

}
}//namespace image_io isis

#endif /* IMAGEFORMAT_VISTA_H_ */
