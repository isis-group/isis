/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef PROGPARAMETER_HPP
#define PROGPARAMETER_HPP

#include "property.hpp"
#include <string>
#include <map>

namespace isis{namespace util{

class ProgParameter: public PropertyValue
{
	std::string m_description;
public:
	ProgParameter();
	template<typename T> ProgParameter(const T& ref,bool _needed = true):PropertyValue(ref,_needed){}
	bool parse(const isis::util::Type< std::string >& props);
	const std::string &description()const;
	void setDescription(const std::string &desc);
	template<typename T> operator const T()const{
		LOG_IF(empty(),isis::CoreDebug,isis::error) << "Program parameters must not be empty. Please set it to any value.";
		return get()->cast_to_Type<T>();
	}
	operator const int()const{
		LOG_IF(empty(),isis::CoreDebug,isis::error) << "Program parameters must not be empty. Please set it to any value.";
		if(get()->is<bool>()) //hack to make if(ProgParameter) possible
			return get()->cast_to_Type<bool>();
		else if(get()->is<Selection>()) //use the implicit cast of selection to int
			return ((Selection)get()->cast_to_Type<Selection>());
		else
			return get()->cast_to_Type<int>();
	}
};

class ParameterMap: public std::map<std::string,ProgParameter>{
	struct neededP{
		bool operator()(const_reference ref)const{return ref.second.needed();}
	};
	struct notneededP{
		bool operator()(const_reference ref)const{return not ref.second.needed();}
	};
	bool parsed;
public:
	ParameterMap();
	bool parse(int argc, char** argv);
	bool isComplete()const;
	void printAll()const;
	void printNeeded()const;
};
	
}}

namespace std {
/// Streaming output for ProgParameter - classes
template<typename charT, typename traits> basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits> &out,const isis::util::ProgParameter &s){
	const std::string &desc=s.description();
	if(not desc.empty()){
		out << desc << ", ";
	}
	LOG_IF(s.empty(),isis::CoreDebug,isis::error) << "Program parameters must not be empty. Please set it to any value.";
	assert(not s.empty());
	out << "default=\"" << s.toString(false)<< "\", type=" << s->typeName();
	if(s.needed())out << " (needed)";
	return out;
}
}
#endif // PROGPARAMETER_HPP
