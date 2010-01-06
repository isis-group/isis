#include "imageFormat_Dicom.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/date_time/gregorian/conversion.hpp>
#include <boost/date_time/posix_time/conversion.hpp>

namespace isis { namespace image_io {

void ImageFormat_Dicom::parseAS(DcmElement* elem,const std::string &name,util::PropMap &map)
{
	bool ok = true;
	long duration;
	OFString buff;
	elem->getOFString(buff, 0);
	
	try {
		duration = boost::lexical_cast<int64_t>(buff.substr(0, 3));
	} catch (boost::bad_lexical_cast e) {
		ok = false;
	}
	
	if (ok) {
		switch (buff.at(buff.size() - 1)) {
			case 'D':
			case 'd':
				break;
			case 'W':
			case 'w':
				duration *= 7;
				break;
			case 'M':
			case 'm':
				duration *= 30.5;// this is not precise (ignores leap years)
				break;
			case 'Y':
			case 'y':
				duration *= 365;// this is not precise (ignores leap years)
				break;
			default:
				LOG(ImageIoLog, util::warning)
				<< "Missing age-type-letter, assuming days";;
		}
		
		map[name] = duration;
		
		LOG(ImageIoDebug, util::verbose_info)
		<< "Parsed age for " << name << "(" <<  buff << ")"
		<< " as " << duration << " days";
	} else
		LOG(ImageIoLog, util::warning)
		<< "Cannot parse age string \"" << buff << "\" in the field \"" << name << "\"";
}
	
void ImageFormat_Dicom::parseDA(DcmElement* elem,const std::string &name,util::PropMap &map)
{
	//@todo if we drop support for old yyyy.mm.dd this would be much easier
	boost::regex reg("^([[:digit:]]{4})\\.?([[:digit:]]{2})\\.?([[:digit:]]{2})$");
	boost::cmatch results;
	OFString buff;
	elem->getOFString(buff, 0);
	
	if (boost::regex_match(buff.c_str(), results, reg)) {
		boost::gregorian::date date(
		boost::lexical_cast<int16_t>(results.str(1)), //year
									boost::lexical_cast<int16_t>(results.str(2)), //month
																 boost::lexical_cast<int16_t>(results.str(3)) //day of month
																 );
																 LOG(ImageIoDebug, util::verbose_info) << "Parsed date for " << name <<
																 "(" <<  buff << ")" << " as " << date;
																 tm time = boost::gregorian::to_tm(date);
																 map[name] = mktime(&time);
	} else
		LOG(ImageIoLog, util::warning)
		<< "Cannot parse Date string \"" << buff << "\" in the field \"" << name << "\"";
}
	
void ImageFormat_Dicom::parseTM(DcmElement* elem,const std::string &name,util::PropMap &map)
{
	short shift = 0;
	OFString buff;
	bool ok = true;
	boost::posix_time::time_duration time;
	
	elem->getOFString(buff, 0);

	// Insert the ":"
	if (buff.at(2) != ':') {
		buff.insert(2, 1, ':');
		shift++;
	}
	if (buff.size() > 4 + shift && buff.at(4 + shift) != ':') {
		buff.insert(4 + shift, 1, ':');
		shift++;
	}

	//Try standard-parser
	try {
		time = boost::posix_time::duration_from_string(buff.c_str());
		ok = not time.is_not_a_date_time();
	} catch (std::logic_error e) {
		ok = false;
	}

	if (ok) {
		LOG(ImageIoDebug, util::verbose_info)
		<< "Parsed time for " << name << "(" <<  buff << ")" << " as " << time;
		tm tm_time = boost::posix_time::to_tm(time);
		map[name] = mktime(&tm_time);
	} else
		LOG(ImageIoLog, util::warning)
		<< "Cannot parse Time string \"" << buff << "\" in the field \"" << name << "\"";
}

void ImageFormat_Dicom::dcmObject2PropMap(DcmObject* master_obj, util::PropMap &map)
{
	for (DcmObject* obj = master_obj->nextInContainer(NULL);obj;obj = master_obj->nextInContainer(obj)) {
		std::string name(const_cast<DcmTag&>(obj->getTag()).getTagName());

		if (name == "PixelData")
			continue;//skip the image data
		else if (name == "CSAImageHeaderInfo") {
			map["CSAImageHeaderInfo"] = util::PropMap();
			util::Type<util::PropMap> &csa = map["CSAImageHeaderInfo"]->cast_to_Type<util::PropMap>();
			//@special handling needed
		} else if (name == "CSASeriesHeaderInfo") {
			//@special handling needed
		} else if (name == "MedComHistoryInformation") {
			//@special handling needed
		} else if (obj->isLeaf()) {
			DcmElement* elem = dynamic_cast<DcmElement*>(obj);
			OFString buff;

			switch (obj->getVR()) {
			case EVR_AS: { // age string (nnnD, nnnW, nnnM, nnnY)
				parseAS(elem,name,map);
			}break;
			case EVR_DA: {
				parseDA(elem,name,map);
			}break;
			case EVR_TM: {
				parseTM(elem,name,map);
			}break;
			case EVR_FL: {
				Float32 buff;
				elem->getFloat32(buff);
				map[name] = buff;//if Float32 is float its fine, if not we will get an linker error here
			}break;
			case EVR_FD: {
				Float64 buff;
				elem->getFloat64(buff);
				map[name] = buff;//if Float64 is double its fine, if not we will get an linker error here
			}break;
			case EVR_DS: { //Decimal String (can be floating point)
				elem->getOFString(buff, 0);
				map[name] = boost::lexical_cast<double>(buff);
			}break;
			case EVR_SL: { //signed long
				Sint32 buff;
				elem->getSint32(buff);
				map[name] = buff;
			}break;
			case EVR_SS: { //signed short
				Sint16 buff;
				elem->getSint16(buff);
				map[name] = buff;
			}break;
			case EVR_UL: { //unsigned long
				Uint32 buff;
				elem->getUint32(buff);
				map[name] = buff;
			}break;
			case EVR_US: { //unsigned short
				Uint16 buff;
				elem->getUint16(buff);
				map[name] = buff;
			}break;
			case EVR_IS: { //integer string
				elem->getOFString(buff, 0);
				map[name] = boost::lexical_cast<int32_t>(buff);
			}break;
			case EVR_AE: //Application Entity (string)
			case EVR_CS: // Code String (string)
			case EVR_LT: //long text
			case EVR_SH: //short string
			case EVR_LO: //long string
			case EVR_ST: //short text
			case EVR_UT: //Unlimited Text
			case EVR_UI: //Unique Identifier [0-9\.]
			case EVR_PN: { //Person Name
				elem->getOFString(buff, 0);
				map[name] = boost::lexical_cast<std::string>(buff);
			}break;
			default: {
				LOG(ImageIoLog, util::warning) << "Implement me "
				<< name << "("
				<< const_cast<DcmTag&>(obj->getTag()).getVRName() << "):"
				<< buff;
			}break;
			}
		} else {
			map[name] = util::PropMap();
			dcmObject2PropMap(obj, map[name]->cast_to_Type<util::PropMap>());
		}
	}
}

}}
