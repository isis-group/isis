#include "imageFormat_Dicom.hpp"
#include <clocale>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <isis/core/common.hpp>
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
	dest.property( name ) = vector; //if Float32 is float its fine, if not we will get an linker error here
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
		map.setValueAs( name, duration );
		LOG( Debug, verbose_info )
				<< "Parsed age for " << name << "(" <<  buff << ")" << " as " << duration << " days";
	} else
		LOG( Runtime, warning )
				<< "Cannot parse age string \"" << buff << "\" in the field \"" << name << "\"";
}

/**
 * Parses the Time string
 * For duration (VR=TM):
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
 * For timestamp (VR=TM and DA) see http://dicom.nema.org/dicom/2013/output/chtml/part05/sect_6.2.html
 */
void ImageFormat_Dicom::parseTime( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map,uint16_t dstID )
{
	OFString buff;
	elem->getOFString( buff, 0 );
	
	util::PropertyValue &prop=map.setValueAs( name, buff.c_str()); // store string
	
	if ( prop.transform(dstID) ) { // try to convert it into timestamp or date
		LOG( Debug, verbose_info ) << "Parsed time for " << name << "(" <<  buff << ")" << " as " << map.property(name).toString(true);
	} else
		LOG( Runtime, warning ) << "Cannot parse Time string \"" << buff << "\" in the field \"" << name << "\"";
}

