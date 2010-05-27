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

#include "DataStorage/io_interface.h"
#include <dcmtk/config/cfunix.h> //@todo add switch for windows if needed
#include <dcmtk/dcmdata/dcfilefo.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace isis
{
namespace image_io
{

class ImageFormat_Dicom: public FileFormat
{
	static void parseAS( DcmElement *elem, const std::string &name, util::PropMap &map );
	static void parseDA( DcmElement *elem, const std::string &name, util::PropMap &map );
	static void parseTM( DcmElement *elem, const std::string &name, util::PropMap &map );
	static boost::posix_time::ptime genTimeStamp( const boost::gregorian::date &date, const boost::posix_time::ptime &time );
	template<typename BASE, typename DST> static DST endian( const BASE *b ) {
		DST ret = 0;
#if __BYTE_ORDER == __LITTLE_ENDIAN

		for ( short i = 0; i < ( short )sizeof( DST ); i++ )
#elif __BYTE_ORDER == __BIG_ENDIAN
		for ( short i = ( short )sizeof( DST ) - 1; i >= 0; i-- )
#else
#error "Sorry your endianess is not supported. What the heck are you comiling on ??"
#endif
			ret += b[i] << i * 8;

		return ret;
	}
	static size_t parseCSAEntry( Uint8 *at, isis::util::PropMap &map );
	static bool parseCSAValue( const std::string &val, const std::string &name, const char *const vr, isis::util::PropMap &map );
	static bool parseCSAValueList( const isis::util::slist &val, const std::string &name, const char *const vr, isis::util::PropMap &map );
	void readMosaic( const data::Chunk &source, data::ChunkList &dest );
public:
	static const char dicomTagTreeName[];
	static const char unknownTagName[];
	static void parseCSA( DcmElement *elem, isis::util::PropMap &map );
	static void parseScalar( DcmElement *elem, const std::string &name, isis::util::PropMap &map );
	static void parseVector( DcmElement *elem, const std::string &name, isis::util::PropMap &map );
	static void parseList( DcmElement *elem, const std::string &name, isis::util::PropMap &map );
	static void dcmObject2PropMap( DcmObject *master_obj, util::PropMap &map );
	static void sanitise( util::PropMap &object, string dialect );
	std::string suffixes();
	std::string name();

	int load( data::ChunkList &chunks, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & );
	void write( const data::Image &image, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & );

	bool tainted();
};
}
}

#endif // IMAGEFORMAT_DICOM_HPP
