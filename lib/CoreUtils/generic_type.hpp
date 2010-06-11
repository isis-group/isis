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

#ifndef GENERIC_TYPE_HPP
#define GENERIC_TYPE_HPP

#include <stdexcept>
#include <cstdlib>
#include "log.hpp"


namespace isis{
namespace data{template<typename TYPE> class TypePtr;}
namespace util{
template<typename TYPE> class Type;

namespace _internal{

class GenericType
{
protected:
	template<typename T> T &m_cast_to() throw( std::invalid_argument ) {
		if ( typeID() == T::staticID ) { // ok its exactly the same type - no fiddling necessary
			return *reinterpret_cast<T *>( this );
		} else {
			T *const ret = dynamic_cast<T * >( this ); //@todo have a look at http://lists.apple.com/archives/Xcode-users/2005/Dec/msg00061.html and http://www.mailinglistarchive.com/xcode-users@lists.apple.com/msg15790.html

			if ( ret == NULL ) {
				std::stringstream msg;
				msg << "cannot cast " << typeName() << " at " << this << " to " << T::staticName();
				throw( std::invalid_argument( msg.str() ) );
			}

			return *ret;
		}
	}
	template<typename T> const T &m_cast_to()const throw( std::invalid_argument ) {
		if ( typeID() == T::staticID ) { // ok its exactly the same type - no fiddling necessary
			return *reinterpret_cast<const T *>( this );
		} else {
			const T *const ret = dynamic_cast<const T * >( this ); //@todo have a look at http://lists.apple.com/archives/Xcode-users/2005/Dec/msg00061.html and http://www.mailinglistarchive.com/xcode-users@lists.apple.com/msg15790.html

			if ( ret == NULL ) {
				std::stringstream msg;
				msg << "cannot cast " << typeName() << " at " << this << " to " << T::staticName();
				throw( std::invalid_argument( msg.str() ) );
			}

			return *ret;
		}
	}

public:
	/// \returns true if the stored value is of type T.
	template<typename T> bool is()const {return is( typeid( T ) );}
	virtual bool is( const std::type_info &t )const = 0;

	/// \returns the value represented as text.
	virtual std::string toString( bool labeled = false )const = 0;

	/// \returns the name of its actual type
	virtual std::string typeName()const = 0;

	/// \returns the id of its actual type
	virtual unsigned short typeID()const = 0;

	/// \returns true if type of this and second are equal
	bool isSameType( const GenericType &second )const;
	virtual ~GenericType() {}
};

/**
* Base class to store and handle references to Type and TypePtr objects.
* The values are refernced as smart pointers to their base class.
* So the references are counted and data are automatically deleted if necessary.
* The usual dereferencing pointer interface ("*" and "->") is supported.
* This class is designed as base class for specialisations, it should not be used directly.
* Because of that, the contructors of this class are protected.
*/
template<typename TYPE_TYPE> class TypeReference: protected boost::scoped_ptr<TYPE_TYPE>
{
	template<typename TT> friend class data::TypePtr; //allow Type and TypePtr to use the protected contructor below
	template<typename TT> friend class Type;
protected:
	//dont use this directly
	TypeReference( TYPE_TYPE *t ): boost::scoped_ptr<TYPE_TYPE>( t ) {}
public:
	///reexport parts of scoped_ptr's interface
	TYPE_TYPE *operator->() const {return boost::scoped_ptr<TYPE_TYPE>::operator->();}
	TYPE_TYPE &operator*() const {return boost::scoped_ptr<TYPE_TYPE>::operator*();}
	///Default contructor. Creates an empty reference
	TypeReference() {}
	/**
	* Copy constructor
	* This operator creates a copy of the referenced Type-Object.
	* So its NO cheap copy. (At least not if the copy-operator contained type is not cheap)
	*/
	TypeReference( const TypeReference &src ) {
		operator=( src );
	}
	/**
	 * Copy operator
	 * This operator replaces the current content by a copy of the content of src.
	 * So its NO cheap copy. (At least not if the copy-operator contained type is not cheap)
	 * If the source is empty the target will drop its content. Thus it will become empty as well.
	 * \returns reference to the (just changed) target
	 */
	TypeReference<TYPE_TYPE>& operator=( const TypeReference<TYPE_TYPE> &src ) {
		boost::scoped_ptr<TYPE_TYPE>::reset( src.empty() ? 0 : src->clone() );
		return *this;
	}
	/**
	 * Copy operator
	 * This operator replaces the current content by a copy of src.
	 * \returns reference to the (just changed) target
	 */
	TypeReference<TYPE_TYPE>& operator=( const TYPE_TYPE &src ) {
		boost::scoped_ptr<TYPE_TYPE>::reset( src.clone() );
		return *this;
	}
	/// \returns true if "contained" type has no value (a.k.a. is undefined)
	bool empty()const {
		return boost::scoped_ptr<TYPE_TYPE>::get() == NULL;
	}
	const std::string toString( bool label = false )const {
		if ( empty() )
			return std::string( "\xd8" ); //ASCII code empty set
		else
			return this->get()->toString( label );
	}
};

}}}
#endif // GENERIC_TYPE_HPP
