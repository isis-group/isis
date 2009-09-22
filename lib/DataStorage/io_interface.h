#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H

#ifdef __cplusplus
#include <list>
#include <string>

namespace isis{ namespace data{
class FileReader {
public:
	virtual std::string name()=0;
	virtual std::string suffixes()=0;
	virtual std::string dialects()=0;
	virtual bool tainted(){return true;}
};
}}
#else
typedef struct FileFormat FileFormat;
#endif


#ifdef __cplusplus
extern "C" {
#endif
	
#if defined(__STDC__) || defined(__cplusplus)
	extern isis::data::FileReader* factory();
#else
	extern FileFormat* factory();
#endif
	
#ifdef __cplusplus
}
#endif

#endif //IO_INTERFACE_H