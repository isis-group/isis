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


namespace isis
{
namespace data {template<typename TYPE> class ValueArray;}
namespace util
{
template<typename TYPE> class Value;

namespace _internal
{

class GenericValue
{
protected:
	GenericValue &operator=(const GenericValue &){}//prevent direct usage
	template<typename T> T &m_cast_to() {
		LOG_IF( getTypeID() != T::staticID, Debug, error ) << "using " << getTypeName() << " at " << this << " as " << T::staticName() << " aborting ...";
		assert( getTypeID() == T::staticID );
		//      return *dynamic_cast<T *>( this );// @todo Mac doesn't like that (http://www.cocoabuilder.com/archive/xcode/247376-rtti-dynamic-cast-across-shared-module-boundaries.html)
		return *( reinterpret_cast<T *>( this ) );
	}
	template<typename T> const T &m_cast_to()const {
		LOG_IF( getTypeID() != T::staticID, Debug, error ) << "using " << getTypeName() << " at " << this << " as " << T::staticName() << " aborting ...";
		assert( getTypeID() == T::staticID );
		//      return *dynamic_cast<const T *>( this );// @todo Mac doesn't like that (http://www.cocoabuilder.com/archive/xcode/247376-rtti-dynamic-cast-across-shared-module-boundaries.html)
		return *( reinterpret_cast<const T *>( this ) );
	}

public:
	/// \returns the value represented as text.
	virtual std::string toString( bool labeled = false )const = 0;

	/// \returns the name of its actual type
	virtual std::string getTypeName()const = 0;

	/// \returns the ID of its actual type
	virtual unsigned short getTypeID()const = 0;

	/// \returns true if the type is a floating point scalar
	virtual bool isFloat()const = 0;

	/// \returns true if the type is a integral scalar
	virtual bool isInteger()const = 0;

	/// \returns true if type of this and second are equal
	bool isSameType( const GenericValue &second )const;
	virtual ~GenericValue() {}
};

/**
 * Base class to store and handle references to Value and ValueArray objects.
 * The values are refernced as smart pointers to their base class.
 * So the references are counted and data are automatically deleted if necessary.
 * The usual dereferencing pointer interface ("*" and "->") is supported.
 * This class is designed as base class for specialisations, it should not be used directly.
 * Because of that, the contructors of this class are protected.
 */
template<typename TYPE_TYPE> class GenericReference: protected boost::scoped_ptr<TYPE_TYPE>
{
	template<typename TT> friend class data::ValueArray; //allow Value and ValueArray to use the protected contructor below
	template<typename TT> friend class Value;
protected:
	//dont use this directly
	GenericReference( TYPE_TYPE *t ): boost::scoped_ptr<TYPE_TYPE>( t ) {}
public:
	///reexport parts of scoped_ptr's interface
	TYPE_TYPE *operator->() const {return boost::scoped_ptr<TYPE_TYPE>::operator->();}
	TYPE_TYPE &operator*() const {return boost::scoped_ptr<TYPE_TYPE>::operator*();}
	///Default contructor. Creates an empty reference
	GenericReference() {}
	/**
	 * Copy constructor
	 * This operator creates a copy of the referenced Value-Object.
	 * So its NO cheap copy. (At least not if the copy-operator of the contained type is not cheap)
	 */
	GenericReference( const GenericReference &src ): boost::scoped_ptr<TYPE_TYPE>( NULL ) {
		operator=( src );
	}
	GenericReference( const TYPE_TYPE &src ): boost::scoped_ptr<TYPE_TYPE>( NULL ) {
		operator=( src );
	}
	/**
	 * Copy operator
	 * This operator replaces the current content by a copy of the content of src.
	 * So its NO cheap copy. (At least not if the copy-operator of the contained type is not cheap)
	 * If the source is empty the target will drop its content. Thus it will become empty as well.
	 * \returns reference to the (just changed) target
	 */
	GenericReference<TYPE_TYPE>& operator=( const GenericReference<TYPE_TYPE> &src ) {
		boost::scoped_ptr<TYPE_TYPE>::reset( src.isEmpty() ? 0 : src->clone() );
		return *this;
	}
	/**
	 * Copy operator
	 * This operator replaces the current content by a copy of src.
	 * \returns reference to the (just changed) target
	 */
	GenericReference<TYPE_TYPE>& operator=( const TYPE_TYPE &src ) {
		boost::scoped_ptr<TYPE_TYPE>::reset( src.clone() );
		return *this;
	}
	/// \returns true if "contained" type has no value (a.k.a. is undefined)
	bool isEmpty()const {
		return boost::scoped_ptr<TYPE_TYPE>::get() == NULL;
	}
	const std::string toString( bool label = false )const {
		if ( isEmpty() )
			return std::string( "\xd8" ); //ASCII code empty set
		else
			return this->get()->toString( label );
	}

	bool operator==(const GenericReference &cmp)const{return **this == *cmp;}
	bool operator==(const TYPE_TYPE &cmp)const{return **this == cmp;}
};

}
}
}
#endif // GENERIC_TYPE_HPP
