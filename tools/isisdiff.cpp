#include "DataStorage/io_factory.hpp"
#include "DataStorage/io_application.hpp"
#include <boost/foreach.hpp>
#include "CoreUtils/application.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>


struct DiffLog {static const char *name() {return "Diff";}; enum {use = _ENABLE_LOG};};
struct DiffDebug {static const char *name() {return "DiffDebug";}; enum {use = _ENABLE_DEBUG};};

using namespace isis;

boost::filesystem::path getCommonSource( std::list<boost::filesystem::path> sources )
{
	sources.erase( std::unique( sources.begin(), sources.end() ), sources.end() );

	if( sources.empty() ) {
		LOG( DiffLog, error ) << "Failed to get common source";
		return boost::filesystem::path();
	} else if( sources.size() == 1 )
		return sources.front();
	else {
		BOOST_FOREACH( boost::filesystem::path & ref, sources )
		ref.remove_filename();
		return getCommonSource( sources );
	}
}

boost::filesystem::path getCommonSource( const data::Image img )
{
	if( img.hasProperty( "source" ) )
		return img.getPropertyAs<std::string>( "source" );
	else {
		std::list<boost::filesystem::path> sources;
		BOOST_FOREACH( const util::PropertyValue & ref, img.getChunksProperties( "source", true ) ) {
			sources.push_back( ref->as<std::string>() );
		}
		return getCommonSource( sources );
	}
}