void ImageFormat_Dicom::parseScalar( DcmElement *elem, const util::PropertyMap::PropPath &name, util::PropertyMap &map )
{
	switch ( elem->getVR() ) {
	case EVR_AS: { // age string (nnnD, nnnW, nnnM, nnnY)
		parseAS( elem, name, map );
	}
	break;
	case EVR_DA: {
		parseTime( elem, name, map, util::Value<util::date>::staticID() );
	}
	break;
	case EVR_TM: {
		parseTime( elem, name, map, util::Value<util::timestamp>::staticID() ); //duration is for milliseconds stored as decimal number
	}
	break;
	case EVR_DT: {
		parseTime( elem, name, map, util::Value<util::timestamp>::staticID() );
	}
	break;
	case EVR_FL: {
		Float32 buff;
		elem->getFloat32( buff );
		map.setValueAs<float>( name, buff ); //if Float32 is float its fine, if not we will get an compiler error here
	}
	break;
	case EVR_FD: {
		Float64 buff;
		elem->getFloat64( buff );
		map.setValueAs<double>( name, buff ); //if Float64 is double its fine, if not we will get an compiler error here
	}
	break;
	case EVR_DS: { //Decimal String (can be floating point)
		OFString buff;
		elem->getOFString( buff, 0 );
		map.setValueAs<double>( name, std::stod( buff.c_str() ) );
	}
	break;
	case EVR_SL: { //signed long
		Sint32 buff;
		elem->getSint32( buff );
		map.setValueAs<int32_t>( name, buff ); //seems like Sint32 is not allways int32_t, so enforce it
	}
	break;
	case EVR_SS: { //signed short
		Sint16 buff;
		elem->getSint16( buff );
		map.setValueAs<int16_t>( name, buff );
	}
	break;
	case EVR_UL: { //unsigned long
		Uint32 buff;
		elem->getUint32( buff );
		map.setValueAs<uint32_t>( name, buff );
	}
	break;
	case EVR_US: { //unsigned short
		Uint16 buff;
		elem->getUint16( buff );
		map.setValueAs<uint16_t>( name, buff );
	}
	break;
	case EVR_IS: { //integer string
		OFString buff;
		elem->getOFString( buff, 0 );
		map.setValueAs<int32_t>( name, std::stoi( buff.c_str() ) );
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
	case EVR_AT: // @todo find a better way to interpret the value (see http://northstar-www.dartmouth.edu/doc/idl/html_6.2/Value_Representations.html)
	case EVR_PN: { //Person Name
		OFString buff;
		elem->getOFString( buff, 0 );
		map.setValueAs<std::string>( name, buff.c_str() );
	}
	break;
	case EVR_UN: //Unknown, see http://www.dabsoft.ch/dicom/5/6.2.2/
	case EVR_OB:{ //bytes .. if it looks like text, use it as text
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
			map.setValueAs<std::string>( name, o.str() ); //stuff it into a string
		} else
			map.setValueAs<std::string>( name, std::string( ( char * )buff, len ) ); //stuff it into a string
	}
	break;
	case EVR_OW: { //16bit words - parse as base256 strings
		Uint16 *buff;
		elem->getUint16Array( buff ); // get the raw data
		Uint32 len = elem->getLength();
		std::stringstream o;
		std::copy( buff, buff + len, std::ostream_iterator<Uint16>( o << std::hex ) );
		map.setValueAs<std::string>( name, o.str() ); //stuff it into a string
	}
	break;
	default: {
		OFString buff;
		elem->getOFString( buff, 0 );
		LOG( Runtime, notice ) << "Don't know how to handle Value Representation " << util::MSubject(const_cast<DcmTag &>( elem->getTag() ).getVRName()) << 
		" of " << std::make_pair(name ,buff);
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
		map.setValueAs( name, util::dlist( buff, buff + len ) );
	}
	break;
	case EVR_FD: {
		Float64 *buff;
		elem->getFloat64Array( buff );
		map.setValueAs( name, util::dlist( buff, buff + len ) );
	}
	break;
	case EVR_IS: {
		map.setValueAs( name, _internal::dcmtkListString2list<int>( elem ));
	}
	break;
	case EVR_SL: {
		Sint32 *buff;
		elem->getSint32Array( buff );
		map.setValueAs( name, util::ilist( buff, buff + len ));
	}
	break;
	case EVR_US: {
		Uint16 *buff;
		elem->getUint16Array( buff );
		map.setValueAs( name, util::ilist( buff, buff + len ));
	}
	break;
	case EVR_SS: {
		Sint16 *buff;
		elem->getSint16Array( buff );
		map.setValueAs( name, util::ilist( buff, buff + len ));
	}
	break;
	case EVR_CS: // Code String (string)
	case EVR_SH: //short string
	case EVR_LT: //long text
	case EVR_LO: //long string
	case EVR_DA: //date string
	case EVR_TM: //time string
	case EVR_UT: //Unlimited Text
	case EVR_ST: { //short text
		map.setValueAs( name, _internal::dcmtkListString2list<std::string>( elem ));
	}
	break;
	case EVR_DS: {
		map.setValueAs( name, _internal::dcmtkListString2list<double>( elem ));
	}
	break;
	case EVR_AS:
	case EVR_UL:
	case EVR_AE: //Application Entity (string)
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

	LOG( Debug, verbose_info ) << "Parsed the list " << name << " as " << map.property( name );
}

void ImageFormat_Dicom::parseCSA( DcmElement *elem, util::PropertyMap &map, std::list<util::istring> dialects )
{
	Uint8 *array;
	elem->getUint8Array( array );
	const size_t len = elem->getLength();

	for ( std::string::size_type pos = 0x10; pos < ( len - sizeof( Sint32 ) ); ) {
		pos += parseCSAEntry( array + pos, map, dialects );
	}
}
size_t ImageFormat_Dicom::parseCSAEntry( Uint8 *at, util::PropertyMap &map, std::list<util::istring> dialects )
{
	size_t pos = 0;
	const char *const name = ( char * )at + pos;
	pos += 0x40;
	if(name[0]==0)
		throw std::logic_error("empty CSA entry name");
	/*Sint32 &vm=*((Sint32*)array+pos);*/
	pos += sizeof( Sint32 );
	const char *const vr = ( char * )at + pos;
	pos += 0x4;
	/*Sint32 syngodt=endian<Uint8,Uint32>(array+pos);*/
	pos += sizeof( Sint32 );
	const Sint32 nitems = endian<Uint8, Uint32>( at + pos );
	pos += sizeof( Sint32 );
	static const std::string whitespaces( " \t\f\v\n\r" );
	
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
				) || checkDialect(dialects, "withExtProtocols") ) {
				const std::string insert( ( char * )at + pos );
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
				LOG( Runtime, verbose_info ) << "Skipping " << name << " as its not requested by the dialect (use dialect \"withExtProtocols\" to get it)";
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
					LOG( Debug, verbose_info ) << "Found scalar entry " << path << ":" << map.property( path ) << " in CSA header";
				}
			} else if ( ret.size() > 1 ) {
				if ( parseCSAValueList( ret, path, vr, map ) ) {
					LOG( Debug, verbose_info ) << "Found list entry " << path << ":" << map.property( path ) << " in CSA header";
				}
			}
		} catch ( std::exception &e ) {
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
		map.setValueAs( name, std::stoi( val ));
	} else if ( vr == "UL" ) {
		map.setValueAs( name, boost::lexical_cast<uint32_t>( val ));
	} else if ( vr == "CS" or vr == "LO" or vr == "SH" or vr == "UN" or vr == "ST" or vr == "UT" ) {
		map.setValueAs( name, val );
	} else if ( vr == "DS" or vr == "FD" ) {
		map.setValueAs( name, std::stod( val ));
	} else if ( vr == "US" ) {
		map.setValueAs( name, boost::lexical_cast<uint16_t>( val ));
	} else if ( vr == "SS" ) {
		map.setValueAs( name, boost::lexical_cast<int16_t>( val ));
	} else if ( vr == "UT" or vr == "LT" ) {
		map.setValueAs( name, val);
	} else {
		LOG( Runtime, error ) << "Dont know how to parse CSA entry " << std::make_pair( name, val ) << " type is " << util::MSubject( vr );
		return false;
	}

	return true;
}
bool ImageFormat_Dicom::parseCSAValueList( const util::slist &val, const util::PropertyMap::PropPath &name, const util::istring &vr, util::PropertyMap &map )
{
	if ( vr == "IS" or vr == "SL" or vr == "US" or vr == "SS" ) {
		map.setValueAs( name, util::listToList<int32_t>( val.begin(), val.end() ) );
	} else if ( vr == "UL" ) {
		map.setValueAs( name, val); // @todo we dont have an unsigned int list
	} else if ( vr == "CS" or vr == "LO" or vr == "SH" or vr == "UN" or vr == "ST" or vr == "SL" ) {
		map.setValueAs( name, val );
	} else if ( vr == "DS" or vr == "FD" ) {
		map.setValueAs( name, util::listToList<double>( val.begin(), val.end() ) );
	} else if ( vr == "CS" ) {
		map.setValueAs( name, val );
	} else {
		LOG( Runtime, error ) << "Don't know how to parse CSA entry list " << std::make_pair( name, val ) << " type is " << util::MSubject( vr );
		return false;
	}

	return true;
}

