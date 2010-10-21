/*
 * _image.hpp
 *
 *  Created on: Oct 19, 2010
 *      Author: tuerke
 */

#ifndef IMAGE_HPP_
#define IMAGE_HPP_

#include "DataStorage/image.hpp"
#include "CoreUtils/vector.hpp"
#include <vector>

namespace isis
{
namespace python
{
class _Image : public isis::data::Image, boost::python::wrapper<isis::data::Image>
{
public:
	_Image ( PyObject *p) : self( p ) {}
	_Image ( PyObject *p, const isis::data::Image &base ) : isis::data::Image( base ), self( p ) {}

	float _voxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth )
	{
		switch( this->typeID()) {
		case data::TypePtr<int8_t>::staticID:
			return isis::data::Image::voxel<int8_t>( first, second, third, fourth );
			break;
		case data::TypePtr<u_int8_t>::staticID:
			return isis::data::Image::voxel<u_int8_t>( first, second, third, fourth );
			break;
		case data::TypePtr<int16_t>::staticID:
			return isis::data::Image::voxel<int16_t>( first, second, third, fourth );
			break;
		case data::TypePtr<u_int16_t>::staticID:
			return isis::data::Image::voxel<u_int16_t>( first, second, third, fourth );
			break;
		case data::TypePtr<int32_t>::staticID:
			return isis::data::Image::voxel<int32_t>( first, second, third, fourth );
			break;
		case data::TypePtr<u_int32_t>::staticID:
			return isis::data::Image::voxel<u_int32_t>( first, second, third, fourth );
			break;
		case data::TypePtr<float>::staticID:
			return isis::data::Image::voxel<float>( first, second, third, fourth );
			break;
		case data::TypePtr<double>::staticID:
			return isis::data::Image::voxel<double>( first, second, third, fourth );
			break;
		default:
			return 0;
		}
	}
	float _voxel( const isis::util::ivector4 &coord ) {
			return _voxel( coord[0], coord[1], coord[2], coord[3] );
		}


	bool _setVoxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const float &value )
	{
		switch( this->typeID()) {
		case data::TypePtr<int8_t>::staticID:
			isis::data::Image::voxel<int8_t>(first, second, third, fourth) = value;
			return true;
			break;
		case data::TypePtr<u_int8_t>::staticID:
			isis::data::Image::voxel<u_int8_t>(first, second, third, fourth) = value;
			return true;
			break;
		case data::TypePtr<int16_t>::staticID:
			isis::data::Image::voxel<int16_t>(first, second, third, fourth) = value;
			return true;
			break;
		case data::TypePtr<u_int16_t>::staticID:
			isis::data::Image::voxel<u_int16_t>(first, second, third, fourth) = value;
			return true;
			break;
		case data::TypePtr<int32_t>::staticID:
			isis::data::Image::voxel<int32_t>(first, second, third, fourth) = value;
			return true;
			break;
		case data::TypePtr<u_int32_t>::staticID:
			isis::data::Image::voxel<u_int32_t>(first, second, third, fourth) = value;
			return true;
			break;
		case data::TypePtr<float>::staticID:
			isis::data::Image::voxel<float>(first, second, third, fourth) = value;
			return true;
			break;
		case data::TypePtr<double>::staticID:
			isis::data::Image::voxel<double>(first, second, third, fourth) = value;
			return true;
			break;
		default:
			return false;
		}
	}

	bool _setVoxel( const isis::util::ivector4 &coord, const float &value ) {
		return _setVoxel( coord[0], coord[1], coord[2], coord[3], value);
	}

	std::list<isis::data::Chunk> _getChunkList( void ) {
		std::list<isis::data::Chunk> retChunkList;
		std::vector<boost::shared_ptr<isis::data::Chunk> > chunkList( this->getChunkList());
		BOOST_FOREACH(std::vector<boost::shared_ptr<isis::data::Chunk> >::reference ref, chunkList)
		{
			retChunkList.push_back(*ref);
		}
		return retChunkList;
	}

