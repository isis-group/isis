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
#include "DataStorage/fileptr.hpp"

#include "VistaSaParser.hpp"

namespace isis
{

namespace image_io
{

class ImageFormat_VistaSa: public FileFormat
{
private:
	typedef isis::data::ValueArrayReference ( *readerPtr )( isis::data::FilePtr data, size_t offset, size_t size );
	std::map<isis::util::istring, readerPtr> vista2isis;

	data::Chunk makeChunk( data::FilePtr data, data::FilePtr::iterator data_start, const util::PropertyMap &props);
public:
	ImageFormat_VistaSa();
	std::string getName()const {return "Vista standalone";}
	int load ( std::list< isis::data::Chunk >& chunks, const std::string &filename, const isis::util::istring &dialect ) throw ( std::runtime_error & );
	void write( const data::Image &image, const std::string &filename, const util::istring &dialect )  throw( std::runtime_error & );

	bool tainted()const {return false;}//internal plugins are not tainted
	util::istring dialects( const std::string &/*filename*/ )const {return "fsl spm";}

	void sanitize(util::PropertyMap &obj);
	bool isFunctional(const std::list< isis::data::Chunk >& chunks);
	std::list< data::Chunk > transformFunctional(const std::list< data::Chunk >& in_chunks);

protected:
	util::istring suffixes( io_modes /*mode = both */ )const {return ".v";}
};

}
}

#endif // IMAGEFORMAT_VISTA_SA_HPP