#include "_ndimensional.hpp"

namespace isis {
namespace python {
namespace data {
	

_NDimensional::_NDimensional(PyObject* p)
 : self( p ) 
{}


_NDimensional::_NDimensional(PyObject* p, const isis::data::_internal::NDimensional< 4 >& base)
: isis::data::_internal::NDimensional<4>(base), 
 self(p)
{}

_NDimensional::_NDimensional(PyObject *p, const isis::util::ivector4& dims)
: self(p)
{
	init( dims );
}

	
void _NDimensional::_init(const isis::util::ivector4& dims)
{
	init( dims );
}
void _NDimensional::_init(const size_t& first, const size_t& second, const size_t& third, const size_t& fourth)
{
	init( isis::util::ivector4( first, second, third, fourth ) );
}

size_t _NDimensional::_getLinearIndex(const isis::util::ivector4& dims)
{
	return getLinearIndex( dims );
}

size_t _NDimensional::_getLinearIndex( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth )
{
	return getLinearIndex( isis::util::ivector4( first, second, third, fourth ) );
}

isis::util::ivector4 _NDimensional::_getCoordsFromLinIndex(const size_t &linIndex)
{
	size_t coords[4];
	getCoordsFromLinIndex( linIndex, coords );
	return isis::util::ivector4( coords[0], coords[1], coords[2], coords[3] );
}

bool _NDimensional::_isInRange(const isis::util::ivector4& dims)
{
	size_t coords[4];
	for( size_t i = 0; i < 4; i++ ) {
		coords[i] = dims[i];
	}
	return isInRange( coords );
}

bool _NDimensional::_isInRange( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth )
{
	return _isInRange( isis::util::ivector4( first, second, third, fourth ) );
}

std::string _NDimensional::_getSizeAsString()
{
	return getSizeAsString();
}

isis::util::ivector4 _NDimensional::_getSizeAsVector()
{
	return getSizeAsVector();
}

isis::util::fvector4 _NDimensional::_getFoV(const isis::util::fvector4& voxelSize, const isis::util::fvector4& voxelGap)
{
	return getFoV( voxelSize, voxelGap );
}


}}}