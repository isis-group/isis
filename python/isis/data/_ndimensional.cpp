#include "_ndimensional.hpp"
#include <boost/python/detail/wrapper_base.hpp>

namespace isis
{
namespace python
{
namespace data
{


_NDimensional::_NDimensional( PyObject *p )
	: self( p )
{}


_NDimensional::_NDimensional( PyObject *p, const isis::data::_internal::NDimensional< 4 >& base )
	: isis::data::_internal::NDimensional<4>( base ),
	  self( p )
{}

_NDimensional::_NDimensional( PyObject *p, const isis::util::ivector4 &dims )
	: self( p )
{
	init( dims );
}

namespace NDimensional
{

void _init( isis::data::_internal::NDimensional< 4 >& base, const isis::util::ivector4 &dims )
{
	base.init( dims );
}

void _init( isis::data::_internal::NDimensional< 4 >& base, const size_t &first, const size_t &second, const size_t &third, const size_t &fourth )
{
	base.init( isis::util::ivector4( first, second, third, fourth ) );
}

size_t _getLinearIndex( const isis::data::_internal::NDimensional< 4 >& base, const isis::util::ivector4 &dims )
{
	return base.getLinearIndex( dims );
}

size_t _getLinearIndex( const isis::data::_internal::NDimensional< 4 >& base, const size_t &first, const size_t &second, const size_t &third, const size_t &fourth )
{
	return base.getLinearIndex( isis::util::ivector4( first, second, third, fourth ) );
}

util::ivector4 _getCoordsFromLinIndex( const isis::data::_internal::NDimensional< 4 > &base , const size_t &linIndex )
{
	size_t coords[4];
	base.getCoordsFromLinIndex( linIndex, coords );
	return isis::util::ivector4( coords[0], coords[1], coords[2], coords[3] );
}

bool _isInRange( const isis::data::_internal::NDimensional< 4 >& base, const isis::util::ivector4 &dims )
{
	size_t coords[4];

	for( size_t i = 0; i < 4; i++ ) {
		coords[i] = dims[i];
	}

	return base.isInRange( coords );
}

bool _isInRange( const isis::data::_internal::NDimensional< 4 >& base, const size_t &first, const size_t &second, const size_t &third, const size_t &fourth )
{
	return _isInRange( base, isis::util::ivector4( first, second, third, fourth ) );
}

util::fvector4 _getFoV( const isis::data::_internal::NDimensional< 4 >& base, const isis::util::fvector4 &voxelSize, const isis::util::fvector4 &voxelGap )
{
	return base.getFoV( voxelSize, voxelGap );
}

std::string _getSizeAsString( const isis::data::_internal::NDimensional< 4 >& base )
{
	return base.getSizeAsString();
}

util::ivector4 _getSizeAsVector( const isis::data::_internal::NDimensional< 4 >& base )
{
	return base.getSizeAsVector();
}


} // end namespace NDimensional

}
}
}