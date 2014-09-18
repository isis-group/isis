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

#ifndef VISTASAPARSER_HPP
#define VISTASAPARSER_HPP

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>

#include "DataStorage/fileptr.hpp"

namespace isis
{
namespace image_io
{
namespace _internal
{

namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;


struct VistaHeader

{

};


template<typename Iterator>
struct vista_header_grammer : qi::grammar<Iterator, VistaHeader(), ascii::space_type > {

	vista_header_grammer() : vista_header_grammer::base_type ( vista_header ) {
	using qi::lit;
	using qi::lexeme;
	using ascii::char_;
	using ascii::string;
	using namespace qi::labels;

	using phoenix::at_c;
	using phoenix::push_back;

	start_tag = lit ( "V-data 2 {" );

}
qi::rule<Iterator, VistaHeader(), ascii::space_type > vista_header;
qi::rule<Iterator, std::string, ascii::space_type> start_tag;
qi::rule<Iterator, void ( std::string ), ascii::space_type> end_tag;
};






struct VistaObject {
	data::ValueArrayReference data;
};

class VistaSaParser
{
public:

	VistaSaParser ( data::FilePtr fPtr );
	typedef std::shared_ptr< VistaHeader > HeaderType;
	typedef std::list< HeaderType > HeaderListType;
	typedef std::list< std::pair< HeaderType, VistaObject > > HeaderObjectListType;

	HeaderObjectListType getHeaderObjectMap();


private:
	HeaderListType parseHeader( );

	data::FilePtr m_filePtr;
	HeaderListType m_vheader;




};


}


}
}

#endif // VISTASAPARSER_HPP
