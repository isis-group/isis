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
#include <boost/mpl/assert.hpp>
#include <set>
#include <cmath>
#include "log.hpp"
#include "log_modules.hpp"
#include <boost/algorithm/string/case_conv.hpp>

#ifdef _MSC_VER
typedef boost::int8_t   int8_t;
typedef boost::int16_t  int16_t;
typedef boost::int32_t  int32_t;
typedef boost::uint8_t  uint8_t;
typedef boost::uint16_t uint16_t;
typedef boost::uint32_t uint32_t;
typedef boost::int64_t  int64_t;
typedef boost::uint64_t uint64_t;
#define DISABLE_WARN(NUM)   #pragma warning(disable:4244)
#define DISABLE_WARN_LINE(NUM) #pragma warning(suppress:4996)
#else
#include <stdint.h>
#define DISABLE_WARN(NUM)
#define DISABLE_WARN_LINE(NUM)
#endif

namespace isis
{
namespace util
{

/**
Write a list of elements to a std::basic_ostream
\param start starting iterator of input
\param end end iterator of input
\param o the output stream to write into
\param delim delimiter used to seperate the elements (default: " ")
\param prefix will be send to the stream as start (default: "")
\param suffix will be send to the stream at the end (default: "")
*/
template<class InputIterator, typename _CharT, typename _Traits> std::basic_ostream<_CharT, _Traits> &
write_list( InputIterator start, InputIterator end,
			std::basic_ostream<_CharT, _Traits> &o,
			const std::string delim = ",",
			const std::string prefix = "{", const std::string suffix = "}" )
{
	o << prefix;

	if ( start != end ) {
		o << *start;
		start++;
	}

	for ( InputIterator i = start; i != end; i++ )
		o << delim << *i;

	o << suffix;
	return o;
}

/// use write_list to create a string from a list
template<class InputIterator> std::string list2string(
	InputIterator start, InputIterator end,
	const std::string delim = ",",
	const std::string prefix = "{", const std::string suffix = "}" )
{
	std::ostringstream ret;
	write_list( start, end, ret, delim, prefix, suffix );
	return ret.str();
}
/// do lexical_cast\<T\> on the elements of a list and return them
template<typename T, typename InputIterator> std::list<T> list2list( InputIterator start, InputIterator end )
{
	std::list<T> ret;

	for ( ; start != end; start++ )
		ret.push_back( boost::lexical_cast<T>( *start ) );

	return ret;
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
	std::string source, const boost::regex separator,
	boost::regex prefix, boost::regex postfix )
{
	std::list<TARGET> ret;
	assert( ! separator.empty() );

	if ( ! prefix.empty() ) {
		if ( prefix.str()[0] != '^' )
			prefix = boost::regex( std::string( "^" ) + prefix.str(), prefix.flags() );

		source = boost::regex_replace( source, prefix, "", boost::format_first_only | boost::match_default );
	}

	if ( ! postfix.empty() ) {
		if ( postfix.str()[postfix.size()-1] != '$' )
			postfix = boost::regex( postfix.str() + "$", postfix.flags() );

		source = boost::regex_replace( source, postfix, "", boost::format_first_only | boost::match_default );
	}

	boost::sregex_token_iterator i = boost::make_regex_token_iterator( source, separator, -1 );
	const boost::sregex_token_iterator token_end;

	while ( i != token_end )
		ret.push_back( boost::lexical_cast<TARGET>( *i++ ) );

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
	const boost::regex separator = boost::regex( "\\s+" ) )
{
	return string2list<TARGET>( source, separator, separator, separator );
}

/**
 * Very Simple tokenizer.
 * Splits source into tokens and tries to lexically cast them to TARGET.
 * If that fails, boost::bad_lexical_cast is thrown.
 * Leading and trailing seperators are ignored.
 *
 * \param source the source string to be split up
 * \param separator string to delimit the tokens
 * \returns a list of the casted tokens
 */
//@todo test
template<typename TARGET> std::list<TARGET> string2list( const std::string &source,  char separator )
{
	std::list<TARGET> ret;

	for (
		size_t next = source.find_first_not_of( separator );
		next != std::string::npos;
		next = source.find_first_not_of( separator, next )
	) {
		const size_t start = next;
		next = source.find( separator, start );
		ret.push_back( boost::lexical_cast<TARGET>( source.substr( start, next - start ) ) );
	}

	return ret;
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
continousFind( ForwardIterator &current, const ForwardIterator end, const T &compare, CMP compOp )
{
	//find the first iterator which is does not compare less
	current = std::lower_bound( current, end, compare, compOp );

	if ( current == end //if we're at the end
		 || compOp( compare, *current ) //or compare is less than that iterator
	   )
		return false;//we didn't find a match
	else
		return true;//not(current <> compare) makes compare == current
}

/**
 * Fuzzy comparison between floating point values.
 * Will raise a compiler error when not used with floating point types.
 * \returns true if the difference between the two values is significantly small compared to their values.
 */
template<typename T> bool fuzzyEqual( T a, T b, unsigned short boost = 1 )
{
	BOOST_MPL_ASSERT( ( boost::is_float<T> ) );
	const T epsilon = std::numeric_limits<T>::epsilon();

	if ( a == b )
		return true;

	if( a < 0 && b > 0 ) {
		b += -a;
		a = 0;
	} else if( a > 0 && b < 0 ) {
		a += -b;
		b = 0;
	}

	if( a == 0 && std::fabs( b ) < epsilon * boost )
		return true;

	if( b == 0 && std::fabs( a ) < epsilon * boost )
		return true;

	const T dist_fac = ( std::max( a, b ) / std::min( a, b ) ) - 1;
	return  dist_fac <= epsilon * boost;
}

/// @cond _internal
namespace _internal
{
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
continousFind( ForwardIterator &current, const ForwardIterator end, const T &compare )
{
	return continousFind( current, end, compare, std::less<T>() );
}

///Caseless less-compare for std::string
struct caselessStringLess {
	bool operator() ( const std::string &a, const std::string &b ) const {
		return strcasecmp( a.c_str(), b.c_str() ) < 0;
		//      return boost::algorithm::to_lower_copy(a) < boost::algorithm::to_lower_copy(b);
		//@todo for WinXP maybe look at http://www.winehq.org/pipermail/wine-patches/2004-August/012083.html
	}
};
}
/// @endcond
typedef CoreDebug Debug;
typedef CoreLog Runtime;

template<typename HANDLE> void enable_log( LogLevel level )
{
	ENABLE_LOG( CoreLog, HANDLE, level );
	ENABLE_LOG( CoreDebug, HANDLE, level );
}
}//util
template<typename HANDLE> void enable_log_global( LogLevel level )
{
	ENABLE_LOG( CoreLog, HANDLE, level );
	ENABLE_LOG( CoreDebug, HANDLE, level );
	ENABLE_LOG( ImageIoLog, HANDLE, level );
	ENABLE_LOG( ImageIoDebug, HANDLE, level );
	ENABLE_LOG( DataLog, HANDLE, level );
	ENABLE_LOG( DataDebug, HANDLE, level );
}
}//isis

namespace std
{
/// Streaming output for std::pair
template<typename charT, typename traits, typename _FIRST, typename _SECOND > basic_ostream<charT, traits>&
operator<<( basic_ostream<charT, traits> &out, const pair<_FIRST, _SECOND> &s )
{
	return out << s.first << ":" << s.second;
}

/// Streaming output for std::map
template<typename charT, typename traits, typename _Key, typename _Tp, typename _Compare, typename _Alloc >
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const map<_Key, _Tp, _Compare, _Alloc>& s )
{
	isis::util::write_list( s.begin(), s.end(), out, "\n", "", "" );
	return out;
}

/// Formatted streaming output for std::map\<string,...\>
template<typename charT, typename traits, typename _Tp, typename _Compare, typename _Alloc >
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const map<std::string, _Tp, _Compare, _Alloc>& s )
{
	size_t key_len = 0;
	typedef typename map<std::string, _Tp, _Compare, _Alloc>::const_iterator m_iterator;

	for ( m_iterator i = s.begin(); i != s.end(); i++ )
		if ( key_len < i->first.length() )
			key_len = i->first.length();

	for ( m_iterator i = s.begin(); i != s.end(); ) {
		out << make_pair( i->first + std::string( key_len - i->first.length(), ' ' ), i->second );

		if ( ++i != s.end() )
			out << std::endl;
	}

	return out;
}

///Streaming output for std::list
template<typename charT, typename traits, typename _Tp, typename _Alloc >
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const list<_Tp, _Alloc>& s )
{
	isis::util::write_list( s.begin(), s.end(), out );
	return out;
}
///Streaming output for std::set
template<typename charT, typename traits, typename _Tp, typename _Alloc >
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const set<_Tp, _Alloc>& s )
{
	isis::util::write_list( s.begin(), s.end(), out );
	return out;
}

}
#endif //CORE_COMMON_HPP
