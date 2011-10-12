#include "_image.hpp"

namespace isis
{
namespace python
{
namespace data
{

_Image::_Image( PyObject *p )
	: boost::python::wrapper< Image >(), self( p )
{}

_Image::_Image( PyObject *p, const isis::data::Image &base )
	: Image( base ), boost::python::wrapper< Image >(), self( p )
{}






}
}
}