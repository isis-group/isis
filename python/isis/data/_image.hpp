/*
 * _image.hpp
 *
 *  Created on: Oct 19, 2010
 *      Author: tuerke
 */

#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include "DataStorage/image.hpp"
#include "CoreUtils/vector.hpp"
#include <vector>

#include "core/_propmap.hpp"

using namespace isis::data;

namespace isis
{
namespace python
{
namespace data 
{
	
class _Image : public Image, boost::python::wrapper<Image>
{

public:

	_Image ( PyObject *p ) : boost::python::wrapper< Image >(), self( p ) {}
	_Image ( PyObject *p, const Image &base ) : Image( base ), boost::python::wrapper< Image >(), self( p ) {}

	float _voxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth ) {
		Chunk ch = this->getChunk( first, second, third, fourth, false );

		switch( ch.getTypeID() ) {
		case ValuePtr<int8_t>::staticID:
			return Image::voxel<int8_t>( first, second, third, fourth );
			break;
		case ValuePtr<uint8_t>::staticID:
			return Image::voxel<uint8_t>( first, second, third, fourth );
			break;
		case ValuePtr<int16_t>::staticID:
			return Image::voxel<int16_t>( first, second, third, fourth );
			break;
		case ValuePtr<uint16_t>::staticID:
			return Image::voxel<uint16_t>( first, second, third, fourth );
			break;
		case ValuePtr<int32_t>::staticID:
			return Image::voxel<int32_t>( first, second, third, fourth );
			break;
		case ValuePtr<uint32_t>::staticID:
			return Image::voxel<uint32_t>( first, second, third, fourth );
			break;
		case ValuePtr<float>::staticID:
			return Image::voxel<float>( first, second, third, fourth );
			break;
		case ValuePtr<double>::staticID:
			return Image::voxel<double>( first, second, third, fourth );
			break;
		default:
			return 0;
		}
	}
	float _voxel( const isis::util::ivector4 &coord ) {
		return _voxel( coord[0], coord[1], coord[2], coord[3] );
	}


	bool _setVoxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const float &value ) {
		Chunk ch = this->getChunk( first, second, third, fourth, false );

		switch( ch.getTypeID() ) {
		case ValuePtr<int8_t>::staticID:
			Image::voxel<int8_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<uint8_t>::staticID:
			Image::voxel<uint8_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<int16_t>::staticID:
			Image::voxel<int16_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<uint16_t>::staticID:
			Image::voxel<uint16_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<int32_t>::staticID:
			Image::voxel<int32_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<uint32_t>::staticID:
			Image::voxel<uint32_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<float>::staticID:
			Image::voxel<float>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<double>::staticID:
			Image::voxel<double>( first, second, third, fourth ) = value;
			return true;
			break;
		default:
			return false;
		}
	}

	bool _setVoxel( const isis::util::ivector4 &coord, const float &value ) {
		return _setVoxel( coord[0], coord[1], coord[2], coord[3], value );
	}

	std::list<Chunk> _getChunksAsVector( void ) {
		std::list<Chunk> retChunkList;
		std::vector<Chunk>  chunkList( this->copyChunksToVector() );
		BOOST_FOREACH( std::vector<Chunk> ::reference ref, chunkList ) {
			retChunkList.push_back( ref );
		}
		return retChunkList;
	}

	const isis::util::ivector4 _getSizeAsVector( ) {
		return this->getSizeAsVector();
	}

	Chunk _getChunk( const isis::util::ivector4 &coord, bool copy_metadata ) {
		return this->getChunk( coord[0], coord[1], coord[2], coord[3], copy_metadata );
	}

	Chunk _getChunkAs( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const std::string &type ) {
		Chunk ret = this->getChunk( first, second, third, fourth ); // get a cheap copy
		ret.convertToType( util::getTransposedTypeMap( true, true )[type] );
		return ret;
	}

	Chunk _getChunkAs( const isis::util::ivector4 &coord, const std::string &type ) {
		return _getChunkAs( coord[0], coord[1], coord[2], coord[3], type );
	}

	float _getMin( ) {
		return this->getMinMax().first->as<float>();
	}
	float _getMax( ) {
		return this->getMinMax().second->as<float>();
	}

