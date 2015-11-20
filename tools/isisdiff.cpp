#include "data/io_factory.hpp"
#include "data/io_application.hpp"
#include "util/application.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <boost/regex.hpp>


struct DiffLog {static const char *name() {return "Diff";}; enum {use = _ENABLE_LOG};};
struct DiffDebug {static const char *name() {return "DiffDebug";}; enum {use = _ENABLE_DEBUG};};

static const char *_props[] = {"sequenceNumber", "sequenceDescription", "sequenceStart", "indexOrigin"};
static const char *_skips[] = {"sequenceDescription=localizer"};

using namespace isis;

std::pair<std::string, int>  parseFilename( std::string name )
{
	const boost::regex reg( "^([^:]*):([[:digit:]]+)$" );
	boost::cmatch results;
	std::pair<std::string, int> ret;
	ret.second = -1;

	if ( boost::regex_match( name.c_str(), results, reg ) ) {
		ret.first = results.str( results.size() - 2 );
		ret.second = std::stoi( results.str( results.size() - 1 ) );
	} else
		ret.first = name;

	return ret;
}

bool hasSameProp( const data::Image &img, const util::PropertyMap::PropPath &pname, const util::PropertyValue pval )
{
	return 
	( !img.hasProperty( pname ) && pval.isEmpty() ) || 
	img.property( pname ) == pval; // if property does not exist, an empty propertyValue is returned and those compare unequal to everything
}
bool hasDifferentProp( const data::Image &img, const util::PropertyMap::PropPath &pname, const util::PropertyValue pval )
{
	return !hasSameProp( img, pname, pval ); // if property does not exist, an empty propertyValue is returned and those compare unequal to everything
}

data::Image pickImg( int pos, std::list<data::Image> list )
{
	std::list< data::Image >::iterator at = list.begin();
	std::advance( at, pos );

	if( at == list.end() ) {
		LOG( DiffLog, error ) << "Sorry, there is no image " << pos;
		throw( std::logic_error( std::string( "no " ) + std::to_string( pos ) + "th image" ) );
	}

	return *at;
}

size_t doFit( const data::Image reference, std::list<data::Image> &org_images, std::list<data::Image> &images, const char *propName )
{
	//first time org is empty - images is full
	if(propName[0]==0){
		LOG(DiffLog,warning) << "Empty property names are invalid! Use \'-selectwith\' instead of \'-selectwith \"\"\'.";
	} else {
		const util::PropertyMap::PropPath propPath( propName );
		util::PropertyValue propval = reference.property( propPath );

		//now move all with different prop back into org
		for( std::list<data::Image>::iterator i = images.begin(); i != images.end(); ) {
			if( hasDifferentProp( *i, propPath, propval ) ) {
				org_images.push_back( *i );
				images.erase( i++ );
			} else
				i++;
		}

		LOG( DiffLog, info )
			<< images.size() << " candidates left for " << reference.identify() << ", "
			<< org_images.size() << " not considered after checking for " << propName << "=" << reference.property( propPath );
	}
	return images.size();
}

boost::filesystem::path getCommonSource(const std::list<data::Image> &images){
	
	std::list<std::string> sources;
	for(const data::Image &img:images){
		if(img.hasProperty("source"))
			sources.push_back(img.getValueAs<std::string>("source"));
		std::list<std::string> s=img.getChunksValuesAs<std::string>("source",true);
		sources.splice(sources.end(),s);
	}
	return util::getRootPath(std::list<boost::filesystem::path>(sources.begin(),sources.end()));
}

std::list<data::Image> findFitting( const data::Image reference, std::list<data::Image> &org_images, const util::slist &props )
{
	std::list< data::Image > images;
	images.splice( images.begin(), org_images ); //first move all into images

	for( const std::string & prop :  props ) {
		if( doFit( reference, org_images, images, prop.c_str() ) < 1 ) //now move all with different prop back into org
			return images; // abort if no image is left
	}
	return images;
}

bool diff( const data::Image &img1, const data::Image &img2, const util::slist &ignore )
{
	bool ret = false;
	util::PropertyMap::DiffMap diff = img1.getDifference( img2 );
	diff.erase( "source" ); //its kinda obvious that images from different sources have different source flag (and its useless to consider this a difference anyway)
	for( util::slist::const_reference ref :  ignore )
		diff.erase( util::istring( ref.begin(), ref.end() ) );

	const std::string name1 = img1.identify();
	const std::string name2 = img2.identify();
	LOG( DiffLog, info ) << "Comparing " << name1 << " and " << name2;

	if ( ! diff.empty() ) {
		std::cout
				<< "Metadata of " << std::endl << name1 << " and " << std::endl << name2  << " differ:" << std::endl
				<< diff << std::endl;
		ret = true;
	}

	if ( img1.getSizeAsVector() != img2.getSizeAsVector() ) {
		std::cout
				<< "Image sizes of " << std::endl << name1 << " and " << std::endl << name2 << " differ:"
				<< img1.getSizeAsString() << "/" << img2.getSizeAsString() << std::endl;
		ret = true;
	} else {
		size_t voxels = img1.compare( img2 );

		if ( voxels != 0 ) {
			std::cout << voxels * 100. / img1.getVolume() << "% of the voxels in " << std::endl << name1 << " and " << std::endl << name2 << " differ" << std::endl;
			ret = true;
		}
	}

	return ret;
}

