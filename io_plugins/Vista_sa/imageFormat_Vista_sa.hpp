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

#include <isis/data/io_interface.h>
#include <isis/data/fileptr.hpp>

namespace isis
{

namespace image_io
{

class ImageFormat_VistaSa: public FileFormat
{
private:
	static const std::locale vista_locale;
	class vista_date_facet : public std::time_get< std::stringstream::char_type >{
        virtual dateorder do_date_order() const{return std::time_base::dmy;}
	};
	template<typename STORED> static util::PropertyValue& setPropFormated(util::PropertyMap::PropPath name,const STORED &prop, util::PropertyMap &obj){
		std::stringstream ss;
		ss.imbue(vista_locale);
		ss << prop;
		return obj.touchProperty(name)=ss.str();
	}
	
public:
	std::string getName()const {return "Vista standalone";}
	std::list<data::Chunk> load( data::ByteArray source, std::list<util::istring> formatstack, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback )throw ( std::runtime_error & ) override;
	void write( const data::Image &image, const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback )throw( std::runtime_error & ); // not used
	void write( const std::list<data::Image> &images, const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback )throw( std::runtime_error & );
	
	bool tainted()const {return false;}//internal plugins are not tainted
	util::istring dialects( const std::string &/*filename*/ )const {return "";}
	static void sanitize( util::PropertyMap &obj );
	static void unsanitize( util::PropertyMap &obj );


protected:
	util::istring suffixes( io_modes /*mode = both */ )const {return ".v";}
};

}
}

#endif // IMAGEFORMAT_VISTA_SA_HPP