	const std::string _getMainOrientation( ) {
		switch( this->getMainOrientation() ) {
		case sagittal:
			return std::string( "sagittal" );
			break;
		case reversed_sagittal:
			return std::string( "reversed_sagittal" );
			break;
		case axial:
			return std::string( "axial" );
			break;
		case reversed_axial:
			return std::string( "reversed_axial" );
			break;
		case coronal:
			return std::string( "coronal" );
			break;
		case reversed_coronal:
			return std::string( "reversed_coronal" );
			break;
		default:
			return std::string( "unknown" );
			break;
		}
	}

	void _transformCoords( boost::python::list matrix ) {
		std::vector< boost::python::list > rows;

		for ( int i = 0; i < boost::python::len( matrix ); ++i ) {
			rows.push_back( boost::python::extract< boost::python::list >( matrix[i] ) );
		}

		boost::numeric::ublas::matrix<float> boostMatrix( 3, 3 );

		for ( unsigned short i = 0; i < 3; i++ ) {
			for ( unsigned short j = 0; j < 3; j++ ) {
				boostMatrix( i, j ) = boost::python::extract<float> ( rows[i][j] );
			}
		}

		this->transformCoords( boostMatrix );
	}

	bool _makeOfTypeName( std::string type ) {
		if( type[type.size() - 1] != '*' ) {
			type.append( "*" );
		}

		return this->convertToType( util::getTransposedTypeMap( true, true )[type] );
	}

	size_t _spliceDownTo( const std::string dim ) {
		if( boost::iequals( dim, "sliceDim" ) ) {
			return this->spliceDownTo( sliceDim );
		} else if ( boost::iequals( dim, "timeDim" ) ) {
			return this->spliceDownTo( timeDim );
		} else if ( boost::iequals( dim, "rowDim" ) ) {
			return this->spliceDownTo( rowDim );
		} else if ( boost::iequals( dim, "columnDim" ) ) {
			return this->spliceDownTo( columnDim );
		} else {
			LOG( Runtime, error ) << dim << " is an unknown dimension. Possible dimensions are rowDim, columnDim, sliceDim and timeDim.";
			return 0;
		}
	}
	Image _deepCopy( void ) {
		switch( this->getMajorTypeID() ) {
		case ValuePtr<int8_t>::staticID:
			return MemImage<int8_t>( *this );
			break;
		case ValuePtr<uint8_t>::staticID:
			return MemImage<uint8_t>( *this );
			break;
		case ValuePtr<int16_t>::staticID:
			return MemImage<int16_t>( *this );
			break;
		case ValuePtr<uint16_t>::staticID:
			return MemImage<uint16_t>( *this );
			break;
		case ValuePtr<int32_t>::staticID:
			return MemImage<int32_t>( *this );
			break;
		case ValuePtr<uint32_t>::staticID:
			return MemImage<uint32_t>( *this );
			break;
		case ValuePtr<float>::staticID:
			return MemImage<float>( *this );
			break;
		case ValuePtr<double>::staticID:
			return MemImage<double>( *this );
			break;
		default:
			LOG( Runtime, error ) << "Unregistered pixel type " << util::getTypeMap()[this->getMajorTypeID()] << ".";
			return MemImage<int8_t>( *this );
		}
	}

	Image _deepCopy( std::string type ) {
		if( type[type.size() - 1] != '*' ) {
			type.append( "*" );
		}

		Image retImage = _deepCopy();

		if ( ! isis::util::getTransposedTypeMap()[type] ) {
			LOG( isis::python::Runtime, isis::error ) << "Unable to convert to type "
					<< type << ". Keeping type.";
		} else {
			retImage.convertToType( isis::util::getTransposedTypeMap()[type] );
		}

		return retImage;
	}

	Image _cheapCopy( void ) {
		return *this;
	}

	isis::util::PropertyMap &_getPropMap() {
		isis::util::PropertyMap &retMap = static_cast<isis::util::PropertyMap &>( *this );
		return retMap;
	}

private:
	PyObject *self;

};


class _ImageList : public std::list<Image>
{


};

}
}
}

#endif /* _IMAGE_HPP_ */
