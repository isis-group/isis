#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H

#ifdef __cplusplus
#include <list>
#include <string>

namespace isis{ namespace data{
class FileFormat {
public:
	struct format{
		std::string name;
		std::list<std::string> suffixes,dialects;
	};
	virtual std::string name()=0;
	virtual std::list<format> formats()=0;
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