/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

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

#include "VistaSaParser.hpp"
#include <boost/foreach.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

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

typedef fusion::vector2<std::string, std::string> s_entry;

template<typename Iterator> void addEntry(
	const s_entry &a,
	typename qi::rule<Iterator, isis::util::PropertyMap(), ascii::space_type>::context_type &context )
{
	isis::util::PropertyMap &pmap = context.attributes.car;
	isis::util::istring name = a.m0.c_str();

	if( pmap.hasBranch( name ) ) {
		LOG( Runtime, warning ) << "There is allready a branch " << name << " skipping " << name << "=" << a.m1;
	} else if( pmap.hasProperty( name ) ) {
		if( pmap.propertyValue( name ) == a.m1 )
			LOG( Runtime, info ) << "Skipping duplicate " << std::make_pair( name, a.m1 );
		else
			LOG( Runtime, warning ) << "There is already an entry " << std::make_pair( name, pmap.propertyValue( name ) ) << " skipping " << std::make_pair( name, a.m1 );
	} else
		context.attributes.car.setPropertyAs( name, a.m1 );
}

template<typename Iterator> void addBlock(
	const fusion::vector3<std::string, boost::optional<std::string >, isis::util::PropertyMap> &a,
	typename qi::rule<Iterator, isis::util::PropertyMap(), ascii::space_type>::context_type &context
)
{
	isis::util::PropertyMap &pmap = context.attributes.car;
	isis::util::istring name = a.m0.c_str();

	if( pmap.hasBranch( name ) || pmap.hasProperty( name ) ) {
		LOG( Runtime, error ) << "There is already an entry " << name << " skipping this one" ;
	} else
		pmap.branch( name ) = a.m2;
}

template<typename Iterator> void addHist(
	const std::vector<s_entry> &a,
	typename qi::rule<Iterator, isis::util::PropertyMap(), ascii::space_type>::context_type &context
)
{
	isis::util::PropertyMap &pmap = context.attributes.car;
	isis::util::PropertyValue &hist = pmap.propertyValue( "history" );

	if( hist.isEmpty() )
		hist = isis::util::slist();

	if( hist.is<isis::util::slist>() ) {
		BOOST_FOREACH( const s_entry & ref, a )
		hist.castTo<isis::util::slist>().push_back( ref.m0 + ":" + ref.m1 );
	}
}

template<typename Iterator> void addChunk(
	const isis::util::PropertyMap &a,
	typename qi::rule<Iterator, isis::util::PropertyMap(), ascii::space_type>::context_type &context
)
{
	context.attributes.car = a;
}


bool parse_vista( data::ValueArray< uint8_t >::iterator &first, data::ValueArray< uint8_t >::iterator last, util::PropertyMap &vista_map, std::list< util::PropertyMap >& ch_list )
{
	using qi::_1;
	using qi::lit;
	using qi::alnum;
	using namespace boost::spirit;

	namespace phoenix = boost::phoenix;
	typedef BOOST_TYPEOF( ascii::space | '\t' | '\n' ) SKIP_TYPE;
	typedef data::ValueArray< uint8_t >::iterator Iterator;

	int version;
	SKIP_TYPE skipper = ascii::space | '\t' | '\n';

	qi::rule<Iterator, int(), SKIP_TYPE> magic = "V-data" >> int_;
	qi::rule<Iterator, std::string(), SKIP_TYPE> word = lexeme[+( ascii::char_ - '"' - '{'-'}'-skipper )];
	qi::rule<Iterator, std::string(), SKIP_TYPE> quoted_string = lexeme['"' >> *( ascii::char_ - '"' ) >> '"'];
	qi::rule<Iterator, std::string(), SKIP_TYPE> label = lexeme[+( ascii::alnum|'_'|'-' )] >> ':';
	qi::rule<Iterator, s_entry(), SKIP_TYPE> entry = label >> ( quoted_string | word ) >> !lit( '{' );

	qi::rule<Iterator, isis::util::PropertyMap(), SKIP_TYPE> block, chunk;
	qi::rule<Iterator, std::vector<s_entry>(), SKIP_TYPE> hist_block = lit( "history" ) >> ':' >> '{' >> *entry >> '}';
	qi::rule<Iterator, fusion::vector3<std::string, boost::optional<std::string >, isis::util::PropertyMap>(), SKIP_TYPE> named_block = label >> -word >> block;

	chunk = lit( "image" ) >> ':' >> "image" >> block[addChunk<Iterator>]; //@todo cannot use automatic apply, because PropertyMap is uncleanly derived from std::map
	block = ( '{' >> *( entry[addEntry<Iterator>] | hist_block[addHist<Iterator>] | chunk[phoenix::push_back( phoenix::ref( ch_list ), _1 )] | named_block[addBlock<Iterator>] ) >> '}' );

	qi::rule<Iterator, SKIP_TYPE> vista = magic[phoenix::ref( version ) =_1] >> block[phoenix::ref( vista_map )=_1];

	block.name( "block" );
	named_block.name( "named_block" );
	entry.name( "entry" );
	label.name( "label" );
	magic.name( "magic" );
	quoted_string.name( "quoted" );
	word.name( "word" );
	chunk.name( "chunk" );

	//     qi::debug(block);
	//     qi::debug(magic);
	//     qi::debug(entry);
	//     qi::debug(label);
	//     qi::debug(quoted_string);
	//     qi::debug(word);
	//     qi::debug(named_block);
	//     qi::debug(chunk);

	return phrase_parse( first, last, vista, skipper );
}

}
}
}
