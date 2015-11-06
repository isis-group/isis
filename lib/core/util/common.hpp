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
#include <regex>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/assert.hpp>
#include <set>
#include <cmath>
#include "log.hpp"
#include "log_modules.hpp"
#include "../config.hpp"

#ifdef WIN32
#include <Windows.h>
#endif

namespace isis
{
namespace util
{

/**
* Continously searches in a sorted list using the given less-than comparison.
* It starts at current and increments it until the referenced value is not less than the compare-value anymore.
* Than it returns.
* \param current the current-position-iterator for the sorted list.
* This value is changed directly, so after the function returns is references the first entry of the list
* which does not compare less than compare or, if such a value does not exist in the list, it will be equal to end.
* \param end the end of the list
* \param compare the compare-value
* \param compOp the comparison functor. It must provide "bool operator()(T,T)".
* \returns true if the value current currently refers to is equal to compare
*/
template<typename ForwardIterator, typename T, typename CMP> bool
continousFind( ForwardIterator &current, const ForwardIterator end, const T &compare, const CMP &compOp )
{
	//find the first iterator which does not compare less
	current = std::lower_bound( current, end, compare, compOp );
	
	if ( current == end //if we're at the end
		|| compOp( compare, *current ) //or compare less than that iterator (eg. iterator greater than compare)
	)
		return false;//we didn't find a match
	else
		return true;//not(current <> compare) makes compare == current
}
	
std::string getLastSystemError();
boost::filesystem::path getRootPath(std::list< boost::filesystem::path > sources,bool sorted=false);

boost::filesystem::path pathReduce(std::set< boost::filesystem::path > sources);

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

	if ( start != end )
		o << *( start++ );

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
template<typename T, typename InputIterator> std::list<T> listToList( InputIterator start, InputIterator end )
{
	std::list<T> ret;

	for ( ; start != end; start++ )
		ret.push_back( boost::lexical_cast<T>( *start ) );

	return ret;
}

/**
 * Simple tokenizer (regexp version).
 * Splits source into tokens and tries to lexically cast them to TARGET.
 * If that fails, boost::bad_lexical_cast is thrown.
 * Before the string is split up leading and rear separators will be cut.
 * \param source the source string to be split up
 * \param separator regular expression to delimit the tokens (defaults to [^\\s,;])
 * \returns a list of the casted tokens
 */
template<typename TARGET, typename charT, typename traits> std::list<TARGET> stringToList(
	const std::basic_string<charT, traits> &source,
	const std::regex separator = std::regex( "[\\s,;]+",std::regex_constants::optimize ) )
{
	typedef typename std::basic_string<charT, traits>::const_iterator iterator_type;
	std::list<TARGET> ret;
	std::regex_token_iterator<iterator_type> i(source.begin(),source.end(), separator,-1 );
	static const std::regex_token_iterator<iterator_type> end=std::regex_token_iterator<iterator_type>();
	
	while ( i != end ) {
		ret.push_back( boost::lexical_cast<TARGET>( ( i++ )->str() ) );
	}
	return ret;
}
template<typename TARGET> std::list<TARGET> stringToList(
	const char source[],
	const std::regex separator = std::regex( "[\\s,;]+",std::regex_constants::optimize ) )
{
	return stringToList<TARGET>(std::string(source),separator);
}
/**
 * Generic tokenizer.
 * Splits source into tokens and tries to lexically cast them to TARGET.
 * If that fails, boost::bad_lexical_cast is thrown.
 * \param source the source string to be split up
 * \param separator string to delimit the tokens
 * \param prefix regular expression for text to be removed from the string before it is split up
 * ("^" is recommended to be there)
 * \param postfix regular expression for text to be removed from the string before it is split up
 * ("$" is recommended to be there)
 * \returns a list of the casted tokens
 */
template<typename TARGET, typename charT, typename traits> std::list<TARGET> stringToList(
	std::basic_string<charT, traits> source, const std::regex &separator,
	const std::regex prefix, const std::regex postfix )
{
	std::list<TARGET> ret;
	const std::string empty;
	source=std::regex_replace(source,prefix,empty);
	source=std::regex_replace(source,postfix,empty);
	
	return stringToList<TARGET>(source,separator);
}

/**
 * Very Simple tokenizer.
 * Splits source into tokens and tries to lexically cast them to TARGET.
 * If that fails, boost::bad_lexical_cast is thrown.
 * Leading and trailing seperators are ignored.
 *
 * \param source the source string to be split up
 * \param separator character to delimit the tokens
 * \returns a list of the casted tokens
 */
//@todo test
template<typename TARGET, typename charT, typename traits> std::list<TARGET>
stringToList( const std::basic_string<charT, traits> &source,  charT separator )
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
 * - Will return true if and only if the difference between two values is "small" compared to their magnitude.
 * - Will raise a compiler error when not used with floating point types.
 * @param a first value to compare with
 * @param b second value to compare with
 * @param scale scaling factor to determine which multiplies of the types floating point resolution (epsilon) should be considered equal
 * Eg. "1" means any difference less than the epsilon of the used floating point type will be considered equal.
 * If any of the values is greater than "1" the "allowed" difference will be bigger.
 * \returns \f[ |a-b| <= \varepsilon_T * \lceil |a|,|b|,|thresh| \rceil \f].
 */
template<typename T> bool fuzzyEqual( T a, T b, unsigned short scale = 10 )
{
	BOOST_MPL_ASSERT( ( boost::is_float<T> ) );

	static const T epsilon = std::numeric_limits<T>::epsilon(); // get the distange between 1 and the next representable value
	T bigger, smaller;

	a = std::abs( a );
	b = std::abs( b );

	if( a < b ) {
		bigger = b;
		smaller = a;
	} else {
		smaller = b;
		bigger = a;
	}

	if( smaller == 0 )
		return bigger < std::numeric_limits<T>::min() * scale;

	const T factor = 1 / smaller; // scale smaller to that value
	return ( bigger * factor ) <= ( 1 + epsilon * scale ); //scaled bigger should be between 1 and the next representable value
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
