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


namespace isis
{
namespace image_io
{
namespace _internal
{


VistaSaParser::VistaSaParser ( data::FilePtr fPtr )
	: m_filePtr ( fPtr ) {}

VistaSaParser::HeaderObjectListType VistaSaParser::getHeaderObjectMap ()
{
	m_vheader = parseHeader();
}

VistaSaParser::HeaderListType VistaSaParser::parseHeader ()
{

	using namespace boost::spirit;
	std::list< uint8_t > instream;
	data::FilePtr::iterator begin = m_filePtr.begin();

	bool parsed = qi::parse<data::FilePtr::iterator> ( begin, m_filePtr.end(),
				  qi::lit ( "V-data 2" )
				  >> ascii::char_
				  >> '{'
				  >> *ascii::char_
				  >> '}',
				  instream );
	BOOST_FOREACH ( std::list<uint8_t>::const_reference g, instream ) {
		std::cout << g ;
	}
	std::cout << "end" << std::endl;

}


}
}
}
