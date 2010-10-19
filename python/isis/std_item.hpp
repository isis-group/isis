/*
 * std_item.hpp
 *
 *  Created on: Oct 19, 2010
 *      Author: tuerke
 */


#ifndef STD_ITEM_HPP_
#define STD_ITEM_HPP_

void IndexError() { PyErr_SetString(PyExc_IndexError, "Index out of range"); }

template<class T>
class std_item {

public:
	typedef typename T::value_type V;

	static void add(T &x, V const& v)
	{
		x.push_back(v);
	}

	virtual void del(T &x, int i)
	{
		if( i<0 ) i+=x.size();
		if( i>=0 && i<x.size() ) x.erase(i);
		else IndexError();
	}

	virtual void set(T &x, int i, V const& v)
	{
		if( i<0 ) i+=x.size();
		if( i>=0 && i<x.size() ) x[i]=v;
		else IndexError();
	}

	virtual V& get(T const& x, int i)
	{
		if( i<0 ) i+=x.size();
		if( i>=0 && i<x.size() ) return x[i];
		IndexError();
	}
};

template<class T>
class std_list : public std_item<T>
{
public:
	typedef typename T::value_type V;

	virtual V& get(T &x, int i)
	{
		if( i<0 ) i+=x.size();
		if( i>=0 && i<x.size() ) {
			typename T::iterator iter = x.begin();
			std::advance(iter, i);
			return *iter;
		}
		IndexError();
	}
	virtual void set(T &x, int i, V const& v)
	{
		if( i<0 ) i+=x.size();
		if( i>=0 && i<x.size() ) {
			typename T::iterator iter = x.begin();
			std::advance(iter, i);
			*iter = v;
		}
		else IndexError();
	}
	virtual void del(T &x, int i)
	{
		if( i<0 ) i+=x.size();
		if( i>=0 && i<x.size() ) {
			typename T::iterator iter = x.begin();
			std::advance(iter, i);
			x.erase(iter);
		}
		else IndexError();
	}
};

#endif /* STD_ITEM_HPP_ */
