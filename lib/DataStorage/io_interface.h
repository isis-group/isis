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
#include <DataStorage/image.hpp>

namespace isis{ namespace image_io{
class FileFormat {
public:
	virtual std::string name()=0;
	virtual std::string suffixes()=0;
	virtual std::string dialects()=0;
	virtual size_t maxDim()=0;
	virtual bool tainted(){return true;}
	virtual data::ChunkList load(const std::string& filename,const std::string& dialect)=0;
	virtual bool write(const data::Image &image,const std::string& filename,const std::string& dialect)=0;
	virtual bool write(const data::ImageList &images,const std::string& filename,const std::string& dialect);
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
