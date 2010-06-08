#include "DataStorage/io_interface.h"
#include "boost/foreach.hpp"
#include "CoreUtils/log.hpp"
#include <boost/filesystem.hpp>
#include "ImageIO/common.hpp"

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

	if ( images.size() > 1 ) {
		BOOST_FOREACH( data::ImageList::const_reference ref, images ) {
			if ( not ref->hasProperty( "sequenceNumber" ) ) {
				LOG( Runtime, error )
						<< "sequenceNumber is missing, so I can't generate a unique filename. Won't write...";
				ret = false;
				continue;
			}

			std::string unique_name = std::string( "S" ) + ref->getProperty<std::string>( "sequenceNumber" ) + "_" + file;
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
