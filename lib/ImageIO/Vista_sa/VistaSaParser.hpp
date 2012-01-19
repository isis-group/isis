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

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "DataStorage/fileptr.hpp"

namespace isis
{
namespace image_io
{
namespace _internal
{


struct VistaHeader
{

};

struct VistaObject
{
	data::ValuePtrReference data;
};

class VistaSaParser : boost::spirit::qi::grammar<data::FilePtr::iterator, VistaHeader(), boost::spirit::ascii::space_type >
{
public:

	VistaHeader ( data::FilePtr fPtr );
	typedef boost::shared_ptr< VistaHeader > HeaderType;
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
