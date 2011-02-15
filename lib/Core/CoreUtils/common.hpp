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

#ifdef _MSC_VER
#define NOMINMAX 1
#endif

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

#ifdef _MSC_VER
#include <Windows.h>
typedef boost::int8_t   int8_t;
typedef boost::int16_t  int16_t;
typedef boost::int32_t  int32_t;
typedef boost::uint8_t  uint8_t;
typedef boost::uint16_t uint16_t;
typedef boost::uint32_t uint32_t;
typedef boost::int64_t  int64_t;
typedef boost::uint64_t uint64_t;
#else
#include <stdint.h>
#endif

namespace isis
{
#ifdef _MSC_VER
// dear microsoft, if I 'ld mean "boost::mpl::size_t", I 'ld write "boost::mpl::size_t" *argl*
typedef ::size_t size_t;
#endif
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
listToOStream( InputIterator start, InputIterator end,
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

// specialization to print char-list as number lists, not strings
//@todo check if this works for VC
template<typename _CharT, typename _Traits> std::basic_ostream<_CharT, _Traits> &
listToOStream( const unsigned char *start, const unsigned char *end,
			std::basic_ostream<_CharT, _Traits> &o,
			const std::string delim = ",",
			const std::string prefix = "{", const std::string suffix = "}" )
{
	o << prefix;

	if ( start != end ) {
		o << ( unsigned short )*start;
		start++;
	}

	for ( const unsigned char *i = start; i != end; i++ )
		o << delim << ( unsigned short )*i;

	o << suffix;
	return o;
}

/// use listToOStream to create a string from a list
template<class InputIterator> std::string listToString(
	InputIterator start, InputIterator end,
	const std::string delim = ",",
	const std::string prefix = "{", const std::string suffix = "}" )
{
	std::ostringstream ret;
	listToOStream( start, end, ret, delim, prefix, suffix );
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
	std::string source, const boost::regex &separator,
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

	while ( i != token_end ) {
		ret.push_back( boost::lexical_cast<TARGET>( ( i++ )->str() ) );
	}

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
	const boost::regex separator = boost::regex( "[[:space:]]" ) )
{
	return string2list<TARGET>( source, separator, separator, separator );
}

/**
 * Very Simple tokenizer.
 * Splits source into tokens and tries to lexically cast them to TARGET.
 * If that fails, boost::bad_lexical_cast is thrown.
 * Leading and trailing seperators are ignored.
 *
 * In contrast to the versions based on regular expressions, this can handle any basic_string as input.
 *
 * \param source the source string to be split up
 * \param separator string to delimit the tokens
 * \returns a list of the casted tokens
 */
//@todo test
template<typename TARGET, typename charT, typename traits> std::list<TARGET>
string2list( const std::basic_string<charT, traits> &source,  charT separator )
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
 * Fuzzy comparison between floating point values.
 * Will raise a compiler error when not used with floating point types.
 * @param a first value to compare with
 * @param b second value to compare with
 * @param boost a scaling factor to regulate the "fuzzyness" of the operation. A higher
 * value will result in a more fuzzy check. Normally one would use multiple of 10.
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

typedef CoreDebug Debug;
typedef CoreLog Runtime;

/**
 * Set logging level for the namespace util.
 * This logging level will be used for every LOG(Debug,...) and LOG(Runtime,...) within the util namespace.
 * This is affected by by the _ENABLE_LOG and _ENABLE_DEBUG settings of the current compile and won't have an
 * effect on the Debug or Runtime logging if the corresponding define is set to "0".
 * So if you compile with "-D_ENABLE_DEBUG=0" against a library which (for example) was comiled with "-D_ENABLE_DEBUG=1",
 * you won't be able to change the logging level of the debug messages of these library.
 */
template<typename HANDLE> void enableLog( LogLevel level )
{
	ENABLE_LOG( CoreLog, HANDLE, level );
	ENABLE_LOG( CoreDebug, HANDLE, level );
}
}//util

/**
 * Set logging level for the namespaces util,data and image_io.
 * This logging level will be used for every LOG(Debug,...) and LOG(Runtime,...) within the image_io namespace.
 * This is affected by by the _ENABLE_LOG and _ENABLE_DEBUG settings of the current compile and won't have an
 * effect on the Debug or Runtime logging if the corresponding define is set to "0".
 * So if you compile with "-D_ENABLE_DEBUG=0" against a library which (for example) was comiled with "-D_ENABLE_DEBUG=1",
 * you won't be able to change the logging level of the debug messages of these library.
 */
template<typename HANDLE> void enableLogGlobal( LogLevel level )
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
template<typename charT, typename _FIRST, typename _SECOND > basic_ostream<charT, std::char_traits<charT> >&
operator<<( basic_ostream<charT, std::char_traits<charT> > &out, const pair<_FIRST, _SECOND> &s )
{
	return out << s.first << ":" << s.second;
}

/// Streaming output for std::map
template<typename charT, typename _Key, typename _Tp, typename _Compare, typename _Alloc >
basic_ostream<charT, std::char_traits<charT> >& operator<<( basic_ostream<charT, std::char_traits<charT> > &out, const map<_Key, _Tp, _Compare, _Alloc>& s )
{
	isis::util::listToOStream( s.begin(), s.end(), out, "\n", "", "" );
	return out;
}

/// Formatted streaming output for std::map\<string,...\>
template<typename charT, typename _Tp, typename _Compare, typename _Alloc >
basic_ostream<charT, std::char_traits<charT> >& operator<<( basic_ostream<charT, std::char_traits<charT> > &out, const map<std::string, _Tp, _Compare, _Alloc>& s )
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
template<typename charT, typename _Tp, typename _Alloc >
basic_ostream<charT, std::char_traits<charT> >& operator<<( basic_ostream<charT, std::char_traits<charT> > &out, const list<_Tp, _Alloc>& s )
{
	isis::util::listToOStream( s.begin(), s.end(), out );
	return out;
}
///Streaming output for std::set
template<typename charT, typename _Tp, typename _Alloc >
basic_ostream<charT, std::char_traits<charT> >& operator<<( basic_ostream<charT, std::char_traits<charT> > &out, const set<_Tp, _Alloc>& s )
{
	isis::util::listToOStream( s.begin(), s.end(), out );
	return out;
}

}
#endif //CORE_COMMON_HPP
