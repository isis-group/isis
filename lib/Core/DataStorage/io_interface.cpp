#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <iomanip>
#include <iostream>

#include "CoreUtils/log.hpp"
#include "DataStorage/common.hpp"
#include "DataStorage/io_interface.h"

namespace isis
{
namespace image_io
{

void FileFormat::write( const isis::data::ImageList &images, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & )
{
	boost::filesystem::path path( filename );
	std::string file = path.leaf();
	path.remove_leaf();
	bool ret = true;
	bool uniqueSequenceNumber;
	if ( images.size() > 1 ) {
		//check if there is more than one image with the same sequenceNumber
		std::set<u_int16_t> sequenceSet;
		//map to get an index for every sequenceNumber
		std::map<u_int16_t, size_t> imagePerSequenceCounter;
		BOOST_FOREACH( data::ImageList::const_reference ref, images ) {
			if ( ref->hasProperty( "sequenceNumber" ) ) {
				sequenceSet.insert( ref->getProperty<u_int16_t>( "sequenceNumber" ) );
				imagePerSequenceCounter.insert( std::pair<u_int16_t, unsigned short>( ref->getProperty<u_int16_t>( "sequenceNumber" ), 0 ) );
			}
		}
		const unsigned short imageDigitsSequence = std::log10( sequenceSet.size() ) + 1;
		const unsigned short imageDigitsImagePerSequenceCounter = std::log10( images.size() ) + 1;

		if( sequenceSet.size() == images.size() ) {
			uniqueSequenceNumber = true;
		} else {
			uniqueSequenceNumber = false;
		}
		BOOST_FOREACH( data::ImageList::const_reference ref, images ) {
			if ( ! ref->hasProperty( "sequenceNumber" ) ) {
				LOG( Runtime, error )
						<< "sequenceNumber is missing, so I can't generate a unique filename. Won't write...";
				ret = false;
				continue;
			}
			std::stringstream sequenceNumber;
			std::stringstream sequenceCounter;
			sequenceCounter << std::setfill('0') << std::setw( imageDigitsImagePerSequenceCounter ) << boost::lexical_cast<std::string>(  ++imagePerSequenceCounter[ref->getProperty<u_int16_t>( "sequenceNumber" ) ] );
			sequenceNumber << std::setfill('0') << std::setw( imageDigitsSequence ) << ref->getProperty<std::string>( "sequenceNumber" );
			std::string unique_name = "";
			if ( uniqueSequenceNumber ) {
				unique_name = std::string( "S" ) + sequenceNumber.str() + "_" + file;
			}
			//if we have more than one image with the same sequenceNumber we have to add a second index to the filename to avoid overwriting
			else {
				unique_name = std::string( "S" ) + sequenceNumber.str() + "_" + sequenceCounter.str()  + "_" + file;
			}
			LOG( Runtime, info )   << "Writing image to " <<  path / unique_name;
			write( *ref, ( path / unique_name ).string(), dialect );
		}
	} else {
		write( *images.front(), ( path / file ).string(), dialect );
	}
}

bool FileFormat::hasOrTell( const std::string &name, const isis::util::PropMap &object, isis::LogLevel level )
{
	if ( object.hasProperty( name ) ) {
		return true;
	} else {
		LOG( Runtime, level ) << "Missing property " << util::MSubject( name );
		return false;
	}
}

void FileFormat::throwGenericError( std::string desc )
{
	throw( std::runtime_error( desc ) );
}

void FileFormat::throwSystemError( int err, std::string desc )
{
	throw( boost::system::system_error( err, boost::system::get_system_category(), desc ) );
}

const float FileFormat::invalid_float = -std::numeric_limits<float>::infinity();
}
}
