/*
 Copyright (C) 2010  reimer@cbs.mpg.de

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


#ifndef SELECTION_HPP_INCLUDED
#define SELECTION_HPP_INCLUDED
#include <map>
#include "common.hpp"
#include "istring.hpp"

namespace isis
{
namespace util
{

/**
 * Here, a Selection is one of our types (see types.hpp) and
 * meant as an enumeration of "things" described by strings,
 * e.g. properties for easy acces of several properties from a PopertyMap.
 * It's using isis::util::istring, therefore
 * the options are CASE INSENSITIVE.
 */
class Selection
{
	typedef std::map<util::istring, unsigned short> MapType;
	MapType ent_map;
	int m_set;
	template<typename OPERATION> bool comp_op(const Selection &ref,const OPERATION &op)const{
		LOG_IF(ent_map != ref.ent_map,Debug,error) << "Comparing different Selection sets, result will be \"false\"";
		LOG_IF(!(m_set && ref.m_set),Debug,error) << "Comparing unset Selection, result will be \"false\"";
		return (m_set && ref.m_set) && ent_map == ref.ent_map && op(m_set, ref.m_set);
	}
public:
	/**
	 * Default constructor.
	 * Creates a selection with the given options.
	 * \param entries comma separated list of the options as a string
	 * \param init_val the string which should be selected after initialisation (must be one from entries)
	 * \warning this is really only <b>comma</b> separated, so write "first,second,and,so,on" and not "first, second, and, so, on"
	 */
	Selection( const char *entries, const char *init_val = "" );
	/**
	 * Default constructor.
	 * Creates a selection from a number-option map.
	 * \param map a map which maps specific numbers (must not be 0) to options to be used
	 */
	template<typename T> Selection( const std::map<T, std::string> &map );
	/// Fallback contructor to enable creation of empty selections
	Selection();
	/**
	 * Set the selection to the given type.
	 * If the given option does not exist, a runtime error will be send and the selection won't be set.
	 * \param entry the option the selection should be set to.
	 * \returns true if the option was set, false otherwise.
	 */
	bool set( const char *entry );
	bool set( unsigned short entry );
	/**
	 * Implicit cast to int.
	 * The numbers correspont to the order the options where given at the creation of the selection (first option -> 1, second option -> 2 ...)
	 * \returns number corresponding the currently set option or "0" if none is set
	 */
	operator int()const;
	/**
	 * Implicit cast to string.
	 * \returns the currently set option or "<<NOT_SET>>" if none is set
	 */
	operator std::string()const;
	/**
	 * Implicit cast to istring.
	 * \returns the currently set option or "<<NOT_SET>>" if none is set
	 */
	operator util::istring()const;
	/**
	 * Common comparison.
	 * \returns true if both selection have the same options and are currently set to the the option. False otherwise.
	 */
	bool operator==( const Selection &ref )const;
	bool operator>( const Selection &ref )const;
	bool operator<( const Selection &ref )const;
	/**
	 * String comparison.
	 * \returns true if the currently set option is non-case-sensitive equal to the given string. False otherwise.
	 */
	bool operator==( const char ref[] )const;
	/**
	 * Number comparison.
	 * \returns true if the number corresponding the currently set option is equal to the given number. False otherwise.
	 */
	bool operator==( const int ref )const;
	bool operator>( const int ref )const;
	bool operator<( const int ref )const;

	/// \returns a list of all options
	std::list<util::istring> getEntries()const;
};

template<typename T> Selection::Selection( const std::map< T, std::string >& map ): m_set( 0 )
{
	for( typename std::map< T, std::string >::const_iterator i = map.begin(); i != map.end(); i++ ) {
		const MapType::value_type pair(
			util::istring( i->second.begin(), i->second.end() ),
			i->first
		);
		assert( pair.second != 0 ); // 0 is reserved for <<NOT_SET>>

		if( !ent_map.insert( pair ).second ) {
			LOG( Debug, error ) << "Entry " << util::MSubject( pair ) << " could not be inserted";
		}
	}
}

}
}

namespace std
{
/// Streaming output for selections.
template<typename charT, typename traits>
basic_ostream<charT, traits> &operator<<( basic_ostream<charT, traits> &out, const isis::util::Selection &s )
{
	return out << ( std::string )s;
}
}
#endif //SELECTION_HPP_INCLUDED
