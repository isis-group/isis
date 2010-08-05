#include "imageFormat_Dicom.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "common.hpp"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcdicent.h"

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
	return util::string2list<T>( std::string( buff.c_str() ), '\\' );
}

}
/**
 * Parses the Age String
 * A string of characters with one of the following formats -- nnnD, nnnW, nnnM, nnnY;
 * where nnn shall contain the number of days for D, weeks for W, months for M, or years for Y.
 * Example - "018M" would represent an age of 18 months.
 */
void ImageFormat_Dicom::parseAS( DcmElement *elem, const std::string &name, util::PropMap &map )
{
	u_int16_t duration;
	OFString buff;
	elem->getOFString( buff, 0 );

	if ( _internal::try_cast( buff.substr( 0, 3 ), duration ) ) {
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
			duration *= 30.436875;// year/12
			break;
		case 'Y':
		case 'y':
			duration *= 365.2425; //mean length of a year
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
void ImageFormat_Dicom::parseDA( DcmElement *elem, const std::string &name, util::PropMap &map )
{
	//@todo if we drop support for old yyyy.mm.dd this would be much easier
	boost::regex reg( "^([[:digit:]]{4})\\.?([[:digit:]]{2})\\.?([[:digit:]]{2})$" );
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
void ImageFormat_Dicom::parseTM( DcmElement *elem, const std::string &name, util::PropMap &map )
{
	short shift = 0;
	OFString buff;
	bool ok = true;
	boost::posix_time::time_duration time;
	elem->getOFString( buff, 0 );

	// Insert the ":" -- make it hh:mm:ss.frac
	if ( buff.at( 2 ) != ':' ) {
		buff.insert( 2, 1, ':' );
		shift++;
	}

	if ( ( buff.size() > size_t( 4 + shift ) ) && ( buff.at( 4 + shift ) != ':' ) ) {
		buff.insert( 4 + shift, 1, ':' );
		shift++;
	}

	//Try standard-parser for hh:mm:ss.frac
	try {
		time = boost::posix_time::duration_from_string( buff.c_str() );
		ok = not time.is_not_a_date_time();
	} catch ( std::logic_error e ) {
		ok = false;
	}

	if ( ok ) {
		LOG( Debug, verbose_info )
				<< "Parsed time for " << name << "(" <<  buff << ")" << " as " << time;
		map.propertyValue( name ) = boost::posix_time::ptime( boost::gregorian::date( 1400, 1, 1 ), time );
		//although TM is defined as time of day we dont have a day here, so we fake one
	} else
		LOG( Runtime, warning )
				<< "Cannot parse Time string \"" << buff << "\" in the field \"" << name << "\"";
}

void ImageFormat_Dicom::parseScalar( DcmElement *elem, const std::string &name, util::PropMap &map )
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
		map.propertyValue( name ) = buff; //if Float32 is float its fine, if not we will get an linker error here
	}
	break;
	case EVR_FD: {
		Float64 buff;
		elem->getFloat64( buff );
		map.propertyValue( name ) = buff; //if Float64 is double its fine, if not we will get an linker error here
	}
	break;
	case EVR_DS: { //Decimal String (can be floating point)
		elem->getOFString( buff, 0 );
		map.propertyValue( name ) = boost::lexical_cast<double>( buff );
	}
	break;
	case EVR_SL: { //signed long
		Sint32 buff;
		elem->getSint32( buff );
		map.propertyValue( name ) = buff;
	}
	break;
	case EVR_SS: { //signed short
		Sint16 buff;
		elem->getSint16( buff );
		map.propertyValue( name ) = buff;
	}
	break;
	case EVR_UL: { //unsigned long
		Uint32 buff;
		elem->getUint32( buff );
		map.propertyValue( name ) = buff;
	}
	break;
	case EVR_US: { //unsigned short
		Uint16 buff;
		elem->getUint16( buff );
		map.propertyValue( name ) = buff;
	}
	break;
	case EVR_IS: { //integer string
		elem->getOFString( buff, 0 );
		map.propertyValue( name ) = boost::lexical_cast<int32_t>( buff );
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
		map.propertyValue( name ) = boost::lexical_cast<std::string>( buff );
	}
	break;
	default: {
		elem->getOFString( buff, 0 );
		LOG( Runtime, info ) << "Implement me "
							 << name << "("
							 << const_cast<DcmTag &>( elem->getTag() ).getVRName() << "):"
							 << buff;
	}
	break;
	}
}

