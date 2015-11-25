#include "propmap.hpp"
#include <json/json.h>
#include <memory>

namespace isis
{
namespace util
{

ptrdiff_t PropertyMap::readJson( const uint8_t* streamBegin, const uint8_t* streamEnd, char extra_token, std::string list_trees )
{
	Json::CharReaderBuilder builder;
	Json::Value root;
	std::string errs;
	builder["collectComments"] = false;
	std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	const bool good= reader->parse((char*)streamBegin,(char*)streamEnd,&root,&errs);
	return -1; //@todo implement me
}

}
}