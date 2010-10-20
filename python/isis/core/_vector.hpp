
#include "CoreUtils/vector.hpp"

using namespace isis::util;

namespace isis
{
namespace python
{


// helper class vector
template<class T>
class _Vector4 : public vector4<T>, boost::python::wrapper< vector4<T> >
{
public:
	_Vector4 () {}
	_Vector4( PyObject *p) :  self( p ) {}

	_Vector4( PyObject *p, const T &first, const T &second, const T &third, const T &fourth)
		: vector4<T>( first, second, third, fourth ),  self( p ) {}

	_Vector4( PyObject *p, const vector4<T> &v)
		: vector4<T>( v ), self( p ) {}


	void setItem( size_t elem, const T &value) {
		if( elem <= 3 && elem >= 0 ) {
			this->operator [](elem) = value;
		} else {
			std::cerr << "Index out of range!" << std::endl;
		}
	}

	T getItem( size_t elem ) {
		if( elem <= 3 && elem >= 0 ) {
			return this->operator [](elem);
		} else {
			std::cerr << "Index out of range!" << std::endl;
		}


	}



private:
	PyObject* self;

};

}
}


