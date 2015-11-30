#include "propmap.hpp"
#include <json/json.h>
#include <memory>

namespace isis
{
namespace util
{
namespace _internal {
	void value2prop(PropertyMap &branch,const PropertyMap::PropPath &path,const Json::Value &val, char extra_token);
	template< typename T, class W > void value2prop_array(PropertyMap &branch,const PropertyMap::PropPath &path,const Json::Value &val,const W &wrapper){
		PropertyValue &property=branch.touchProperty(path);
		for(const Json::Value &v:val){
			if(v.isArray()) // flattening more-dimensional arrays
				value2prop_array<T>(branch,path,v,wrapper);
			else
				property.push_back<T>(wrapper(v));
		}
	}
	void object2Branch(PropertyMap &branch,Json::Value obj, char extra_token){
		assert(obj.isObject());
		for(const std::string &name:obj.getMemberNames()){
			const PropertyMap::PropPath path(
				util::stringToList<PropertyMap::key_type>(PropertyMap::key_type(name.c_str()),extra_token)
			);
			std::cout << name << ":" << obj.toStyledString() << std::endl;
			value2prop(branch,path,obj[name],extra_token);
		}
	}
	template<class C> bool checkArray(const Json::Value &array,const C &checker){
		assert(array.isArray());
		bool good=true;
		for(Json::Value::const_iterator i=array.begin();good && i!=array.end();i++){
			if(i->isArray())
				good=checkArray(*i,checker);
			else
				good=checker(*i);
		}
		return good;
	}
	void value2prop(PropertyMap &branch,const PropertyMap::PropPath &path,const Json::Value &val, char extra_token){
		switch(val.type()){
			case Json::intValue:{
				if(val.isInt())branch.setValueAs<int32_t>(path,val.asInt());
				else branch.setValueAs<int64_t>(path,val.asInt64());
			}break;
			case Json::uintValue:{
				if(val.isUInt())branch.setValueAs<u_int32_t>(path,val.asUInt());
				else branch.setValueAs<u_int64_t>(path,val.asUInt64());
			}break;
			case Json::realValue:branch.setValueAs(path,val.asDouble()); break;
			case Json::stringValue:branch.setValueAs(path,val.asString()); break;
			case Json::booleanValue:branch.setValueAs(path,val.asBool()); break;
			case Json::arrayValue:{
				if(checkArray(val,std::mem_fn(&Json::Value::isBool)))
					value2prop_array<bool>(branch,path,val,std::mem_fn(&Json::Value::asBool));
				else if(checkArray(val,std::mem_fn(&Json::Value::isInt)))
					value2prop_array<int32_t>(branch,path,val,std::mem_fn(&Json::Value::asInt));
				else if(checkArray(val,std::mem_fn(&Json::Value::isUInt)))
					value2prop_array<uint32_t>(branch,path,val,std::mem_fn(&Json::Value::asUInt));
				else if(checkArray(val,std::mem_fn(&Json::Value::isInt64)))
					value2prop_array<int64_t>(branch,path,val,std::mem_fn(&Json::Value::asInt64));
				else if(checkArray(val,std::mem_fn(&Json::Value::isUInt64)))
					value2prop_array<u_int64_t>(branch,path,val,std::mem_fn(&Json::Value::asUInt64));
				else if(checkArray(val,std::mem_fn(&Json::Value::isDouble)))
					value2prop_array<double>(branch,path,val,std::mem_fn(&Json::Value::asDouble));
				else 
					value2prop_array<std::string>(branch,path,val,std::mem_fn(&Json::Value::asString));
			}break;
			case Json::objectValue:object2Branch(branch.touchBranch(path),val,extra_token);break;
			case Json::nullValue:break;
		}
	}
}
ptrdiff_t PropertyMap::readJson( const uint8_t* streamBegin, const uint8_t* streamEnd, char extra_token, std::string list_trees )
{
	Json::CharReaderBuilder builder;
	Json::Value root;
	std::string errs;
	builder["collectComments"] = false;
	std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	const bool good= reader->parse((char*)streamBegin,(char*)streamEnd,&root,&errs);
	
	_internal::object2Branch(*this,root,extra_token);

	return good?0:-1; //@todo implement me
}

}
}