	const isis::util::ivector4 _sizeToVector( ) {
		return this->sizeToVector();
	}

	isis::data::Chunk _getChunk( const isis::util::ivector4& coord, bool copy_metadata ) {
		return this->getChunk(coord[0], coord[1], coord[2], coord[3], copy_metadata );
	}

	isis::data::Chunk _getChunkAs( const size_t& first, const size_t& second, const size_t &third, const size_t &fourth, const std::string &type ) {
		isis::data::Chunk ret = this->getChunk( first, second, third, fourth ); // get a cheap copy
		ret.makeOfTypeId( util::getTransposedTypeMap(true, true)[type] );
		return ret;
	}

	isis::data::Chunk _getChunkAs( const isis::util::ivector4& coord, const std::string& type ) {
		return _getChunkAs( coord[0], coord[1], coord[2], coord[3], type);
	}

	float _getMin( ) {
		float _min, _max;
		this->getMinMax(_min, _max);
		return _min;
	}
	float _getMax( ) {
		float _min, _max;
		this->getMinMax(_min, _max);
		return _max;
	}

	const std::string _getMainOrientation( ) {
		switch(this->getMainOrientation()) {
		case sagittal:
			return std::string("sagittal");
			break;
		case reversed_sagittal:
			return std::string("reversed_sagittal");
			break;
		case axial:
			return std::string("axial");
			break;
		case reversed_axial:
			return std::string("reversed_axial");
			break;
		case coronal:
			return std::string("coronal");
			break;
		case reversed_coronal:
			return std::string("reversed_coronal");
			break;
		}
	}

	void _transformCoords( boost::python::list matrix ) {
		std::vector< boost::python::list > rows;
		for (int i = 0; i < boost::python::len(matrix); ++i)
		{
			rows.push_back(boost::python::extract< boost::python::list >(matrix[i]));
		}
		boost::numeric::ublas::matrix<float> boostMatrix( 3, 3 );
		boostMatrix( 0, 0 ) = boost::python::extract<float> (rows[0][0]);
		boostMatrix( 0, 1 ) = boost::python::extract<float> (rows[0][1]);
		boostMatrix( 0, 2 ) = boost::python::extract<float> (rows[0][2]);
		boostMatrix( 1, 0 ) = boost::python::extract<float> (rows[1][0]);
		boostMatrix( 1, 1 ) = boost::python::extract<float> (rows[1][1]);
		boostMatrix( 1, 2 ) = boost::python::extract<float> (rows[1][2]);
		boostMatrix( 2, 0 ) = boost::python::extract<float> (rows[2][0]);
		boostMatrix( 2, 1 ) = boost::python::extract<float> (rows[2][1]);
		boostMatrix( 2, 2 ) = boost::python::extract<float> (rows[2][2]);
		this->transformCoords( boostMatrix );
 	}

	bool _makeOfTypeName( std::string type )
	{
		if( type[type.size() - 1] != '*' ) {
			type.append("*");
		}
		return this->makeOfTypeId( util::getTransposedTypeMap(true, true)[type] );
	}

	size_t _spliceDownTo( const std::string dim ) {
		if( boost::iequals(dim, "sliceDim")) {
			return this->spliceDownTo( isis::data::sliceDim );
		} else if ( boost::iequals(dim, "timeDim") ) {
			return this->spliceDownTo( isis::data::timeDim );
		} else if ( boost::iequals(dim, "readDim") ) {
			return this->spliceDownTo( isis::data::readDim );
		}else if ( boost::iequals(dim, "phaseDim") ) {
			return this->spliceDownTo( isis::data::phaseDim );
		} else {
			LOG( Runtime, error ) << dim << " is an unknown dimension. Possible dimensions are readDim, phaseDim, sliceDim and timeDim.";
			return 0;
		}
	}

private:
	PyObject *self;

};



class _ImageList : public std::list<isis::data::Image>
{


};

}}

#endif /* IMAGE_HPP_ */
