#ifndef _NDIMENSIONAL_HPP
#define _NDIMENSIONAL_HPP


#include <boost/python.hpp>
#include "DataStorage/ndimensional.hpp"

namespace isis {
namespace python {
namespace data {
	
class _NDimensional : public isis::data::_internal::NDimensional<4>, public boost::python::wrapper< isis::data::_internal::NDimensional<4> >
{
public:
	_NDimensional( PyObject *p );
	_NDimensional( PyObject *p, const isis::data::_internal::NDimensional< 4 >& base);
	_NDimensional( PyObject *p, const isis::util::ivector4 &dims);
	
private:
	PyObject *self;
};
namespace NDimensional {

void _init( isis::data::_internal::NDimensional<4> &base, const isis::util::ivector4 &dims );
void _init( isis::data::_internal::NDimensional<4> &base, const size_t &first, const size_t &second, const size_t &third, const size_t &fourth );

size_t _getLinearIndex( const isis::data::_internal::NDimensional<4> &base, const isis::util::ivector4 &dims);
size_t _getLinearIndex( const isis::data::_internal::NDimensional<4> &base, const size_t &first, const size_t &second, const size_t &third, const size_t &fourth);
	
isis::util::ivector4 _getCoordsFromLinIndex( const isis::data::_internal::NDimensional<4> &base, const size_t &linIndex );

bool _isInRange( const isis::data::_internal::NDimensional<4> &base, const isis::util::ivector4 &dims );
bool _isInRange( const isis::data::_internal::NDimensional<4> &base, const size_t &first, const size_t &second, const size_t &third, const size_t &fourth );

std::string _getSizeAsString( const isis::data::_internal::NDimensional<4> &base );
isis::util::ivector4 _getSizeAsVector( const isis::data::_internal::NDimensional<4> &base );
isis::util::fvector4 _getFoV( const isis::data::_internal::NDimensional<4> &base, const isis::util::fvector4 &voxelSize, const isis::util::fvector4 &voxelGap );
} // end namespace NDimensional
}}} // end namespace


#endif