#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>
#include "CoreUtils/application.hpp"
#include <boost/lexical_cast.hpp>

using namespace isis;

void dropDuplicate( data::ImageList &list )
{
	for( data::ImageList::iterator a = list.begin(); a != list.end(); a++ ) {
		data::ImageList::iterator b = a;
		b++;

		while( b != list.end() ) {
			const util::PropMap &aref = **a, bref = **b;

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

std::pair<std::string,int>  parseFilename(std::string name){
	const boost::regex reg( "^([^:]*):([[:digit:]]+)$" );
	boost::cmatch results;
	std::pair<std::string,int> ret;ret.second=-1;

	if ( boost::regex_match( name.c_str(), results, reg ) ) {
		ret.first=results.str(results.size()-2);
		ret.second=boost::lexical_cast<int>(results.str(results.size()-1));
	} else
		ret.first=name;

	return ret;
}

int main( int argc, char *argv[] )
{
	util::Application app( "isis data diff" );
	app.parameters["ignore"] = util::slist();
	app.parameters["ignore"].needed() = false;
	app.parameters["ignore"].setDescription( "List of properties which should be ignored when comparing" );
	app.parameters["in"] = util::slist();
	app.parameters["in"].setDescription( "The two files (or directories) which you want to compare" );

	if ( ! app.init( argc, argv ) )
		return 1;

	int ret = 0;
	util::slist files = app.parameters["in"];

	if ( files.size() != 2 ) {
		std::cout << "You'll have to give two input files" << std::endl << "(but you gave " << files << ")" << std::endl;
		return 1;
	}

	std::pair<std::string,int> in1=parseFilename(files.front()),in2=parseFilename(files.back());
	data::ImageList images1;
	data::ImageList images2;

	if(in1.second >=0 && in2.second >=0){ // seems like we got numbers
		assert(!in1.first.empty());
		data::ImageList erg=data::IOFactory::load( in1.first );
		assert(erg.size()>in1.second);
		data::ImageList::iterator at=erg.begin();std::advance(at,in1.second);
		images1.push_back(*at);
		if(!in2.first.empty()){
			erg=data::IOFactory::load( in2.first );
		}
		assert(erg.size()>in2.second);
		at=erg.begin();std::advance(at,in2.second);
		images2.push_back(*at);
	} else {
		images1 = data::IOFactory::load( in1.first );
		images2 = data::IOFactory::load( in2.first );
		dropDuplicate( images1 );
		dropDuplicate( images2 );
	}

	if ( images1.size() != images2.size() ) {
		std::cout << "Amount of found images in " << files.front() << "(" << images1.size() << ") and " << files.back() << "(" << images2.size() << ") differs" << std::endl;
		return -1;
	}

	data::ImageList::const_iterator i, j;
	int count;
	util::slist ignore = app.parameters["ignore"];
	ignore.push_back( "source" );

	for ( i = images1.begin(), j = images2.begin(), count = 0; i != images1.end(); i++, j++, count++ ) {
		const data::Image &first = **i, &second = **j;
		util::PropMap::diff_map diff = first.getDifference( second );
		BOOST_FOREACH( util::slist::const_reference ref, ignore )
		diff.erase( ref );
		ret += diff.size();
		const std::string countStr=(images1.size()>1 ? std::string(":") + boost::lexical_cast<std::string>(count) :std::string(""));

		if ( ! diff.empty() ) {
			std::cout
					<< "Metadata of " << files.front() << countStr  << " and "
					<< files.back() << countStr  << " differ:" << std::endl
					<< diff << std::endl;
		}

		if ( first.getSizeAsVector() != second.getSizeAsVector() ) {
			std::cout
					<< "Image sizes of " << files.front() << countStr << " and "
					<< files.back() << countStr  << " differ:"
					<< first.getSizeAsString() << "/" << second.getSizeAsString() << std::endl;
			ret++;
		} else {
			size_t voxels = first.cmp( second );

			if ( voxels != 0 ) {
				std::cout << voxels*100/first.getSizeAsVector().product() 
						  << "% of the voxels in " << files.front() << countStr << " and "
						  << files.back() << countStr  << " differ" << std::endl;
				ret++;
			}
		}
	}

	return ret;
}