void ImageFormat_Dicom::dcmObject2PropMap( DcmObject *master_obj, util::PropertyMap &map, std::list<util::istring> dialects )const
{
	const std::string  old_loc=std::setlocale(LC_ALL,"C");
	for ( DcmObject *obj = master_obj->nextInContainer( NULL ); obj; obj = master_obj->nextInContainer( obj ) ) {
		const DcmTagKey &tag = obj->getTag();

		if ( tag == DcmTagKey( 0x7fe0, 0x0010 ) )
			continue;//skip the image data
		else if ( tag == DcmTagKey( 0x0029, 0x1010 ) || tag == DcmTagKey( 0x0029, 0x1020 ) ) { //CSAImageHeaderInfo
			boost::optional< util::PropertyValue& > known = map.queryProperty( "Private Code for (0029,1000)-(0029,10ff)" );

			if( known && known->as<std::string>() == "SIEMENS CSA HEADER" ) {
				if(!checkDialect(dialects,"nocsa")){
					const util::PropertyMap::PropPath name = ( tag == DcmTagKey( 0x0029, 0x1010 ) ) ? "CSAImageHeaderInfo" : "CSASeriesHeaderInfo";
					DcmElement *elem = dynamic_cast<DcmElement *>( obj );
					try{
						parseCSA( elem, map.touchBranch( name ), dialects );
					} catch(std::exception &e){
						LOG( Runtime, error ) << "Error parsing CSA data ("<< util::MSubject(e.what()) <<"). Deleting " << util::MSubject(name);
					}
				}
			} else {
				LOG( Runtime, warning ) << "Ignoring entry " << tag.toString() << ", binary format " << *known << " is not known";
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
			dcmObject2PropMap( obj, map.touchBranch( tag2Name( tag ) ), dialects );
		}
	}
	std::setlocale(LC_ALL,old_loc.c_str());
}

}
}
