//
// C++ Interface: common
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef CORE_COMMON_HPP
#define CORE_COMMON_HPP

#include <utility>
#include <ostream>
#include <map>
#include <string>
#include <sstream>


/*! \addtogroup util
*  Additional documentation for group `mygrp'
*  @{
*/
namespace isis { namespace util {

/**
Write a list of elements to a std::basic_ostream
\param start starting iterator of input
\param end end iterator of input
\param o the output stream to write into
\param delim delimiter used to seperate the elements (default: " ")
\param prefix will be send to the stream as start (default: "")
\param suffix will be send to the stream at the end (default: "")
*/
template<class InputIterator,typename _CharT, typename _Traits> std::basic_ostream<_CharT, _Traits> &
write_list(InputIterator start,InputIterator end,
		   std::basic_ostream<_CharT, _Traits> &o,
		   std::string delim=" ",
		   std::string prefix="{",std::string suffix="}"){
	o << prefix;
	if(start!=end){
		o << *start;
		start++;
	}
	for(InputIterator i=start;i!=end;i++)
		o << delim << *i;
	o << suffix;
	return o;
}

/// use write_list to create a string from a list
template<class InputIterator> std::string list2string(
	InputIterator start,InputIterator end,
	std::string delim=" ",
	std::string prefix="{",std::string suffix="}")
{
	std::ostringstream ret;
	write_list(start,end,ret,delim,prefix,suffix);
	return ret.str();
}
	
/// @cond _hidden
struct CoreLog{
	enum{
		use_rel = _ENABLE_CORE_LOG
	};
};
struct CoreDebug{
	enum{
		use_rel = _ENABLE_CORE_DEBUG
	};
};
/// @endcond
}}

namespace std {
/// Streaming output for std::pair
template<typename charT, typename traits, typename _FIRST, typename _SECOND > basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits> &out, const pair<_FIRST,_SECOND> &s)
{
	return out << s.first << ":" << s.second;
}

/// Streaming output for std::map
template<typename charT, typename traits,
typename _Key, typename _Tp, typename _Compare, typename _Alloc > basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits> &out,const map<_Key,_Tp,_Compare,_Alloc>& s)
{
	isis::util::write_list(s.begin(),s.end(),out,"\n","","");
	return out;
}
	
}
/** @} */
#endif //CORE_COMMON_HPP
