/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef IMAGEFORMAT_DICOM_HPP
#define IMAGEFORMAT_DICOM_HPP

#include <DataStorage/io_interface.h>
#include <dcmtk/config/cfunix.h> //@todo add switch for windows if needed
#include <dcmtk/dcmdata/dcfilefo.h>
#include "common.hpp"

namespace isis{ namespace image_io{
	
class ImageFormat_Dicom: public FileFormat{
	void dcmObject2PropMap(DcmObject* master_obj,util::PropMap &map);
	void parseAS(DcmElement* elem,const std::string &name,util::PropMap &map);
	void parseDA(DcmElement* elem,const std::string &name,util::PropMap &map);
	void parseTM(DcmElement* elem,const std::string &name,util::PropMap &map);
public:
	std::string suffixes();
	std::string name();

	data::ChunkList load( std::string filename, std::string dialect );
	bool write(const data::Image &image,std::string filename,std::string dialect );

	bool tainted();
	size_t maxDim();
};
}}

#endif // IMAGEFORMAT_DICOM_HPP
