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


#ifndef IMAGEFORMAT_NIFTI_PARSER_HPP
#define IMAGEFORMAT_NIFTI_PARSER_HPP


#include "DataStorage/valuearray.hpp"
#include "DataStorage/chunk.hpp"

namespace isis
{
namespace image_io
{
namespace _internal
{

class DCMStack: public util::PropertyMap
{
public:
	DCMStack() {}
	DCMStack( const util::PropertyMap &src );
	void writeJson( std::ostream &out );
	bool readJson( data::ValueArray< uint8_t > stream, char extra_token = 0 );
	void translateToISIS( data::Chunk& orig );
private:
	static void writeSubtree( const PropertyMap& src, std::ostream &out );
	void decodeMosaic();
};

}
}
}

#endif // IMAGEFORMAT_NIFTI_PARSER_HPP