void ImageFormat_Dicom::parseVector( DcmElement *elem, const std::string &name, util::PropMap &map )
{
	OFString buff;
	size_t len = elem->getVM();

	switch ( elem->getVR() ) {
	case EVR_FL: {
		Float32 *buff;
		elem->getFloat32Array( buff );
		util::fvector4 vector;
		vector.copyFrom( buff, buff + len );
		map.propertyValue( name ) = vector; //if Float32 is float its fine, if not we will get an linker error here
	}
	break;
	case EVR_FD: {
		Float64 *buff;
		elem->getFloat64Array( buff );
		util::dvector4 vector;
		vector.copyFrom( buff, buff + len );
		map.propertyValue( name ) = vector; //if Float64 is double its fine, if not we will get an linker error here
	}
	break;
	case EVR_IS: {
		const util::ilist tokens = _internal::dcmtkListString2list<int>( elem );
		util::ivector4 vector;
		vector.copyFrom( tokens.begin(), tokens.end() );
		map.propertyValue( name ) = vector;
	}
	break;
	case EVR_SL: {
		Sint32 *buff;
		elem->getSint32Array( buff );
		util::ivector4 vector;
		vector.copyFrom( buff, buff + len );
		map.propertyValue( name ) = vector;
	}
	break;
	case EVR_US: {
		Uint16 *buff;
		elem->getUint16Array( buff );
		util::ivector4 vector;
		vector.copyFrom( buff, buff + len );
		map.propertyValue( name ) = vector;
	}
	break;
	case EVR_CS: // Code String (string)
	case EVR_SH: //short string
	case EVR_ST: { //short text
		map.propertyValue( name ) = _internal::dcmtkListString2list<std::string>( elem );
	}
	break;
	case EVR_DS: {
		const util::dlist tokens = _internal::dcmtkListString2list<double>( elem );
		util::dvector4 vector;
		vector.copyFrom( tokens.begin(), tokens.end() );
		map.propertyValue( name ) = vector;
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
		LOG( Runtime, info ) << "Implement me "
							 << name << "("
							 << const_cast<DcmTag &>( elem->getTag() ).getVRName() << "):"
							 << buff;
	}
	break;
	}

	LOG( Debug, verbose_info ) << "Parsed the vector " << name << " as " << map.propertyValue( name );
}

void ImageFormat_Dicom::parseList( DcmElement *elem, const std::string &name, util::PropMap &map )
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
		map.propertyValue( name ) = _internal::dcmtkListString2list<string>( elem );
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
		LOG( Runtime, info ) << "Implement me "
							 << name << "("
							 << const_cast<DcmTag &>( elem->getTag() ).getVRName() << "):"
							 << buff;
	}
	break;
	}

	LOG( Debug, verbose_info ) << "Parsed the list " << name << " as " << map.propertyValue( name );
}

