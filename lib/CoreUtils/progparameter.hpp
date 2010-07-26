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

namespace isis
{
namespace util
{
/**
 * Container class for programm parameters.
 * This is used as container for programm parameters.
 * It is derived from PropertyValue, and thus can store any known type.
 * Additionally it has a description and a function to parse and store strings into its current type.
 */
class ProgParameter: public PropertyValue
{
	std::string m_description;
public:
	/**
	 * Default constructor.
	 * This creates an empty/typeless parameter container.
	 * Note that empty containers cannot parse a value because they dont have a type they could parse into.
	 * Thus parameters created using this must be set to any type/value before parse() is called.
	 */
	ProgParameter();
	/**
	 * Create a programm parameter using a initial value/type.
	 * \param ref the intial value/type the programm parameter should get
	 * (The value is used as default value if the parameter never gets to parse any other value)
	 * \param needed mark this parameter as needed
	 */
	ProgParameter( const ProgParameter &ref ): PropertyValue( static_cast<const PropertyValue &>( ref ) ) {}
	template<typename T> ProgParameter( const T &ref, bool needed = true ): PropertyValue( ref, needed ) {}
	/**
	 * Parse the given string as value into this parameter.
	 * The parsing is done by automatic type-conversion from std::string to the type of the parameter.
	 * Paramaters of type bool will be set true in any case
	 * Thus you cannot parse into empty parameters (they dont have a type).
	 */
	bool parse( const isis::util::Type< std::string >& props );
	/// \return the description string
	const std::string &description()const;
	/// set the description string
	void setDescription( const std::string &desc );
	/**
	 * Implicit cast to T.
	 * If the parameter does not contain T, a rutime error will be raised
	 */
	template<typename T> operator const T()const {
		LOG_IF( empty(), isis::CoreDebug, isis::error ) << "Program parameters must not be empty. Please set it to any value.";
		return get()->cast_to<T>();
	}
	/**
	 * special implicit conversion to be used in if/switch constructs
	 * The allowed conversions types are
	 * - boolean parameters will be will be converted to a equivalent true/false value
	 * - Selection will be converted to a equivalent integer value
	 * - int will be used as itself
	 *
	 * In any other case a runtime error will be raised.
	 */
	operator const int()const;
};

/**
 * Container for ProgParameter.
 * Handles instances of ProgParameter for every expected programm parameter and sets them by reading an argc/argv pair.
 */
class ParameterMap: public std::map<std::string, ProgParameter, _internal::caselessStringLess>
{
	struct neededP {
		bool operator()( const_reference ref )const {return ref.second.needed();}
	};
	struct notneededP {
		bool operator()( const_reference ref )const {return !ref.second.needed();}
	};
	bool parsed;
public:
	/// create an empty parameter map
	ParameterMap();
	/**
	 * parse a given argc/argv pair.
	 * Reads and tokenizes argv, detecs the parameter names (names are preceded by a "-") and sends the values
	 * (everything between one parameter name and the other) to the corresponding ProgParameter-objects to be parsed.
	 */
	bool parse( int argc, char **argv );
	/// \returns true, if non needed parameter is unset (was not parsed yet)
	bool isComplete()const;
	/// print name and description of every ProgParameter in the container
	void printAll()const;
	/// print name and description of every needed ProgParameter in the container
	void printNeeded()const;
};

}
}

namespace std
{
/// Streaming output for ProgParameter - classes
template<typename charT, typename traits> basic_ostream<charT, traits>&
operator<<( basic_ostream<charT, traits> &out, const isis::util::ProgParameter &s )
{
	const std::string &desc = s.description();

	if ( ! desc.empty() ) {
		out << desc << ", ";
	}

	LOG_IF( s.empty(), isis::CoreDebug, isis::error ) << "Program parameters must not be empty. Please set it to any value.";
	assert( !s.empty() );
	out << "default=\"" << s.toString( false ) << "\", type=" << s->typeName();

	if ( s.needed() )out << " (needed)";

	return out;
}
}
#endif // PROGPARAMETER_HPP
