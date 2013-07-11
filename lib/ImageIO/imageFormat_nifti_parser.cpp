/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  Enrico Reimer <reimer@cbs.mpg.de>

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

// #define BOOST_SPIRIT_DEBUG_PRINT_SOME 50
// #define BOOST_SPIRIT_DEBUG_INDENT 5

#include <boost/foreach.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "imageFormat_nifti_parser.hpp"

namespace isis
{
namespace image_io
{
namespace _internal
{

extern const char dcmmeta_root[] = "DcmMeta";
extern const char dcmmeta_global[] = "DcmMeta/global";
extern const char dcmmeta_perslice_data[] = "DcmMeta/global/slices";
extern const char dcmmeta_const_data[] = "DcmMeta/global/const";

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

typedef BOOST_TYPEOF( ascii::space | '\t' | boost::spirit::eol ) SKIP_TYPE;
typedef data::ValueArray< uint8_t >::iterator ch_iterator;
typedef qi::rule<ch_iterator, isis::util::PropertyMap(), SKIP_TYPE>::context_type PropertyMapContext;
typedef boost::variant<util::PropertyValue, util::PropertyMap> value_cont;

struct add_member {
	char extra_token;
	add_member( char _extra_token ): extra_token( _extra_token ) {}
	void operator()( const fusion::vector2<std::string, value_cont> &a, PropertyMapContext &context, bool & )const {
		const value_cont &container = a.m1;
		const util::PropertyMap::PropPath label = extra_token ?
				util::stringToList<util::PropertyMap::KeyType>( a.m0, extra_token ) :
				util::PropertyMap::PropPath( a.m0.c_str() );
		util::PropertyMap &target = context.attributes.car;

		if( target.hasBranch( label ) || target.hasProperty( label ) ) {
			LOG( Runtime, error ) << "There is already an entry " << target << " skipping this one" ;
		} else {
			switch( container.which() ) {
			case 1:
				target.branch( label ) = boost::get<util::PropertyMap>( container );
				break;
			case 0:
				target.propertyValue( label ) = boost::get<util::PropertyValue>( container );
				break;
			}
		}
	}
};

struct flattener { // simply insert subarrays into the array above
	template<typename T, typename CONTEXT> void operator()( const std::list<T> &a, CONTEXT &context, bool & )const {
		std::list<T> &cont = context.attributes.car;
		cont.insert( cont.end(), a.begin(), a.end() );
	}
};

template<typename T> struct read {typedef qi::rule<ch_iterator, T(), SKIP_TYPE> rule;};

bool parse_json( isis::data::ValueArray< uint8_t > stream, isis::util::PropertyMap &json_map, char extra_token )
{
	using qi::lit;
	using namespace boost::spirit;

	int version;
	read<value_cont>::rule value;
	read<std::string>::rule string( lexeme['"' >> *( ascii::print - '"' ) >> '"'], "string" ), label( string >> ':', "label" );
	read<fusion::vector2<std::string, value_cont> >::rule member( label >> value, "member" );
	read<isis::util::PropertyMap>::rule object( lit( '{' ) >> member[add_member( extra_token )] % ',' >> '}', "object" );
	read<int>::rule integer(int_ >> !lit('.'),"integer") ;
	read<util::dlist>::rule dlist( lit( '[' ) >> ( ( double_[phoenix::push_back( _val, _1 )] | dlist[flattener()] ) % ',' ) >> ']', "dlist" );
	read<util::ilist>::rule ilist( lit( '[' ) >> ( ( integer[phoenix::push_back( _val, _1 )] | ilist[flattener()] ) % ',' ) >> ']', "ilist" );
	read<util::slist>::rule slist( lit( '[' ) >> ( ( string [phoenix::push_back( _val, _1 )] | slist[flattener()] ) % ',' ) >> ']', "slist" );

	value = string | integer | double_ | slist | ilist | dlist |  object;

	data::ValueArray< uint8_t >::iterator begin = stream.begin(), end = stream.end();
	bool erg = phrase_parse( begin, end, object[phoenix::ref( json_map )=_1], ascii::space | '\t' | eol );
	return end == stream.end();
}

template<typename T> bool propDemux(std::list<T> props,std::list<data::Chunk> &chunks,util::PropertyMap::PropPath name){
	if( (props.size()%chunks.size())!=0){
		LOG(Runtime,error) << "The length of the per-slice property DcmMeta entry " << util::MSubject(name) << " does not fit the number of slices, skipping (" << props.size() << "!=" << chunks.size() << ").";
		return false;
	}
	typename std::list<T>::const_iterator prop=props.begin();
	size_t stride=props.size()/chunks.size();
	BOOST_FOREACH(data::Chunk &ch,chunks){
		if(stride==1){
			ch.setPropertyAs<T>(name,*(prop++));
		} else if(stride>1) {
			typename std::list<T>::const_iterator buff=prop;
			std::advance(prop,stride);
			std::list<T> &ref=((util::PropertyValue &)ch.setPropertyAs(name, std::list<T>())).castTo<std::list<T> >();// work around gcc bug
			ref.insert(ref.end(), buff,prop);
		}
	}
	return true;
}


void demuxDcmMetaSlices(std::list<data::Chunk> &chunks, util::PropertyMap &dcmmeta ){
	static const util::PropertyMap::PropPath slicesBranch(dcmmeta_perslice_data);
	if(dcmmeta.hasBranch(slicesBranch)){
		size_t stride=1;
		if(chunks.front().getRelevantDims()==4){ //if we have an 4D-Chunk
			assert(chunks.size()==1); //there can only be one
			chunks=chunks.front().autoSplice(chunks.front().getDimSize(data::sliceDim)); // replace that one by its 3D parts
		}
		for(std::list<data::Chunk>::iterator c=chunks.begin();c!=chunks.end();){ // if we have 3D chunks splice each
			if(c->getRelevantDims()>2){
				std::list< data::Chunk > insert = c->autoSplice(1);
				chunks.insert(c,insert.begin(),insert.end()); // insert spliced chunks back into list
				chunks.erase(c++); // and remove original
			} else
				++c;
		}
		BOOST_FOREACH(const util::PropertyMap::FlatMap::value_type &ppair,dcmmeta.branch(slicesBranch).getFlatMap()){
			bool success;
			util::PropertyMap::PropPath path(util::istring( "DcmMeta/global/"+ppair.first)); // move the prop one branch up (without "slices")
			switch(ppair.second.getTypeID()){
				case util::Value<util::ilist>::staticID:success=propDemux(ppair.second.castTo<util::ilist>(),chunks,path);break;
				case util::Value<util::dlist>::staticID:success=propDemux(ppair.second.castTo<util::dlist>(),chunks,path);break;
				case util::Value<util::slist>::staticID:success=propDemux(ppair.second.castTo<util::slist>(),chunks,path);break;
				default:
					LOG(Runtime,error) << "The the type of " << path << " (" << ppair.second.getTypeName() <<  ") is not a list, skipping.";
			}
			if(success)
				dcmmeta.branch(slicesBranch).remove(ppair.first); //@todo slices won't be removed if empty
		}
	} else {
		LOG(Debug,warning) << "demuxDcmMetaSlices called, but there is no " << slicesBranch << " branch";
	}
}

boost::posix_time::ptime parseTM( const isis::util::PropertyMap &map, const util::PropertyMap::PropPath &name )
{
	short shift = 0;
	bool ok = true;
	boost::posix_time::time_duration ret;
	std::string time_str=map.getPropertyAs<std::string>(name);
	
	// Insert the ":" -- make it hh:mm:ss.frac
	if ( time_str.at( 2 ) != ':' ) {
		time_str.insert( 2, 1, ':' );
		shift++;
	}
	
	if ( ( time_str.size() > size_t( 4 + shift ) ) && ( time_str.at( 4 + shift ) != ':' ) ) {
		time_str.insert( 4 + shift, 1, ':' );
		shift++;
	}
	
	//Try standard-parser for hh:mm:ss.frac
	try {
		ret = boost::posix_time::duration_from_string( time_str.c_str() );
		ok = not ret.is_not_a_date_time();
	} catch ( std::logic_error e ) {
		ok = false;
	}
	
	if ( ok ) {
		LOG( Debug, verbose_info )<< "Parsed time for " << name << "(" <<  time_str << ")" << " as " << ret;
		return boost::posix_time::ptime( boost::gregorian::date( 1400, 1, 1 ), ret );
		//because TM is defined as time of day we dont have a day here, so we fake one
	} else
		LOG( Runtime, warning ) << "Cannot parse Time string \"" << time_str << "\" in the field \"" << name << "\"";
	
	return boost::posix_time::ptime(boost::posix_time::not_a_date_time);
}


}
}
}
