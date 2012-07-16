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

#define HAVE_CONFIG_H // this is needed for autoconf configured dcmtk (e.g. the debian package)

#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdict.h>

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace isis
{
namespace image_io
{

class ImageFormat_Dicom: public FileFormat
{
	static void parseAS( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map );
	static void parseDA( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map );
	static void parseTM( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map );
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
	static size_t parseCSAEntry( Uint8 *at, isis::util::PropertyMap &map, const util::istring &dialect );
	static bool parseCSAValue( const std::string &val, const util::PropertyMap::PropPath &name, const util::istring &vr, isis::util::PropertyMap &map );
	static bool parseCSAValueList( const isis::util::slist &val, const util::PropertyMap::PropPath &name, const util::istring &vr, isis::util::PropertyMap &map );
	static data::Chunk readMosaic( data::Chunk source );
	std::map<DcmTagKey, util::PropertyMap::PropPath> dictionary;
protected:
	util::istring suffixes( io_modes modes = both )const;
	util::PropertyMap::PropPath tag2Name( const DcmTagKey &tag ) const;
public:
	ImageFormat_Dicom();
	void addDicomDict( DcmDataDictionary &dict );
	static const char dicomTagTreeName[];
	static const char unknownTagName[];
	static void parseCSA( DcmElement *elem, isis::util::PropertyMap &map, const util::istring &dialect );
	static void parseScalar( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map );
	static void parseList( DcmElement *elem, const util::PropertyMap::PropPath &name, isis::util::PropertyMap &map );
	void dcmObject2PropMap( DcmObject *master_obj, isis::util::PropertyMap &map, const util::istring &dialect )const;
	static void sanitise( util::PropertyMap &object, util::istring dialect );
	std::string getName()const;
	util::istring dialects( const std::string &filename )const;

	int load( std::list<data::Chunk> &chunks, const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> progress ) throw( std::runtime_error & );
	void write( const data::Image &image,     const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> progress ) throw( std::runtime_error & );

	bool tainted()const;
};
}
}

#endif // IMAGEFORMAT_DICOM_HPP