void ImageFormat_Dicom::parseCSA( DcmElement *elem, isis::util::PropMap &map, const std::string &dialect )
{
	Uint8 *array;
	elem->getUint8Array( array );
	const size_t len = elem->getLength();

	for ( std::string::size_type pos = 0x10; pos < ( len - sizeof( Sint32 ) ); ) {
		pos += parseCSAEntry( array + pos, map, dialect );
	}
}
size_t ImageFormat_Dicom::parseCSAEntry( Uint8 *at, isis::util::PropMap &map, const std::string &dialect )
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

			if( std::string( "MrPhoenixProtocol" ) != name  || dialect == "withPhoenixProtocol" ) {
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
				LOG( Runtime, info ) << "Skipping MrPhoenixProtocol as its not requested by the dialect";
			}

			pos += (
					   ( len + sizeof( Sint32 ) - 1 ) / sizeof( Sint32 )
				   ) *
				   sizeof( Sint32 );//increment pos by len aligned to sizeof(Sint32)*/
		}

		try {
			if ( ret.size() == 1 ) {
				if ( parseCSAValue( ret.front(), name, vr, map ) ) {
					LOG( Debug, verbose_info ) << "Found scalar entry " << name << ":" << map.propertyValue( name ) << " in CSA header";
				}
			} else if ( ret.size() > 1 ) {
				if ( parseCSAValueList( ret, name, vr, map ) ) {
					LOG( Debug, verbose_info ) << "Found list entry " << name << ":" << map.propertyValue( name ) << " in CSA header";
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

bool ImageFormat_Dicom::parseCSAValue( const std::string &val, const std::string &name, const char *const vr, isis::util::PropMap &map )
{
	if ( strcasecmp( vr, "IS" ) == 0 or strcasecmp( vr, "SL" ) == 0 ) {
		map.propertyValue( name ) = boost::lexical_cast<int32_t>( val );
	} else if ( strcasecmp( vr, "UL" ) == 0 ) {
		map.propertyValue( name ) = boost::lexical_cast<u_int32_t>( val );
	} else if (
		strcasecmp( vr, "CS" ) == 0 or
		strcasecmp( vr, "LO" ) == 0 or strcasecmp( vr, "SH" ) == 0 or
		strcasecmp( vr, "UN" ) == 0 or strcasecmp( vr, "ST" ) == 0 ) {
		map.propertyValue( name ) = val;
	} else if ( strcasecmp( vr, "DS" ) == 0 or strcasecmp( vr, "FD" ) == 0 ) {
		map.propertyValue( name ) = boost::lexical_cast<double>( val );
	} else if ( strcasecmp( vr, "US" ) == 0 ) {
		map.propertyValue( name ) = boost::lexical_cast<u_int16_t>( val );
	} else if ( strcasecmp( vr, "SS" ) == 0 ) {
		map.propertyValue( name ) = boost::lexical_cast<int16_t>( val );
	} else {
		LOG( Runtime, error ) << "Dont know how to parse CSA entry " << std::make_pair( name, val ) << " type is " << util::MSubject( vr );
		return false;
	}

	return true;
}
bool ImageFormat_Dicom::parseCSAValueList( const util::slist &val, const std::string &name, const char *const vr, isis::util::PropMap &map )
{
	if ( strcasecmp( vr, "IS" ) == 0 or strcasecmp( vr, "SL" ) == 0 or
		 strcasecmp( vr, "US" ) == 0 or strcasecmp( vr, "SS" ) == 0 ) {
		map.propertyValue( name ) = util::list2list<int32_t>( val.begin(), val.end() );
	} else if ( strcasecmp( vr, "UL" ) == 0 ) {
		map.propertyValue( name ) = val; // @todo we dont have an unsigned int list
	} else if ( strcasecmp( vr, "LO" ) == 0 or strcasecmp( vr, "SH" ) == 0 or strcasecmp( vr, "UN" ) == 0 or strcasecmp( vr, "ST" ) == 0 or strcasecmp( vr, "SL" ) == 0 ) {
		map.propertyValue( name ) = val;
	} else if ( strcasecmp( vr, "DS" ) == 0 or strcasecmp( vr, "FD" ) == 0 ) {
		map.propertyValue( name ) = util::list2list<double>( val.begin(), val.end() );
	} else {
		LOG( Runtime, error ) << "Dont know how to parse CSA entry " << std::make_pair( name, val ) << " type is " << util::MSubject( vr );
		return false;
	}

	return true;
}

void ImageFormat_Dicom::dcmObject2PropMap( DcmObject *master_obj, isis::util::PropMap &map, const std::string &dialect )
{
	for ( DcmObject *obj = master_obj->nextInContainer( NULL ); obj; obj = master_obj->nextInContainer( obj ) ) {
		const DcmTag &tag = obj->getTag();
		std::string name;
		const DcmDataDictionary &globalDataDict = dcmDataDict.rdlock();
		const DcmDictEntry *dictRef = globalDataDict.findEntry( tag, tag.getPrivateCreator() );
		LOG( Debug, verbose_info ) << "Parsing " << tag.toString();

		if ( dictRef ) name = dictRef->getTagName();
		else
			name = std::string( unknownTagName ) + tag.toString().c_str();

		dcmDataDict.unlock();

		if ( name == "PixelData" )
			continue;//skip the image data
		else if ( name == "CSAImageHeaderInfo" ) {
			DcmElement *elem = dynamic_cast<DcmElement *>( obj );
			parseCSA( elem, map.branch( "CSAImageHeaderInfo" ), dialect );
		} else if ( name == "CSASeriesHeaderInfo" ) {
			DcmElement *elem = dynamic_cast<DcmElement *>( obj );
			parseCSA( elem, map.branch( "CSASeriesHeaderInfo" ), dialect );
		} else if ( name == "MedComHistoryInformation" ) {
			//@todo special handling needed
			LOG( Debug, info ) << "Ignoring MedComHistoryInformation";
		} else if ( obj->isLeaf() ) {
			DcmElement *elem = dynamic_cast<DcmElement *>( obj );
			const size_t mult = obj->getVM();

			if ( mult == 0 )
				LOG( Runtime, verbose_info ) << "Skipping empty Dicom-Tag " << name;
			else if ( mult == 1 )
				parseScalar( elem, name, map );
			else if ( mult <= 4 )
				parseVector( elem, name, map );
			else
				parseList( elem, name, map ); // for any other value
		} else {
			dcmObject2PropMap( obj, map.branch( name ), dialect );
		}
	}
}

}
}
