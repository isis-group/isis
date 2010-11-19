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
namespace _internal{
bool moreCmp(const util::istring &a,const util::istring &b){return a.length()>b.length();}
}

void FileFormat::write( const isis::data::ImageList &images, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & )
{
	std::list<std::string> names=makeUniqueFilenames(images,filename);
	std::list<std::string>::const_iterator inames=names.begin();
	BOOST_FOREACH( data::ImageList::const_reference ref, images ) {
		std::string uniquePath=*(inames++);
		LOG( Runtime, info )   << "Writing image to " <<  uniquePath;
		try{
			write( *ref, uniquePath, dialect );
		} catch ( std::runtime_error &e ) {
			LOG( Runtime, warning )
					<< "Failed to write image to " <<  uniquePath << " using " <<  name() << " (" << e.what() << ")";
		}
	}
}

bool FileFormat::hasOrTell( const util::PropMap::key_type &name, const isis::util::PropMap &object, isis::LogLevel level )
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

std::list< util::istring > FileFormat::getSuffixes()const
{
	std::list<util::istring> ret=util::string2list<util::istring>( suffixes(), boost::regex( "[[:space:]]" ) );
	BOOST_FOREACH(util::istring &ref,ret)
	{
		ref.erase(0,ref.find_first_not_of('.'));// remove leading . if there are some
	}
	ret.sort(_internal::moreCmp);//start with the longest suffix
	return ret;
}

std::pair< std::string, std::string > FileFormat::makeBasename(const std::string& filename)const
{
	std::list<util::istring> suffixes=getSuffixes();
	util::istring ifilename(filename.begin(),filename.end());
	BOOST_FOREACH(const util::istring &suff,suffixes){
		size_t at=ifilename.rfind(suff);
		if(at!=ifilename.npos){
			if(at && ifilename[at-1]=='.')
				at--;
			return std::make_pair(filename.substr(0,at),filename.substr(at));
		}
	}
	return std::make_pair(filename,std::string());
}

std::string FileFormat::makeFilename(const util::PropMap &props,std::string namePattern)
{
	boost::regex reg( "\\{[^{}]+\\}" );
	boost::match_results<std::string::iterator> what;
	while(boost::regex_search(namePattern.begin(),namePattern.end() , what, reg ))
	{
		const util::PropMap::key_type prop(what[0].str().substr(1,what.length()-2).c_str());
		if(props.hasProperty(prop)){
			namePattern.replace(what[0].first,what[0].second,props.getProperty<std::string>(prop));
			LOG(Debug,info)
			<< "Replacing " << util::MSubject(util::PropMap::key_type("{")+prop+"}") << " by "	<< util::MSubject( props.getProperty<std::string>(prop) )
			<< " the string is now " << util::MSubject(namePattern);
		} else{
			LOG(Runtime,warning) << "The property " << util::MSubject(prop) << " does not exist - ignoring it";
			namePattern.replace(what[0].first,what[0].second,""); // it must be removed, or it will match forever
		}
	}

	return namePattern;
}
std::list<std::string> FileFormat::makeUniqueFilenames(const data::ImageList& images, const std::string& namePattern)const
{
	std::list<std::string> ret;
	std::map<std::string,unsigned short> names,used_names;
	BOOST_FOREACH(data::ImageList::const_reference ref,images){
		names[makeFilename(*ref,namePattern)]++;
	}
	
	BOOST_FOREACH(data::ImageList::const_reference ref,images){
		std::string name=makeFilename(*ref,namePattern);
		if(names[name]>1){
			const unsigned short number=++used_names[name];
			const unsigned short length=(uint16_t)log10(names[name])-(uint16_t)log10(number);
			const std::string snumber=std::string(length,'0')+boost::lexical_cast<std::string>(number);
			const std::pair<std::string,std::string> splitted=makeBasename(name);
			name =splitted.first+"_"+snumber+splitted.second;
		}
		ret.push_back(name);
	}
	return ret;
}

const float FileFormat::invalid_float = -std::numeric_limits<float>::infinity();
}
}
