#include "_ioapplication.hpp"


namespace isis
{

namespace python
{

namespace data
{

_IOApplication::_IOApplication( PyObject *p, const char name[], const bool &input, const bool &output )
	: isis::data::IOApplication( name, input, output ),
	  boost::python::wrapper<isis::data::IOApplication>(),
	  self( p )
{
}


_IOApplication::_IOApplication( PyObject *p, const isis::data::IOApplication &base )
	: isis::data::IOApplication( base ),
	  boost::python::wrapper<isis::data::IOApplication>(),
	  self( p )
{
}

isis::data::Image _IOApplication::_fetchImageAs( isis::python::data::image_types type )
{
	isis::data::Image tmpImage = fetchImage();
	tmpImage.convertToType( static_cast<unsigned short>( type ) );
	return tmpImage;


}


}
}
} //end namespace