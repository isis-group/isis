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
#ifndef IMAGEFORMAT_VISTA_SA_HPP
#define IMAGEFORMAT_VISTA_SA_HPP

#include <DataStorage/io_interface.h>

#include "VistaSaParser.hpp"

namespace isis
{

namespace image_io
{

class ImageFormat_VistaSa: public FileFormat
{

public:
	ImageFormat_VistaSa();
	std::string getName()const;
	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const util::istring &/*dialect*/ )  throw( std::runtime_error & );
	void write( const data::Image &image, const std::string &filename, const util::istring &dialect )  throw( std::runtime_error & );
	bool tainted()const {return false;}//internal plugins are not tainted
	util::istring dialects( const std::string &/*filename*/ )const {return std::string( "fsl spm" );}

protected:
	util::istring suffixes( io_modes mode = both )const;

	boost::shared_ptr< _internal::VistaHeader> m_vheader;

};

}
}

#endif // IMAGEFORMAT_VISTA_SA_HPP