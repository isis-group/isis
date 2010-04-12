//
// C/C++ Interface: io_interface
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H

#ifdef __cplusplus
#include <string>
#include "DataStorage/image.hpp"
#include "ImageIO/common.hpp"

namespace isis{ namespace image_io{

/// Base class for image-io-plugins
class FileFormat {
protected:
	/**
	 * Check if a given property exists in the given PropMap.
	 * If the property doesn't exist a message will be sent to Log using the given loglevel.
	 * \returns object.hasProperty(name)
	 */
	static bool hasOrTell(const std::string& name, const util::PropMap& object, LogLevel level);
	/**
	 * Transform a given property into another and remove the original in the given PropMap.
	 * If the property doesn't exist a message will be sent to Log using the given loglevel.
	 * \returns true if the property existed and was transformed.
	 */
	template<typename TYPE> static bool
	transformOrTell(const std::string& from,const std::string& to, util::PropMap& object, LogLevel level)
	{
		if(hasOrTell(from,object,level) and object.transform<TYPE>(from,to)){
			LOG(Debug,verbose_info) << "Transformed " << from << " into " << object[to];
			return true;
		}
		return false;
	}
public:
	static const float invalid_float;
	virtual std::string name()=0;
	virtual std::string suffixes()=0;
	virtual std::string dialects(){return std::string();};
	virtual bool tainted(){return true;}
	virtual int load(data::ChunkList &chunks,const std::string& filename,const std::string& dialect)=0;
	virtual bool write(const data::Image &image,const std::string& filename,const std::string& dialect)=0;
	virtual bool write(const data::ImageList &images,const std::string& filename,const std::string& dialect);
	virtual ~FileFormat(){}
};
}}
#else
typedef struct FileFormat FileFormat;
#endif


#ifdef __cplusplus
extern "C" {
#endif
	
#if defined(__STDC__) || defined(__cplusplus)
	extern isis::image_io::FileFormat* factory();
#else
	extern FileFormat* factory();
#endif
	
#ifdef __cplusplus
}
#endif

#endif //IO_INTERFACE_H
