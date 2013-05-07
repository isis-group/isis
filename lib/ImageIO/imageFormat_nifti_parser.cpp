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


#define BOOST_SPIRIT_DEBUG_PRINT_SOME 50
#define BOOST_SPIRIT_DEBUG_INDENT 5

#include <boost/foreach.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include "imageFormat_nifti_parser.hpp"

namespace isis
{
namespace image_io
{
namespace _internal
{
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;
typedef BOOST_TYPEOF( ascii::space | '\t' | boost::spirit::eol ) SKIP_TYPE;


// typedef fusion::vector2<std::string, std::string> s_entry;
typedef data::ValueArray< uint8_t >::iterator ch_iterator;
typedef qi::rule<ch_iterator, isis::util::PropertyMap(), SKIP_TYPE>::context_type PropertyMapContext;
typedef boost::variant<util::PropertyValue, util::PropertyMap> value_cont;


void add_member( const fusion::vector2<std::string, value_cont> &a, PropertyMapContext &context )
{
	const value_cont &container = a.m1;
	const util::PropertyMap::PropPath label = a.m0.c_str();
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

struct flattener { // simply insert subarrays into the array above
	template<typename T, typename CONTEXT> void operator()( const std::list<T> &a, CONTEXT &context, bool & )const {
		std::list<T> &cont = context.attributes.car;
		cont.insert( cont.end(), a.begin(), a.end() );
	}
};

template<typename T> struct read {typedef qi::rule<ch_iterator, T(), SKIP_TYPE> rule;};

bool parse_json( isis::data::ValueArray< uint8_t > stream, isis::util::PropertyMap &DcmStack )
{
	using qi::lit;
	using namespace boost::spirit;

	int version;
	SKIP_TYPE skipper = ascii::space | '\t' | eol;

	read<value_cont>::rule value;
	read<std::string>::rule string( lexeme['"' >> *( ascii::print - '"' ) >> '"'], "string" ), label( string >> ':', "label" );
	read<fusion::vector2<std::string, value_cont> >::rule member( label >> value, "member" );
	read<isis::util::PropertyMap>::rule object( lit( '{' ) >> member[add_member] % ',' >> '}', "object" );
	read<util::dlist>::rule dlist( lit( '[' ) >> ( ( double_[phoenix::push_back( _val, _1 )] | dlist[flattener()] ) % ',' ) >> ']', "dlist" );
	read<util::slist>::rule slist( lit( '[' ) >> ( ( string[phoenix::push_back( _val, _1 )] | slist[flattener()] ) % ',' ) >> ']', "slist" );

	value = string | double_ | slist | dlist |  object;
	value.name( "value" );

	/*  qi::debug(member);
	    qi::debug(object);
	    qi::debug(dlist);
	    qi::debug(slist);*/

	data::ValueArray< uint8_t >::iterator begin = stream.begin(), end = stream.end();
	bool erg = phrase_parse( begin, end, object[phoenix::ref( DcmStack )=_1], skipper );
	return end == stream.end();

#undef RULE

}

}
}
}
