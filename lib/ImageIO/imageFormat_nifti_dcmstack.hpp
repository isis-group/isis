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
#include <boost/variant.hpp>

namespace isis
{
namespace image_io
{
namespace _internal
{

extern const char dcmmeta_root[];
extern const char dcmmeta_global[];
extern const char dcmmeta_perslice_data[];
extern const char dcmmeta_const_data[];

// bool parse_json( data::ValueArray< uint8_t > stream, util::PropertyMap &json_map, char extra_token = 0 );

// void demuxDcmMetaSlices( std::list< data::Chunk >& chunks, util::PropertyMap& dcmmeta );

class JsonMap:public util::PropertyMap{
public:
	typedef boost::variant<util::PropertyValue, JsonMap, std::list<util::PropertyValue> > value_cont;
	
	JsonMap(){}
	JsonMap(const util::PropertyMap &src);
	void insertObject( const PropPath& label, const value_cont& container );
	void WriteJson( std::ostream& out );
	bool ReadJson( isis::data::ValueArray< uint8_t > stream, char extra_token = 0 );
private:
	static void WriteSubtree( const std::map< isis::util::istring, isis::util::_internal::treeNode >& src, std::ostream& out );
};

//parse strings formated as dicom TM
boost::posix_time::ptime parseTM( const JsonMap& map, const JsonMap::PropPath& name );


}
}
}

#endif // IMAGEFORMAT_NIFTI_PARSER_HPP
