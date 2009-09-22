#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H

#ifdef __cplusplus
#include <string>
#include "chunk.hpp"

namespace isis{ namespace data{
class FileFormat {
public:
	virtual std::string name()=0;
	virtual std::string suffixes()=0;
	virtual std::string dialects()=0;
	virtual bool tainted(){return true;}
	virtual Chunks load(std::string filename,std::string dialect)=0;
	virtual bool save(const Chunks &chunks,std::string filename,std::string dialect)=0;
};
}}
#else
typedef struct FileFormat FileFormat;
#endif


#ifdef __cplusplus
extern "C" {
#endif
	
#if defined(__STDC__) || defined(__cplusplus)
	extern isis::data::FileFormat* factory();
#else
	extern FileFormat* factory();
#endif
	
#ifdef __cplusplus
}
#endif

#endif //IO_INTERFACE_H