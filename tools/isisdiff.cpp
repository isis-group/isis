#include "DataStorage/io_factory.hpp"
#include "DataStorage/io_application.hpp"
#include <boost/foreach.hpp>
#include "CoreUtils/application.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

using namespace isis;

void dropDuplicate( std::list<data::Image> &list )
{
	for( std::list<data::Image>::iterator a = list.begin(); a != list.end(); a++ ) {
		std::list<data::Image>::iterator b = a;
		b++;

		while( b != list.end() ) {
			const util::PropertyMap &aref = *a, &bref = *b;

			if( aref.getDifference( bref ).empty() ) {
				std::cout << "Duplicate found in data from "
						  << bref.propertyValue( "source" ).toString( false ) << ":" << std::distance( list.begin(), b )
						  << ", dropping.." << std::endl;
				list.erase( b++ );
			} else
				++b;
		}
	}
}

std::pair<std::string, int>  parseFilename( std::string name )
{
	const boost::regex reg( "^([^:]*):([[:digit:]]+)$" );
	boost::cmatch results;
	std::pair<std::string, int> ret;
	ret.second = -1;

	if ( boost::regex_match( name.c_str(), results, reg ) ) {
		ret.first = results.str( results.size() - 2 );
		ret.second = boost::lexical_cast<int>( results.str( results.size() - 1 ) );
	} else
		ret.first = name;

	return ret;
}

bool hasDifferentProp( const data::Image &img, const util::PropertyMap::PropPath &pname, util::PropertyValue pval )
{
	return !( img.propertyValue( pname ) == pval ); // if property does not exist, an empty propertyValue is returned and those compare unequal to everything
}

std::list<data::Image> pickImg( int pos, std::list<data::Image> list )
{
	std::list< data::Image >::iterator at = list.begin();
	std::advance( at, pos );
	return std::list<data::Image>( 1, *at );
}

std::list<data::Image> findFitting( const data::Image reference, std::list<data::Image> images )
{
	images.remove_if( boost::bind( hasDifferentProp, _1, util::PropertyMap::PropPath( "sequenceNumber" ), reference.propertyValue( "sequenceNumber" ) ) );

	if( images.size() <= 1 )return images;

	images.remove_if( boost::bind( hasDifferentProp, _1, util::PropertyMap::PropPath( "sequenceDescription" ), reference.propertyValue( "sequenceDescription" ) ) );

	if( images.size() <= 1 )return images;

	images.remove_if( boost::bind( hasDifferentProp, _1, util::PropertyMap::PropPath( "sequenceStart" ), reference.propertyValue( "sequenceStart" ) ) );

	if( images.size() > 1 )
		std::cout << "still more than one image" << std::endl;;

	return images;
}

int main( int argc, char *argv[] )
{
	util::Application app( "isis data diff" );
	app.parameters["ignore"] = util::slist();
	app.parameters["ignore"].needed() = false;
	app.parameters["ignore"].setDescription( "List of properties which should be ignored when comparing" );


	data::IOApplication::addInput( app.parameters, true, "1", " of the first image" );
	data::IOApplication::addInput( app.parameters, true, "2", " of the second image" );

	if ( ! app.init( argc, argv ) ) return 1;

	std::pair<std::string, int > in1, in2;

	if( app.parameters["in1"]->as<util::slist>().size() == 1 )
		in1 = parseFilename( app.parameters["in1"]->as<util::slist>().front() );
	else
		in1.second = -1;

	if( app.parameters["in2"]->as<util::slist>().size() == 1 )
		in2 = parseFilename( app.parameters["in2"]->as<util::slist>().front() );
	else
		in2.second = -1;

	std::list<data::Image> images1, images2;

	if( in1.second >= 0 && in2.second >= 0 ) { // seems like we got numbers
		std::cout << "Comparing image " << in1.second << " from " << in1.first << " and Image " << in2.second  << " from " << in2.first << std::endl;
		app.parameters["in1"] = in1.first;
		std::list<data::Image> images;
		data::IOApplication::autoload( app.parameters, images, true, "1" );

		images1 = pickImg( in1.second, images );

		if( !in2.first.empty() ) {
			images.clear();
			data::IOApplication::autoload( app.parameters, images, true, "2" );
		}

		images2 = pickImg( in2.second, images );
	} else if( in1.second <= 0 && in2.second <= 0 ) {
		data::IOApplication::autoload( app.parameters, images1, true, "1" );
		data::IOApplication::autoload( app.parameters, images2, true, "2" );
		dropDuplicate( images1 );
		dropDuplicate( images2 );

		if ( images1.size() != images2.size() ) {
			std::cout << "Amount of found images in " << in1.first << "(" << images1.size() << ") and " << in1.first << "(" << images2.size() << ") differs" << std::endl;
			return -1;
		}
	} else {
		std::cerr << "Cannot mix single and multi image mode" << std::cerr;
	}



	int count;
	util::slist ignore = app.parameters["ignore"];
	ignore.push_back( "source" );
	size_t ret = 0;

	for ( std::list<data::Image>::const_iterator i = images1.begin(); i != images1.end(); i++, count++ ) {
		const data::Image &first = *i;

		const std::list<data::Image> candidates = findFitting( first, images2 );

		const data::Image &second = candidates.front();

		util::PropertyMap::DiffMap diff = first.getDifference( second );
		BOOST_FOREACH( util::slist::const_reference ref, ignore ) {
			diff.erase( util::istring( ref.begin(), ref.end() ) );
		}
		ret += diff.size();
		const std::string countStr = ( images1.size() > 1 ? std::string( ":" ) + boost::lexical_cast<std::string>( count ) : std::string( "" ) );

		if ( ! diff.empty() ) {
			std::cout
					<< "Metadata of " << app.parameters["in1"]->as<std::string>() << countStr  << " and "
					<< app.parameters["in2"]->as<std::string>() << countStr  << " differ:" << std::endl
					<< diff << std::endl;
		}

		if ( first.getSizeAsVector() != second.getSizeAsVector() ) {
			std::cout
					<< "Image sizes of " << app.parameters["in1"]->as<std::string>() << countStr << " and "
					<< app.parameters["in2"]->as<std::string>() << countStr  << " differ:"
					<< first.getSizeAsString() << "/" << second.getSizeAsString() << std::endl;
			ret++;
		} else {
			size_t voxels = first.compare( second );

			if ( voxels != 0 ) {
				std::cout << voxels * 100 / first.getSizeAsVector().product()
						  << "% of the voxels in " << app.parameters["in1"]->as<std::string>() << countStr << " and "
						  << app.parameters["in2"]->as<std::string>() << countStr  << " differ" << std::endl;
				ret++;
			}
		}
	}

	return ret;
}
