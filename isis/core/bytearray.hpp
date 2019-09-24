/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  <copyright holder> <email>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include "valuearray.hpp"

namespace isis{
namespace data{

class ByteArray : public ValueArray<uint8_t>
{
	typedef data::ValueArrayReference( *generator_type )( data::ByteArray &, size_t, size_t, bool );
	struct GeneratorMap: public std::map<unsigned short, generator_type> {
		GeneratorMap();
		template<class T> static data::ValueArrayReference generator( data::ByteArray &bytes, size_t offset, size_t len, bool swap_endianess ) {
			return bytes.at<T>( offset, len, swap_endianess );
		}
		struct proc {
			std::map<unsigned short, generator_type> *m_map;
			proc( std::map<unsigned short, generator_type> *map ): m_map( map ) {}
			template<class T> void operator()( const T & ) {
				m_map->insert( std::make_pair( ValueArray<T>::staticID(), &generator<T> ) );
			}
		};
	};
protected:
	bool writing=false; // for derived classes to flag memory as actually writing to (mapped) file (disables endian swap)

public:
	ByteArray()=default;
	ByteArray(const ByteArray &src)=default;
	ByteArray(const ValueArray<uint8_t> &src);
	/**
	 * Creates a ByteArray pointing to a newly allocated array of bytes.
	 * The array is zero-initialized.
	 * If the requested length is 0 no memory will be allocated and the pointer will be "empty".
	 * \param length amount of bytes in the new array
	 */
	ByteArray( size_t length );
	ByteArray( const std::shared_ptr<uint8_t> &ptr, size_t length );
	ByteArray( uint8_t *const ptr, size_t length );
	
	
	/**
	 * Get a ValueArray representing the data in the file.
	 * The resulting ValueArray will use a proxy deleter to keep track of the mapped file.
	 * So the file will be unmapped and closed if, and only if all ValueArray created by this function and the FilePtr are closed.
	 *
	 * If the FilePtr was opened writing, writing access to this ValueArray objects will result in writes to the file. Otherwise it will just write into memory.
	 *
	 * Note that there is no conversion done, just reinterpretation of the raw data in the file.
	 * \param offset the position in the file to start from (in bytes)
	 * \param len the requested length of the resulting ValueArray in elements (if that will go behind the end of the file, a warning will be issued).
	 * \param swap_endianess if endianess should be swapped when reading data file (ignored when used on files opened for writing)
	 */
	template<typename T> ValueArray<T> at( size_t offset, size_t len = 0, bool swap_endianess = false ) {
		util::checkType<T>();
		std::shared_ptr<T> ptr = std::static_pointer_cast<T>( getRawAddress( offset ) );

		if( len == 0 ) {
			len = ( getLength() - offset ) / sizeof( T );
			LOG_IF( ( getLength() - offset ) % sizeof( T ), Runtime, warning )
					<< "The remaining filesize " << getLength() - offset << " does not fit the bytesize of the requested type "
					<< util::Value<T>::staticName();
		}

		LOG_IF( len * sizeof( T ) > ( getLength() - offset ), Debug, error )
				<< "The requested length will be " << len * sizeof( T ) - ( getLength() - offset ) << " bytes behind the end of the file.";
		LOG_IF( writing && swap_endianess, Debug, warning )
				<< "Ignoring request to swap byte order for writing (the systems byte order is " << __BYTE_ORDER__ << " and that will be used)";

		if( writing || !swap_endianess ) { // if not endianess swapping was requested or T is not float (or if we are writing)
			return data::ValueArray<T>( ptr, len ); // return a cheap copy
		} else { // flip bytes into a new ValueArray
			LOG( Debug, verbose_info ) << "Byte swapping " <<  ValueArray<T>::staticName() << " for endianess";
			ValueArray<T> ret( len );
			data::endianSwapArray( ptr.get(), ptr.get() + std::min( len, getLength() / sizeof( T ) ), ret.begin() );
			return ret;
		}
	}
	/**
	 * Get a ValueArrayReference to a ValueArray of the requested type.
	 * The resulting ValueArray will use a proxy deleter to keep track of the mapped file.
	 * So the file will be unmapped and closed if, and only if all ValueArray created by this function and the FilePtr are closed.
	 *
	 * If the FilePtr was opened writing, writing access to this ValueArray objects will result in writes to the file.
	 * Otherwise it will just write into memory.
	 *
	 * If the FilePtr was opened reading and the assumed endianess of the file (see parameter) does not fit the endianess
	 * of the system an (endianess-converted) deep copy is created.
	 *
	 * \param ID the requested type (note that there is no conversion done, just reinterpretation of the raw data in the file)
	 * \param offset the position in the file to start from (in bytes)
	 * \param len the requested length of the resulting ValueArray in elements (if that will go behind the end of the file, a warning will be issued).
	 * \param swap_endianess if endianess should be swapped when reading data file (ignored when used on files opened for writing)
	 */
	data::ValueArrayReference atByID( unsigned short ID, size_t offset, size_t len = 0, bool swap_endianess = false );
	data::ValueArrayBase::ConstReference atByID( unsigned short ID, size_t offset, size_t len = 0, bool swap_endianess = false )const;
};
}
}

#endif // BYTEARRAY_H