void dropWith( util::slist props, std::list< data::Image > &images )
{
	for( util::slist::const_reference propStr :  props ) {
		const std::list< std::string > ppair = util::stringToList<std::string>( propStr, '=' );
		images.remove_if( boost::bind( hasSameProp, _1, ppair.front().c_str(), util::PropertyValue( ppair.back() ) ) );
	}
}

int main( int argc, char *argv[] )
{
	util::Application app( "isis data diff" );
	app.parameters["ignore"] = util::slist();
	app.parameters["ignore"].needed() = false;
	app.parameters["ignore"].setDescription( "List of properties which should be ignored when comparing" );

	app.parameters["skipwith"] = util::slist( _skips, _skips + sizeof( _skips ) / sizeof( char * ) );
	app.parameters["skipwith"].needed() = false;
	app.parameters["skipwith"].setDescription( "List of property=value sets which should should make the program skip the according image" );

	app.parameters["selectwith"] = util::slist( _props, _props + sizeof( _props ) / sizeof( char * ) );
	app.parameters["selectwith"].needed() = false;
	app.parameters["selectwith"].setDescription( "List of properties which should be used to select images for comparison" );

	app.addLogging<DiffLog>();
	app.addLogging<DiffDebug>();

	data::IOApplication::addInput( app.parameters, " of the first image", "1" );
	data::IOApplication::addInput( app.parameters, " of the second image", "2" );

	app.addExample( "-in1 orphaned_data/ -in2 /archive/archived.dataset/ -ignore DICOM/PatientID",
					"Check if (and where) a \"found\" dataset differs from one in your archive ignoring different \"DICOM/PatientID\"s (in case you anonymize your archive)." );

	app.addExample( "-in1 dicom_dataset:3 -in2 :4",
					"Check for differences between the third and the fourth image found in a directory of DICOM files." );

	if ( ! app.init( argc, argv ) ) return 1;

	std::pair<std::string, int > in1, in2;

	if( app.parameters["in1"].as<util::slist>().size() == 1 )
		in1 = parseFilename( app.parameters["in1"].as<util::slist>().front() );
	else
		in1.second = -1;

	if( app.parameters["in2"].as<util::slist>().size() == 1 )
		in2 = parseFilename( app.parameters["in2"].as<util::slist>().front() );
	else
		in2.second = -1;

	size_t ret = 0;
	std::list<data::Image> images1, images2;
	util::slist ignore = app.parameters["ignore"];
	ignore.push_back( "source" );
	std::shared_ptr<util::ConsoleFeedback> feedback( new util::ConsoleFeedback );

	if( in1.second >= 0 && in2.second >= 0 ) { // seems like we got numbers
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

		LOG( DiffLog, info ) << "Comparing single images " << first.identify() << " and " << second.identify();

		if( diff( first, second, ignore ) )
			ret = 1;

	} else if( in1.second <= 0 && in2.second <= 0 ) {
		data::IOApplication::autoload( app.parameters, images1, false, "1", feedback );
		data::IOApplication::autoload( app.parameters, images2, false, "2", feedback );
		dropWith( app.parameters["skipwith"], images1 );
		dropWith( app.parameters["skipwith"], images2 );

		util::slist src1 = app.parameters["in1"], src2 = app.parameters["in2"];
		boost::filesystem::path sPath1 = ( src1.size() == 1 ) ? src1.front() : getCommonSource( images1 );
		boost::filesystem::path sPath2 = ( src2.size() == 1 ) ? src2.front() : getCommonSource( images2 );


		LOG( DiffLog, notice ) << "Comparing " << images1.size() << " images from \"" << sPath1 << "\" and " << images2.size() << " from \"" << sPath2 << "\"";

		for (
			std::list< data::Image >::iterator first = images1.begin();
			first != images1.end() && !images1.empty();
		) {
			const std::list<data::Image> candidates = findFitting( *first, images2, app.parameters["selectwith"] );
			LOG_IF( candidates.size() > 1 || candidates.empty(), DiffLog, warning )
					<< "Could not find a unique image fitting " << first->identify()
					<< ". " << candidates.size() << " where found";

			for( const data::Image & second :  candidates ) {
				if( diff( *first, second, ignore ) )
					ret++;
			}

			if( !candidates.empty() ) { // if we did a comparison - remove the image from the compare list
				images1.erase( first++ );
			} else
				++first; // of not skip that one, and go on
		}

		if( !images1.empty() ) {
			std::cout << "there are " << images1.size() << " images left uncompared from " << util::MSubject( in1.first ) << "(in1)" << std::endl;
			ret += images1.size();
		}

		if( !images2.empty() ) {
			std::cout << "there are " << images2.size() << " images left uncompared from " << util::MSubject( in2.first ) << "(in2)" << std::endl;
			ret += images2.size();
		}
	} else {
		LOG( DiffLog, error ) << "Cannot mix single and multi image mode, exiting.";
		return -1;
	}




	return ret;
}
