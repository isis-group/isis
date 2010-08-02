/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "io_application.hpp"
#include "io_factory.hpp"
#include <boost/mpl/for_each.hpp>

namespace isis
{
namespace data
{
IOApplication::IOApplication( const char name[], bool have_input, bool have_output ): Application( name ), m_input( have_input ), m_output( have_output )
{
	data::IOFactory::setProgressFeedback( &feedback );

	if ( have_input ) {
		parameters["in"] = std::string();
		parameters["in"].setDescription( "input file or dataset" );
		parameters["rf"] = std::string();
		parameters["rf"].needed() = false;
		parameters["rf"].setDescription( "Override automatic detection of file suffix for reading with given value" );
		parameters["rdialect"] = std::string();
		parameters["rdialect"].needed() = false;
		parameters["rdialect"].setDescription(
			"choose dialect for reading. The available dialects depend on the capabilities of IO plugins" );
	}

	if ( have_output ) {
		parameters["out"] = std::string();
		parameters["out"].setDescription( "output file" );
		parameters["wf"] = std::string();
		parameters["wf"].needed() = false;
		parameters["wf"].setDescription( "Override automatic detection of file suffix for writing with given value" );
		parameters["wdialect"] = std::string();
		parameters["wdialect"].needed() = false;
		parameters["wdialect"].setDescription( "choose dialect for writing. The available dialects depend on the capabilities of IO plugins" );
		std::map<unsigned short, std::string> types = util::getTypeMap( true, false );
		// remove some types which are useless as representation
		// "(unsigned short)" is needed because otherwise erase would take the reference of a static constant which is only there during compile time
		types.erase( ( unsigned short )util::Type<util::Selection>::staticID );
		types.erase( ( unsigned short )util::Type<std::string>::staticID );
		types.erase( ( unsigned short )util::Type<boost::posix_time::ptime>::staticID );
		types.erase( ( unsigned short )util::Type<boost::gregorian::date>::staticID );
		types.erase( ( unsigned short )util::Type<util::ilist>::staticID );
		types.erase( ( unsigned short )util::Type<util::dlist>::staticID );
		types.erase( ( unsigned short )util::Type<util::slist>::staticID );
		parameters["repn"] = util::Selection( types );
		parameters["repn"].needed() = false;
		parameters["repn"].setDescription(
			"Representation in which the data shall be written (not implemented yet)." );
		//      boost::mpl::for_each<util::_internal::types>( repnGeneratorGenerator(repnGenerators) );
	}
}

IOApplication::~IOApplication()
{
	data::IOFactory::setProgressFeedback( 0 );
}

bool IOApplication::init( int argc, char **argv, bool exitOnError )
{
	if ( ! util::Application::init( argc, argv, exitOnError ) )
		return false;

	if ( m_input ) {
		return autoload( exitOnError );
	}

	return true;
}
bool IOApplication::autoload( bool exitOnError )
{
	std::string input = parameters["in"];
	std::string rf = parameters["rf"];
	std::string dl = parameters["rdialect"];
	LOG( Runtime, info )
			<< "loading " << util::MSubject( input )
			<< ( rf.empty() ? "" : std::string( " using the format: " ) + rf )
			<< ( ( !rf.empty() && !dl.empty() ) ? " and" : "" )
			<< ( dl.empty() ? "" : std::string( " using the dialect: " ) + dl );
	images = data::IOFactory::load( input, rf, dl );

	if ( images.empty() ) {
		if ( exitOnError )
			exit( 1 );
		else
			return false;
	} else {
		for( ImageList::const_iterator a = images.begin(); a != images.end(); a++ ) {
			for( ImageList::const_iterator b = a; ( ++b ) != images.end(); ) {
				const util::PropMap &aref = **a, bref = **b;
				LOG_IF( aref.getDifference( bref ).empty(), Runtime, warning ) << "The metadata of the images from "
						<< aref.propertyValue( "source" ).toString( false ) << ":" << std::distance<ImageList::const_iterator>( images.begin(), a )
						<< " and " << bref.propertyValue( "source" ).toString( false ) << ":" << std::distance<ImageList::const_iterator>( images.begin(), b )
						<< " are equal. Maybe they are duplicates.";
			}
		}
	}

	return true;
}
bool IOApplication::autowrite( const ImageList &out_images, bool exitOnError )
{
	const util::Selection repn = parameters["repn"];
	const std::string output = parameters["out"];
	const std::string wf = parameters["wf"];
	const std::string dl = parameters["wdialect"];
	ImageList convertedList;

	switch ( ( int )repn ) {
	case util::Type<int8_t>::staticID:
		convertedList = convertTo<int8_t>( out_images );
		break;
	case util::Type<u_int8_t>::staticID:
		convertedList = convertTo<u_int8_t>( out_images );
		break;
	case util::Type<int16_t>::staticID:
		convertedList = convertTo<int16_t>( out_images );
		break;
	case util::Type<u_int16_t>::staticID:
		convertedList = convertTo<u_int16_t>( out_images );
		break;
	case util::Type<int32_t>::staticID:
		convertedList = convertTo<int32_t>( out_images );
		break;
	case util::Type<u_int32_t>::staticID:
		convertedList = convertTo<u_int32_t>( out_images );
		break;
	case util::Type<float>::staticID:
		convertedList = convertTo<float>( out_images );
		break;
	case util::Type<double>::staticID:
		convertedList = convertTo<double>( out_images );
		break;
	default:

		if( parameters["repn"].toString() != "<<NOT_SET>>" ) {
			std::string oldRepn = util::getTypeMap()[out_images.front()->typeID()];
			oldRepn.erase( oldRepn.size() - 1, oldRepn.size() );
			LOG( DataLog, warning ) << "Pixel representation " << parameters["repn"].toString()
									<< " not yet supported. Retaining "
									<< oldRepn << ".";
		}

		convertedList = out_images;
		break;
	}

	LOG( Runtime, info )
			<< "Writing " << out_images.size() << " images"
			//  << (repn ? std::string(" as ") + (std::string)repn : "")
			<< " to " << util::MSubject( output )
			<< ( wf.empty() ? "" : std::string( " using the format: " ) + wf )
			<< ( ( !wf.empty() && !dl.empty() ) ? " and" : "" )
			<< ( dl.empty() ? "" : std::string( " using the dialect: " ) + dl );

	if ( ! IOFactory::write( convertedList, output, wf, dl ) ) {
		if ( exitOnError )
			exit( 1 );

		return false;
	} else
		return true;
}

}
}

