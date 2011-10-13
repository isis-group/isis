#ifndef _NDIMENSIONAL_HPP
#define _NDIMENSIONAL_HPP


#include <boost/python.hpp>
#include "DataStorage/ndimensional.hpp"

namespace isis {
namespace python {
namespace data {
	
class _NDimensional : public isis::data::_internal::NDimensional<4>, boost::python::wrapper< isis::data::_internal::NDimensional<4> >
{
public:
	_NDimensional( PyObject *p );
	_NDimensional( PyObject *p, const isis::data::_internal::NDimensional< 4 >& base);
	_NDimensional( PyObject *p, const isis::util::ivector4 &dims);
	
	void _init ( const isis::util::ivector4 &dims );
	void _init ( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth );
	
	size_t _getLinearIndex( const isis::util::ivector4 &dims );
	size_t _getLinearIndex( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth );
	
	isis::util::ivector4 _getCoordsFromLinIndex( const size_t &linIndex ) ;
	
	bool _isInRange( const isis::util::ivector4 &dims );
	bool _isInRange( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth );
	
	std::string _getSizeAsString();
	isis::util::ivector4 _getSizeAsVector();
	isis::util::fvector4 _getFoV( const isis::util::fvector4 &voxelSize, const isis::util::fvector4 &voxelGap );
	
private:
	PyObject *self;
};
	
}}} // end namespace


#endif