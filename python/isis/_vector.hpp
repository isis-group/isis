
#include "CoreUtils/vector.hpp"

using namespace isis::util;

namespace isis
{
namespace python
{

// helper class vector
template<class T>
class _Vector4 : public vector4<T>
{
public:
	_Vector4( PyObject *p) :  self( p ) {}

	_Vector4( PyObject *p, const T &first, const T &second, const T &third, const T &fourth)
		: vector4<T>( first, second, third, fourth ),  self( p ) {}

	_Vector4( PyObject *p, const vector4<T> &v)
		: vector4<T>( v ), self( p ) {}

	void setItem( size_t elem, const T &value) {
		this->operator [](elem) = value;
	}

	T getItem( size_t elem ) {
		return this->operator [](elem);
	}

private:
	PyObject* self;
};

}
}


