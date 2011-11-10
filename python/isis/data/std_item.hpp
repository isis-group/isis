/*
 * std_item.hpp
 *
 *  Created on: Oct 19, 2010
 *      Author: tuerke
 */


#ifndef STD_ITEM_HPP_
#define STD_ITEM_HPP_


namespace isis
{
namespace python
{
void IndexError()
{
	PyErr_SetString( PyExc_IndexError, "Index out of range" );
	LOG( Runtime, error ) << "Index out of range";
}


template<class T>
class std_list
{
public:
	typedef typename T::value_type V;

	static void add( T &x, V const &v ) {
		x.push_back( v );
	}

	static V &get( T &x, size_t i ) {
		if( i >= x.size() ) {
			IndexError();
		}

		typename T::iterator iter = x.begin();
		std::advance( iter, i );
		return *iter;
	}
	static void set( T &x, size_t i, V const &v ) {
		if( i < x.size() ) {
			typename T::iterator iter = x.begin();
			std::advance( iter, i );
			*iter = v;
		} else IndexError();
	}
	static void del( T &x, size_t i ) {
		if( i < x.size() ) {
			typename T::iterator iter = x.begin();
			std::advance( iter, i );
			x.erase( iter );
		} else IndexError();
	}
};
}
}
#endif /* STD_ITEM_HPP_ */
