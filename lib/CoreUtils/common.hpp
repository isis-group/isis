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
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

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
		   const std::string delim=",",
		   const std::string prefix="{",const std::string suffix="}"){
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
	const std::string delim=",",
	const std::string prefix="{",const std::string suffix="}")
{
	std::ostringstream ret;
	write_list(start,end,ret,delim,prefix,suffix);
	return ret.str();
}

/**
 * Generic tokenizer.
 * Splits source into tokens and tries to lexically cast them to TARGET.
 * If that fails, boost::bad_lexical_cast is thrown.
 * \param source the source string to be split up
 * \param separator regular expression to delimit the tokens (defaults to \\s+)
 * \param prefix regular expression for text to be removed from the string before it is split up 
 * ("^" if not given, will be added at the beginning)
 * \param postfix regular expression for text to be removed from the string before it is split up
 * ("$" if not given, will be added at the end)
 * \returns a list of the casted tokens
 */
template<typename TARGET> std::list<TARGET> string2list(
	std::string source,	const boost::regex separator,
	boost::regex prefix,boost::regex postfix)
{
	std::list<TARGET> ret;
	assert(not separator.empty());

	if(not prefix.empty()){
		if(prefix.str()[0] != '^')
			prefix=boost::regex(std::string("^")+prefix.str(),prefix.flags());
		source=boost::regex_replace(source,prefix,"",boost::format_first_only|boost::match_default);
	}
	if(not postfix.empty()){
		if(postfix.str()[postfix.size()-1] != '$')
			postfix=boost::regex(postfix.str()+"$",postfix.flags());
		
		source=boost::regex_replace(source,postfix,"",boost::format_first_only|boost::match_default);
	}
	boost::sregex_token_iterator i=boost::make_regex_token_iterator(source, separator, -1);
	const boost::sregex_token_iterator token_end;

	while(i != token_end)
		ret.push_back(boost::lexical_cast<TARGET>(*i++));

	return ret;
}
/**
 * Simple tokenizer (regexp version).
 * Splits source into tokens and tries to lexically cast them to TARGET.
 * If that fails, boost::bad_lexical_cast is thrown.
 * Before the string is split up leading and rear separators will be cut.
 * \param source the source string to be split up
 * \param separator string to delimit the tokens
 * \returns a list of the casted tokens
 */
template<typename TARGET> std::list<TARGET> string2list(
	std::string source,
	const boost::regex separator=boost::regex("\\s+"))
{
	return string2list<TARGET>(source,separator,separator,separator);
}
															
/**
 * Simple tokenizer (char version).
 * Splits source into tokens and tries to lexically cast them to TARGET.
 * If that fails, boost::bad_lexical_cast is thrown.
 * Before the string is split up leading and rear separators will be cut.
 *
 * \param source the source string to be split up
 * \param separator string to delimit the tokens
 * Note that this still is put into an regualar expression, so "\\" will cause an runtime error. (Use "\\\\" instead)
 * \returns a list of the casted tokens
 */
template<typename TARGET> std::list<TARGET> string2list(std::string source,	const char separator[])
{
	return string2list<TARGET>(source,boost::regex(separator));
}

/**
 * Continously searches in an sorted list using the given less-than comparison.
 * It starts at current and increments it until the referenced value is not less than the compare-value anymore.
 * Than it returns.
 * \param current the current-position-iterator for the sorted list. 
 * This value is changed directly, so after the function returns is references the first entry of the list 
 * which does not compare less than compare or, if such a value does not exit in the list, it will be equal to end.
 * \param end the end of the list
 * \param compare the compare-value
 * \param compOp the comparison functor. It must provide "bool operator()(T,T)".
 * \returns true if the value current currently refers to is equal to compare
 */
template<typename ForwardIterator, typename T, typename CMP> bool
continousFind(ForwardIterator &current,const ForwardIterator end,const T& compare, CMP compOp)
{
	//find the first iterator which is does not compare less
	current=std::lower_bound( current, end, compare, compOp);
	if (current == end //if we're at the end
		or compOp(compare, *current) //or compare is less than that iterator
		)
			return false;//we didn't find a match
		else
			return true;//not(current <> compare) makes compare == current
}
/**
* Continously searches in an sorted list using std::less.
* It starts at current and increments it until the referenced value is not less than the compare-value anymore.
* Than it returns.
* \param current the current-position-iterator for the sorted list.
* This value is changed directly, so after the function returns is references the first entry of the list
* which does not compare less than compare or, if such a value does not exit in the list, it will be equal to end.
* \param end the end of the list
* \param compare the compare-value
* \returns true if the value current currently refers to is equal to compare
*/
template<typename ForwardIterator, typename T> bool
continousFind(ForwardIterator &current,const ForwardIterator end,const T& compare)
{
	return continousFind(current,end,compare,std::less<T>());
}

///Caseless less-compare for std::string
struct caselessStringLess{
	bool operator() (const std::string& a, const std::string& b) const
	{
		return (strcasecmp (a.c_str ( ), b.c_str ( )) < 0);
	}
};

/// @cond _hidden
	struct CoreLog{static const char* name(){return "Core";};enum {use = _ENABLE_CORE_LOG};};
	struct CoreDebug{static const char* name(){return "Core";};enum {use= _ENABLE_CORE_DEBUG};};
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
template<typename charT, typename traits, typename _Key, typename _Tp, typename _Compare, typename _Alloc >
basic_ostream<charT, traits>& operator<<(basic_ostream<charT, traits> &out,const map<_Key,_Tp,_Compare,_Alloc>& s)
{
	isis::util::write_list(s.begin(),s.end(),out,"\n","","");
	return out;
}

}
#endif //CORE_COMMON_HPP
