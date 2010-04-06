/*
 *  selection.hpp
 *  isis
 *
 *  Created by Enrico Reimer on 03.04.10.
 *  Copyright 2010 cbs.mpg.de. All rights reserved.
 *
 */

#ifndef SELECTION_HPP_INCLUDED
#define SELECTION_HPP_INCLUDED
#include <map>
#include "common.hpp"

namespace isis{ namespace util{

class Selection
{
	typedef std::map<std::string,int,_internal::caselessStringLess> map_type;
	map_type ent_map;
	int m_set;
public:
	Selection(const char *entries);
	Selection();
	bool set(const char* entry);
	operator const int()const;
	operator const std::string()const;
	bool operator==(const Selection &ref)const;
	bool operator==(const char ref[])const;
	bool operator==(const int ref)const;
	
	std::list<std::string> getEntries()const;
};
}}

namespace std{
template<typename charT, typename traits>
basic_ostream<charT,traits> &operator<<(basic_ostream<charT,traits> &out,const isis::util::Selection &s)
{
	return out << (std::string)s;
}

template<typename charT, typename traits>
basic_istream<charT,traits> &operator>>(basic_istream<charT,traits> &in, isis::util::Selection &s)
{
	std::string dummy;
	in >> dummy;
	s.set(dummy.c_str());
	return in;
}
}

#endif //SELECTION_HPP_INCLUDED
