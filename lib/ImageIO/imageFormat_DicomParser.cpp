#include "imageFormat_Dicom.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <DataStorage/common.hpp>
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmdata/dcdicent.h>

namespace isis
{
namespace image_io
{

namespace _internal
{
template<typename ST, typename DT> bool try_cast( const ST &source, DT &dest )
{
	bool ret = true;

	try {
		dest = boost::lexical_cast<DT>( source );
	} catch ( boost::bad_lexical_cast e ) {
		ret = false;
	}

	return ret;
}

template<typename T> std::list<T> dcmtkListString2list( DcmElement *elem )
{
	OFString buff;
	elem->getOFStringArray( buff );
	return util::stringToList<T>( std::string( buff.c_str() ), '\\' );
}

template <typename S, typename V> void arrayToVecPropImp( S *array, util::PropertyMap &dest, const util::PropertyMap::PropPath &name, size_t len )
{
	V vector;
	vector.copyFrom( array, array + len );
	dest.propertyValue( name ) = vector; //if Float32 is float its fine, if not we will get an linker error here
}
template <typename S> void arrayToVecProp( S *array, util::PropertyMap &dest, const util::PropertyMap::PropPath &name, size_t len )
{
	if( len <= 3 )arrayToVecPropImp<S, util::vector3<S> >( array, dest, name, len );
	else arrayToVecPropImp<S, util::vector4<S> >( array, dest, name, len );
}

template<typename T> bool noLatin( const T &t ) {return t >= 127;}
}


/**
 * Parses the Age String
 * A string of characters with one of the following formats -- nnnD, nnnW, nnnM, nnnY;
 * where nnn shall contain the number of days for D, weeks for W, months for M, or years for Y.
 * Example - "018M" would represent an age of 18 months.
 */
void ImageFormat_Dicom::parseAS( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map )
{
	uint16_t duration = 0;
	OFString buff;
	elem->getOFString( buff, 0 );
	static boost::numeric::converter <
	uint16_t, double,
			boost::numeric::conversion_traits<uint16_t, double>,
			boost::numeric::def_overflow_handler,
			boost::numeric::RoundEven<double>
			> double2uint16;

	if ( _internal::try_cast( buff.substr( 0, buff.find_last_of( "0123456789" ) + 1 ), duration ) ) {
		switch ( buff.at( buff.size() - 1 ) ) {
		case 'D':
		case 'd':
			break;
		case 'W':
		case 'w':
			duration *= 7;
			break;
		case 'M':
		case 'm':
			duration = double2uint16( 30.436875 * duration ); // year/12
			break;
		case 'Y':
		case 'y':
			duration = double2uint16( 365.2425 * duration ); //mean length of a year
			break;
		default:
			LOG( Runtime, warning )
					<< "Missing age-type-letter, assuming days";
		}

		map.propertyValue( name ) = duration;
		LOG( Debug, verbose_info )
				<< "Parsed age for " << name << "(" <<  buff << ")"
				<< " as " << duration << " days";
	} else
		LOG( Runtime, warning )
				<< "Cannot parse age string \"" << buff << "\" in the field \"" << name << "\"";
}

/**
 * Parses the Date string
 * A string of characters of the format yyyymmdd;
 * where yyyy shall contain year, mm shall contain the month, and dd shall contain the day.
 * This conforms to the ANSI HISPP MSDS Date common data type.
 * Example - "19930822" would represent August 22, 1993.
 * For reasons of backward compatibility with versions of this standard prior to V3.0,
 * it is recommended that implementations also support a string of characters of the format yyyy.mm.dd for this VR.
 */
void ImageFormat_Dicom::parseDA( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map )
{
	//@todo if we drop support for old yyyy.mm.dd this would be much easier
	static const boost::regex reg( "^([[:digit:]]{4})\\.?([[:digit:]]{2})\\.?([[:digit:]]{2})$" );
	boost::cmatch results;
	OFString buff;
	elem->getOFString( buff, 0 );

	if ( boost::regex_match( buff.c_str(), results, reg ) ) {
		const boost::gregorian::date date(
			boost::lexical_cast<int16_t>( results.str( 1 ) ), //year
			boost::lexical_cast<int16_t>( results.str( 2 ) ), //month
			boost::lexical_cast<int16_t>( results.str( 3 ) ) //day of month
		);
		LOG( Debug, verbose_info )
				<< "Parsed date for " << name << "(" <<  buff << ")" << " as " << date;
		map.propertyValue( name ) = date;
	} else
		LOG( Runtime, warning )
				<< "Cannot parse Date string \"" << buff << "\" in the field \"" << name << "\"";
}

/**
 * Parses the Time string
 * A string of characters of the format hhmmss.frac; where hh contains hours (range "00" - "23"),
 * mm contains minutes (range "00" - "59"), ss contains seconds (range "00" - "59"), and frac contains
 * a fractional part of a second as small as 1 millionth of a second (range "000000" - "999999").
 * A 24 hour clock is assumed. Midnight can be represented by only "0000" since "2400" would violate the
 * hour range. The string may be padded with trailing spaces. Leading and embedded spaces are not allowed.
 * One or more of the components mm, ss, or frac may be unspecified as long as every component to the right
 * of an unspecified component is also unspecified. If frac is unspecified the preceding "." may not be included.
 * Frac shall be held to six decimal places or less to ensure its format conforms to the ANSI HISPP MSDS Time
 * common data type.
 * Examples:
 * - "070907.0705" represents a time of 7 hours, 9 minutes and 7.0705 seconds.
 * - "1010" represents a time of 10 hours, and 10 minutes.
 * - "021" is an invalid value.
 *
 * For reasons of backward compatibility with versions of this standard prior to V3.0, it is
 * recommended that implementations also support a string of characters of the format hh:mm:ss.frac for this VR.
 */
void ImageFormat_Dicom::parseTM( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map )
{
	short shift = 0;
	OFString buff;
	bool ok = true;
	boost::posix_time::time_duration time;
	elem->getOFString( buff, 0 );

	//Try iso-parser (hhmmss.frac)
	try {
		time = boost::date_time::parse_undelimited_time_duration<boost::posix_time::time_duration>(buff.c_str());
		ok = not time.is_not_a_date_time();
	} catch ( std::logic_error e ) {
		ok = false;
	}

	if ( ok ) {
		LOG( Debug, verbose_info ) << "Parsed time for " << name << "(" <<  buff << ")" << " as " << time;
		map.propertyValue( name ) = boost::posix_time::ptime( boost::gregorian::date( 1400, 1, 1 ), time );
		//although TM is defined as time of day we dont have a day here, so we fake one
	} else
		LOG( Runtime, warning ) << "Cannot parse Time string \"" << buff << "\" in the field \"" << name << "\"";
}

void ImageFormat_Dicom::parseScalar( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map )
{
	OFString buff;

	switch ( elem->getVR() ) {
	case EVR_AS: { // age string (nnnD, nnnW, nnnM, nnnY)
		parseAS( elem, name, map );
	}
	break;
	case EVR_DA: {
		parseDA( elem, name, map );
	}
	break;
	case EVR_TM: {
		parseTM( elem, name, map );
	}
	break;
	case EVR_FL: {
		Float32 buff;
		elem->getFloat32( buff );
		map.setPropertyAs<float>( name, buff ); //if Float32 is float its fine, if not we will get an compiler error here
	}
	break;
	case EVR_FD: {
		Float64 buff;
		elem->getFloat64( buff );
		map.setPropertyAs<double>( name, buff ); //if Float64 is double its fine, if not we will get an compiler error here
	}
	break;
	case EVR_DS: { //Decimal String (can be floating point)
		elem->getOFString( buff, 0 );
		map.setPropertyAs<double>( name, boost::lexical_cast<double>( buff ) );
	}
	break;
	case EVR_SL: { //signed long
		Sint32 buff;
		elem->getSint32( buff );
		map.setPropertyAs<int32_t>( name, buff ); //seems like Sint32 is not allways int32_t, so enforce it
	}
	break;
	case EVR_SS: { //signed short
		Sint16 buff;
		elem->getSint16( buff );
		map.setPropertyAs<int16_t>( name, buff );
	}
	break;
	case EVR_UL: { //unsigned long
		Uint32 buff;
		elem->getUint32( buff );
		map.setPropertyAs<uint32_t>( name, buff );
	}
	break;
	case EVR_US: { //unsigned short
		Uint16 buff;
		elem->getUint16( buff );
		map.setPropertyAs<uint16_t>( name, buff );
	}
	break;
	case EVR_IS: { //integer string
		elem->getOFString( buff, 0 );
		map.setPropertyAs<int32_t>( name, boost::lexical_cast<int32_t>( buff ) );
	}
	break;
	case EVR_AE: //Application Entity (string)
	case EVR_CS: // Code String (string)
	case EVR_LT: //long text
	case EVR_SH: //short string
	case EVR_LO: //long string
	case EVR_ST: //short text
	case EVR_UT: //Unlimited Text
	case EVR_UI: //Unique Identifier [0-9\.]
	case EVR_PN: { //Person Name
		elem->getOFString( buff, 0 );
		map.setPropertyAs<std::string>( name, boost::lexical_cast<std::string>( buff ) );
	}
	break;
	case EVR_UN: { //Unknown, see http://www.dabsoft.ch/dicom/5/6.2.2/
		//@todo do a better sanity check
		Uint8 *buff;
		elem->getUint8Array( buff ); // get the raw data
		Uint32 len = elem->getLength();
		const size_t nonLat = std::count_if( buff, buff + len, _internal::noLatin<Uint8> );

		if( nonLat ) { // if its not "just text" encode it as base256
			LOG( Runtime, info ) << "Using " << len << " bytes from " << name << "("
								 << const_cast<DcmTag &>( elem->getTag() ).getVRName() << ") as base256 because there are "
								 << nonLat << " non latin characters in it";
			std::stringstream o;
			std::copy( buff, buff + len, std::ostream_iterator<Uint16>( o << std::hex ) );
			map.setPropertyAs<std::string>( name, o.str() ); //stuff it into a string
		} else
			map.setPropertyAs<std::string>( name, std::string( ( char * )buff, len ) ); //stuff it into a string
	}
	break;
	default: {
		elem->getOFString( buff, 0 );
		LOG( Runtime, notice ) << "Implement me " << name << "("
							   << const_cast<DcmTag &>( elem->getTag() ).getVRName() << "):"
							   << buff;
	}
	break;
	}
}

void ImageFormat_Dicom::parseList( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map )
{
	OFString buff;
	size_t len = elem->getVM();

	switch ( elem->getVR() ) {
	case EVR_FL: {
		Float32 *buff;
		elem->getFloat32Array( buff );
		map.propertyValue( name ) = util::dlist( buff, buff + len );
	}
	break;
	case EVR_FD: {
		Float64 *buff;
		elem->getFloat64Array( buff );
		map.propertyValue( name ) = util::dlist( buff, buff + len );
	}
	break;
	case EVR_IS: {
		map.propertyValue( name ) = _internal::dcmtkListString2list<int>( elem );
	}
	break;
	case EVR_SL: {
		Sint32 *buff;
		elem->getSint32Array( buff );
		map.propertyValue( name ) = util::ilist( buff, buff + len );
	}
	break;
	case EVR_US: {
		Uint16 *buff;
		elem->getUint16Array( buff );
		map.propertyValue( name ) = util::ilist( buff, buff + len );
	}
	break;
	case EVR_CS: // Code String (string)
	case EVR_SH: //short string
	case EVR_ST: { //short text
		map.propertyValue( name ) = _internal::dcmtkListString2list<std::string>( elem );
	}
	break;
	case EVR_DS: {
		map.propertyValue( name ) = _internal::dcmtkListString2list<double>( elem );
	}
	break;
	case EVR_AS:
	case EVR_DA:
	case EVR_TM:
	case EVR_SS:
	case EVR_UL:
	case EVR_AE: //Application Entity (string)
	case EVR_LT: //long text
	case EVR_LO: //long string
	case EVR_UT: //Unlimited Text
	case EVR_UI: //Unique Identifier [0-9\.]
	case EVR_PN:
	default: {
		elem->getOFStringArray( buff );
		LOG( Runtime, notice ) << "Implement me "
							   << name << "("
							   << const_cast<DcmTag &>( elem->getTag() ).getVRName() << "):"
							   << buff;
	}
	break;
	}

	LOG( Debug, verbose_info ) << "Parsed the list " << name << " as " << map.propertyValue( name );
}

void ImageFormat_Dicom::parseCSA( DcmElement *elem, util::PropertyMap &map, const util::istring &dialect )
{
	Uint8 *array;
	elem->getUint8Array( array );
	const size_t len = elem->getLength();

	for ( std::string::size_type pos = 0x10; pos < ( len - sizeof( Sint32 ) ); ) {
		pos += parseCSAEntry( array + pos, map, dialect );
	}
}
size_t ImageFormat_Dicom::parseCSAEntry( Uint8 *at, util::PropertyMap &map, const util::istring &dialect )
{
	size_t pos = 0;
	const char *const name = ( char * )at + pos;
	pos += 0x40;
	assert( name[0] );
	/*Sint32 &vm=*((Sint32*)array+pos);*/
	pos += sizeof( Sint32 );
	const char *const vr = ( char * )at + pos;
	pos += 0x4;
	/*Sint32 syngodt=endian<Uint8,Uint32>(array+pos);*/
	pos += sizeof( Sint32 );
	const Sint32 nitems = endian<Uint8, Uint32>( at + pos );
	pos += sizeof( Sint32 );

	if ( nitems ) {
		pos += sizeof( Sint32 ); //77
		util::slist ret;

		for ( unsigned short n = 0; n < nitems; n++ ) {
			Sint32 len = endian<Uint8, Uint32>( at + pos );
			pos += sizeof( Sint32 );//the length of this element
			pos += 3 * sizeof( Sint32 ); //whatever

			if ( !len )continue;

			if( (
					std::string( "MrPhoenixProtocol" ) != name  && std::string( "MrEvaProtocol" ) != name && std::string( "MrProtocol" ) != name
				) || dialect == "withExtProtocols" ) {
				std::string insert( ( char * )at + pos );
				const std::string whitespaces( " \t\f\v\n\r" );
				const std::string::size_type start = insert.find_first_not_of( whitespaces );

				if ( insert.empty() || start == std::string::npos ) {
					LOG( Runtime, verbose_info ) << "Skipping empty string for CSA entry " << name;
				} else {
					const std::string::size_type end = insert.find_last_not_of( whitespaces ); //strip spaces

					if( end == std::string::npos )
						ret.push_back( insert.substr( start, insert.size() - start ) ); //store the text if there is some
					else
						ret.push_back( insert.substr( start, end + 1 - start ) );//store the text if there is some
				}
			} else {
				LOG( Runtime, info ) << "Skipping " << name << " as its not requested by the dialect (use dialect \"withExtProtocols\" to get it)";
			}

			pos += (
					   ( len + sizeof( Sint32 ) - 1 ) / sizeof( Sint32 )
				   ) *
				   sizeof( Sint32 );//increment pos by len aligned to sizeof(Sint32)*/
		}

		try {
			util::PropertyMap::PropPath path;
			path.push_back( name );

			if ( ret.size() == 1 ) {
				if ( parseCSAValue( ret.front(), path , vr, map ) ) {
					LOG( Debug, verbose_info ) << "Found scalar entry " << path << ":" << map.propertyValue( path ) << " in CSA header";
				}
			} else if ( ret.size() > 1 ) {
				if ( parseCSAValueList( ret, path, vr, map ) ) {
					LOG( Debug, verbose_info ) << "Found list entry " << path << ":" << map.propertyValue( path ) << " in CSA header";
				}
			}
		} catch ( boost::bad_lexical_cast e ) {
			LOG( Runtime, warning ) << "Failed to parse CSA entry " << std::make_pair( name, ret ) << " as " << vr << " (" << e.what() << ")";
		}
	} else {
		LOG( Debug, verbose_info ) << "Skipping empty CSA entry " << name;
		pos += sizeof( Sint32 );
	}

	return pos;
}

bool ImageFormat_Dicom::parseCSAValue( const std::string &val, const util::PropertyMap::PropPath &name, const util::istring &vr, util::PropertyMap &map )
{
	if ( vr == "IS" or vr == "SL" ) {
		map.propertyValue( name ) = boost::lexical_cast<int32_t>( val );
	} else if ( vr == "UL" ) {
		map.propertyValue( name ) = boost::lexical_cast<uint32_t>( val );
	} else if ( vr == "CS" or vr == "LO" or vr == "SH" or vr == "UN" or vr == "ST" ) {
		map.propertyValue( name ) = val;
	} else if ( vr == "DS" or vr == "FD" ) {
		map.propertyValue( name ) = boost::lexical_cast<double>( val );
	} else if ( vr == "US" ) {
		map.propertyValue( name ) = boost::lexical_cast<uint16_t>( val );
	} else if ( vr == "SS" ) {
		map.propertyValue( name ) = boost::lexical_cast<int16_t>( val );
	} else {
		LOG( Runtime, error ) << "Dont know how to parse CSA entry " << std::make_pair( name, val ) << " type is " << util::MSubject( vr );
		return false;
	}

	return true;
}
bool ImageFormat_Dicom::parseCSAValueList( const util::slist &val, const util::PropertyMap::PropPath &name, const util::istring &vr, util::PropertyMap &map )
{
	if ( vr == "IS" or vr == "SL" or vr == "US" or vr == "SS" ) {
		map.propertyValue( name ) = util::listToList<int32_t>( val.begin(), val.end() );
	} else if ( vr == "UL" ) {
		map.propertyValue( name ) = val; // @todo we dont have an unsigned int list
	} else if ( vr == "LO" or vr == "SH" or vr == "UN" or vr == "ST" or vr == "SL" ) {
		map.propertyValue( name ) = val;
	} else if ( vr == "DS" or vr == "FD" ) {
		map.propertyValue( name ) = util::listToList<double>( val.begin(), val.end() );
	} else {
		LOG( Runtime, error ) << "Don't know how to parse CSA entry " << std::make_pair( name, val ) << " type is " << util::MSubject( vr );
		return false;
	}

	return true;
}

void ImageFormat_Dicom::dcmObject2PropMap( DcmObject *master_obj, util::PropertyMap &map, const util::istring &dialect )const
{
	for ( DcmObject *obj = master_obj->nextInContainer( NULL ); obj; obj = master_obj->nextInContainer( obj ) ) {
		const DcmTagKey &tag = obj->getTag();

		if ( tag == DcmTagKey( 0x7fe0, 0x0010 ) )
			continue;//skip the image data
		else if ( tag == DcmTagKey( 0x0029, 0x1010 ) || tag == DcmTagKey( 0x0029, 0x1020 ) ) { //CSAImageHeaderInfo
			bool known = map.hasProperty( "Private Code for (0029,1000)-(0029,10ff)" );
			std::string as = map.getPropertyAs<std::string>( "Private Code for (0029,1000)-(0029,10ff)" );

			if( known && as == "SIEMENS CSA HEADER" ) {
				const util::PropertyMap::PropPath name = ( tag == DcmTagKey( 0x0029, 0x1010 ) ) ? "CSAImageHeaderInfo" : "CSASeriesHeaderInfo";
				LOG( Debug, info ) << "Using " << tag.toString() << " as " << name;
				DcmElement *elem = dynamic_cast<DcmElement *>( obj );
				parseCSA( elem, map.branch( name ), dialect );
			} else {
				LOG( Runtime, warning ) << "Ignoring entry " << tag.toString() << ", binary format " << as << " is not known";
			}
		} else if ( tag == DcmTagKey( 0x0029, 0x0020 ) ) { //MedComHistoryInformation
			//@todo special handling needed
			LOG( Debug, info ) << "Ignoring MedComHistoryInformation at " << tag.toString();
		} else if ( obj->isLeaf() ) { // common case
			if ( obj->getTag() == DcmTag( 0x0008, 0x0032 ) ) {
				OFString buff;
				dynamic_cast<DcmElement *>( obj )->getOFString( buff, 0 );

				if( buff.length() < 8 ) {
					LOG( Runtime, warning ) << "The Acquisition Time " << util::MSubject( buff ) << " is not precise enough, ignoring it";
					continue;
				}
			}

			DcmElement *elem = dynamic_cast<DcmElement *>( obj );
			const size_t mult = obj->getVM();

			if ( mult == 0 )
				LOG( Runtime, verbose_info ) << "Skipping empty Dicom-Tag " << util::MSubject( tag2Name( tag ) );
			else if ( mult == 1 )
				parseScalar( elem, tag2Name( tag ), map );
			else
				parseList( elem, tag2Name( tag ), map );
		} else {
			dcmObject2PropMap( obj, map.branch( tag2Name( tag ) ), dialect );
		}
	}
}

}
}