std::string identify( const data::Image &img )
{
	return
		getCommonSource( img ).file_string()
		+ "[S"
		+ img.getPropertyAs<std::string>( "sequenceNumber" )
		+ ( img.hasProperty( "sequenceDescription" ) ? ( "_" +     img.getPropertyAs<std::string>( "sequenceDescription" ) ) : "" )
		+ ( img.hasProperty( "sequenceStart" )       ? ( " from " + img.getPropertyAs<std::string>( "sequenceStart" ) )      : "" )
		+ "]";
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
bool hasSameProp( const data::Image &img, const util::PropertyMap::PropPath &pname, util::PropertyValue pval )
{
	return img.propertyValue( pname ) == pval; // if property does not exist, an empty propertyValue is returned and those compare unequal to everything
}

data::Image pickImg( int pos, std::list<data::Image> list )
{
	std::list< data::Image >::iterator at = list.begin();
	std::advance( at, pos );
	return *at;
}

bool doFit( const data::Image reference, std::list<data::Image> &org_images, std::list<data::Image> &images, const char *propName )
{
	const util::PropertyMap::PropPath propPath( propName );
	images.remove_if( boost::bind( hasDifferentProp, _1, propPath , reference.propertyValue( propPath ) ) );
	org_images.remove_if( boost::bind( hasSameProp, _1, propPath, reference.propertyValue( propPath ) ) );

	if( images.size() <= 1 ) {
		LOG( DiffDebug, info ) << "Found a unique image with the same " << propName << " " << util::MSubject( reference.propertyValue( propPath ) ) << ", returning....";
		return true;
	} else
		return false;
}

std::list<data::Image> findFitting( const data::Image reference, std::list<data::Image> &org_images )
{
	std::list< data::Image > images = org_images;
	const char *props[] = {"sequenceNumber", "sequenceDescription", "sequenceStart"};

	BOOST_FOREACH( const char * prop, props ) {
		if( doFit( reference, org_images, images, prop ) )
			return images;
	}
	return images;
}

bool diff( const data::Image &img1, const data::Image &img2, const util::slist &ignore )
{
	bool ret = false;
	util::PropertyMap::DiffMap diff = img1.getDifference( img2 );
	BOOST_FOREACH( util::slist::const_reference ref, ignore ) {
		diff.erase( util::istring( ref.begin(), ref.end() ) );
	}
	const std::string name1 = identify( img1 );
	const std::string name2 = identify( img2 );
	LOG( DiffLog, info ) << "Comparing " << name1 << " and " << name2;

	if ( ! diff.empty() ) {
		std::cout
				<< "Metadata of " << name1 << " and " << name2  << " differ:" << std::endl
				<< diff << std::endl;
		ret = true;
	}

	if ( img1.getSizeAsVector() != img2.getSizeAsVector() ) {
		std::cout
				<< "Image sizes of " << name1 << " and " << name2  << " differ:"
				<< img1.getSizeAsString() << "/" << img2.getSizeAsString() << std::endl;
		ret = true;
	} else {
		size_t voxels = img1.compare( img2 );

		if ( voxels != 0 ) {
			std::cout << voxels * 100 / img1.getVolume() << "% of the voxels in " << name1 << " and "
					  << name2  << " differ" << std::endl;
			ret = true;
		}
	}

	return ret;
}

void dropWith( util::slist props, std::list< data::Image > &images )
{
	BOOST_FOREACH( util::slist::const_reference propStr, props ) {
		const std::list< std::string > ppair = util::stringToList<std::string>( propStr, '=' );
		images.remove_if( boost::bind( hasSameProp, _1, ppair.front().c_str(), ppair.back() ) );
	}
}

int main( int argc, char *argv[] )
{
	util::Application app( "isis data diff" );
	app.parameters["ignore"] = util::slist();
	app.parameters["ignore"].needed() = false;
	app.parameters["ignore"].setDescription( "List of properties which should be ignored when comparing" );

	app.parameters["skipwith"] = util::slist( 1, "sequenceDescription=localizer" );
	app.parameters["skipwith"].needed() = false;
	app.parameters["skipwith"].setDescription( "List of property=value sets which should should make the program skip the according image" );

	app.addLogging<DiffLog>( "" );
	app.addLogging<DiffDebug>( "" );

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

	size_t ret = 0;
	std::list<data::Image> images1, images2;
	util::slist ignore = app.parameters["ignore"];
	ignore.push_back( "source" );
	boost::shared_ptr<util::ConsoleFeedback> feedback( new util::ConsoleFeedback );

	if( in1.second >= 0 && in2.second >= 0 ) { // seems like we got numbers
		LOG( DiffLog, notice ) << "Comparing image " << in1.second << " from " << in1.first << " and Image " << in2.second  << " from " << in2.first;
		app.parameters["in1"] = util::slist( 1, in1.first );
		std::list<data::Image> images;
		data::IOApplication::autoload( app.parameters, images, true, "1", feedback );

		const data::Image first = pickImg( in1.second, images );

		if( !in2.first.empty() ) {
			images.clear();
			app.parameters["in2"] = util::slist( 1, in2.first );
			data::IOApplication::autoload( app.parameters, images, true, "2", feedback );
		}

		const data::Image second = pickImg( in2.second, images );

		if( diff( first, second, ignore ) )
			ret = 1;

	} else if( in1.second <= 0 && in2.second <= 0 ) {
		data::IOApplication::autoload( app.parameters, images1, false, "1", feedback );
		data::IOApplication::autoload( app.parameters, images2, false, "2", feedback );
		dropWith( app.parameters["skipwith"], images1 );
		dropWith( app.parameters["skipwith"], images2 );

		LOG( DiffLog, notice ) << "Comparing " << images1.size() << " images from \"" << in1.first << "\" and " << images2.size() << " from \"" << in2.first << "\"";

		for (
			std::list< data::Image >::iterator first = images1.begin();
			first != images1.end() && !images1.empty();
		) {
			const std::list<data::Image> candidates = findFitting( *first, images2 );
			LOG_IF( candidates.size() > 1 || candidates.empty(), DiffLog, warning )
					<< "Could not find a unique image fitting " << identify( *first )
					<< ". " << candidates.size() << " where found";

			BOOST_FOREACH( const data::Image & second, candidates ) {
				if( diff( *first, second, ignore ) )
					ret++;
			}

			if( !candidates.empty() ) { // if we did a comparison - remove the image from the compare list
				images1.erase( first++ );
			} else
				++first; // of not skip that one, and go on
		}

		if( !images1.empty() ) {
			std::cout << "there are " << images1.size() << " images left uncompared from " << util::MSubject( in1.first ) << std::endl;
			ret += images1.size();
		}

		if( !images2.empty() ) {
			std::cout << "there are " << images2.size() << " images left uncompared from " << util::MSubject( in2.first ) << std::endl;
			ret += images2.size();
		}
	} else {
		LOG( DiffLog, error ) << "Cannot mix single and multi image mode, exiting.";
		return -1;
	}




	return ret;
}
