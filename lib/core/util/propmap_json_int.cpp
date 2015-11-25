#include "propmap.hpp"
#include <boost/fusion/container/vector.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace isis
{
namespace util
{
API_EXCLUDE_BEGIN;
/// @cond _internal
namespace _internal
{
	struct parser {
		typedef BOOST_TYPEOF( boost::spirit::ascii::space | '\t' | boost::spirit::eol ) skip_type;
		template<typename T> struct rule{typedef boost::spirit::qi::rule<const uint8_t*, T(), skip_type > decl;};
		typedef boost::variant<PropertyValue, PropertyMap> value_cont;
		
		struct add_member {
			const char extra_token;
			add_member( char _extra_token ): extra_token( _extra_token ) {}
			void operator()( const boost::fusion::vector2<std::string, value_cont> &a, rule<PropertyMap>::decl::context_type &context, bool & )const {
				//todo test me
				const PropertyMap::PropPath label = extra_token ? stringToList<PropertyMap::key_type>( a.m0, std::regex(&extra_token) ) :
				PropertyMap::PropPath( a.m0.c_str() );
				PropertyMap &target = context.attributes.car;
				
				if( target.hasBranch( label ) || target.hasProperty( label ) )
					LOG( Runtime, error ) << "There is already an entry " << MSubject(target) << " skipping this one" ;
				else{
					const value_cont &container= a.m1;
					switch( container.which() ) {
						case 1:
							target.touchBranch( label ) = boost::get<PropertyMap>( container );
							break;
						case 0:
							target.touchProperty( label ) = boost::get<PropertyValue>( container );
							break;
					}
					
				}
			}
		};
	};
}
/// @endcond _internal
API_EXCLUDE_END;


ptrdiff_t PropertyMap::readJson( const uint8_t* streamBegin, const uint8_t* streamEnd, char extra_token, std::string list_trees )
{
	using namespace boost::spirit;
	using qi::lit;
	using _internal::parser;
	
	parser::rule<boost::fusion::vector2<std::string, parser::value_cont> >::decl member;
	
	qi::symbols<char const, char const> esc_char;
	esc_char.add("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
	("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
	("\\\'", '\'')("\\\"", '\"');
	
	parser::rule<std::string>::decl string( lexeme['"' >> *(esc_char | "\\x" >> qi::hex | ascii::print - '"')>> '"'], "string" );
	parser::rule<std::string>::decl label( string >> ':', "label" );
	parser::rule<int>::decl integer( int_ >> !lit( '.' ), "integer" ) ; // an integer followed by a '.' is not an integer
	parser::rule<dlist>::decl dlist( lit( '[' ) >> double_ % ',' >> ']', "dlist" );
	parser::rule<ilist>::decl ilist( lit( '[' ) >> integer % ',' >> ']', "ilist" );
	parser::rule<slist>::decl slist( lit( '[' ) >> string  % ',' >> ']', "slist" );
	parser::rule<PropertyValue>::decl value = integer | double_ | string | ilist | dlist | slist;
	parser::rule<PropertyValue>::decl vallist( lit( '[' ) >> value % ',' >> ']', "value_list" );
	
	parser::rule<PropertyMap>::decl object( lit( '{' ) >> ( member[parser::add_member( extra_token )] % ',' || eps ) >> '}', "object" );
	parser::rule<PropertyMap>::decl list_object( lit( '{' ) >> ( ( label >> vallist )[parser::add_member( extra_token )] % ',' || eps ) >> '}', "list_object" );
	
	for(const std::string &label : util::stringToList<std::string>(list_trees,':')){
		member= member.copy() | lexeme['"' >> ascii::string( label ) >> '"'] >> ':' >> list_object;
	}
	
	member= member.copy() | label >> ( value | vallist | object );
	
	const uint8_t* end = streamEnd;
	qi::phrase_parse( streamBegin, end, object[boost::phoenix::ref( *this ) = _1], ascii::space | '\t' | eol );
	return end - streamEnd;
}


}
}