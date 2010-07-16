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
#include <boost/foreach.hpp>

namespace isis
{
namespace util
{

/**
 * Enum-like class for string based selections.
 * The options are NOT case sensitive.
 */
class Selection
{
	typedef std::map<std::string, unsigned short, _internal::caselessStringLess> map_type;
	map_type ent_map;
	int m_set;
public:
	/**
	 * Default constructor.
	 * Creates a selection with the given options.
	 * \param entries comma separated list of the options as a string
	 */
	Selection( const char *entries );
	/**
	 * Default constructor.
	 * Creates a selection with the given options.
	 * \param entries comma separated list of the options as a string
	 */
	template<typename T> Selection( const std::map<T,std::string> &map);
	/// Fallback contructor to enable creation of empty selections
	Selection();
	/**
	 * Set the selection to the given type.
	 * If the given option does not exist, a runtime error will be send and the selection won't be set.
	 * \param entry the option the selection should be set to.
	 * \returns true if the option was set, false otherwise.
	 */
	bool set( const char *entry );
	/**
	 * Implicit cast to int.
	 * The numbers correspont to the order the options where given at the creation of the selection (first option -> 1, second option -> 2 ...)
	 * \returns number corresponding the currently set option or "0" if none is set
	 */
	operator const int()const;
	/**
	 * Implicit cast to string.
	 * \returns the currently set option or "<<NOT_SET>>" if none is set
	 */
	operator const std::string()const;
	/**
	 * Common comparison.
	 * \returns true if both selection have the same options and are currently set to the the option. False otherwise.
	 */
	bool operator==( const Selection &ref )const;
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
	/// \returns a list of all options
	std::list<std::string> getEntries()const;
};

template<typename T> Selection::Selection(const std::map< T, std::string >& map)
{
	for(typename std::map< T, std::string >::const_iterator i=map.begin();i!=map.end();i++){
		const map_type::value_type pair(i->second,i->first);
		if(!ent_map.insert(pair).second) {
			LOG(Debug,error)<< "Entry " << util::MSubject(pair) << " could not be inserted";
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
