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

namespace isis
{
namespace python
{
class _Image : public isis::data::Image, boost::python::wrapper<isis::data::Image>
{
public:

	_Image ( PyObject *p ) : self( p ), boost::python::wrapper< isis::data::Image >() {}
	_Image ( PyObject *p, const isis::data::Image &base ) : isis::data::Image( base ), self( p ), boost::python::wrapper< isis::data::Image >()  {}

	float _voxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth ) {
		data::Chunk ch = this->getChunk( first, second, third, fourth, false );

		switch( ch.getTypeID() ) {
		case data::ValuePtr<int8_t>::staticID:
			return isis::data::Image::voxel<int8_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<uint8_t>::staticID:
			return isis::data::Image::voxel<uint8_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<int16_t>::staticID:
			return isis::data::Image::voxel<int16_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<uint16_t>::staticID:
			return isis::data::Image::voxel<uint16_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<int32_t>::staticID:
			return isis::data::Image::voxel<int32_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<uint32_t>::staticID:
			return isis::data::Image::voxel<uint32_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<float>::staticID:
			return isis::data::Image::voxel<float>( first, second, third, fourth );
			break;
		case data::ValuePtr<double>::staticID:
			return isis::data::Image::voxel<double>( first, second, third, fourth );
			break;
		default:
			return 0;
		}
	}
	float _voxel( const isis::util::ivector4 &coord ) {
		return _voxel( coord[0], coord[1], coord[2], coord[3] );
	}


	bool _setVoxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const float &value ) {
		data::Chunk ch = this->getChunk( first, second, third, fourth, false );

		switch( ch.getTypeID() ) {
		case data::ValuePtr<int8_t>::staticID:
			isis::data::Image::voxel<int8_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<uint8_t>::staticID:
			isis::data::Image::voxel<uint8_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<int16_t>::staticID:
			isis::data::Image::voxel<int16_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<uint16_t>::staticID:
			isis::data::Image::voxel<uint16_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<int32_t>::staticID:
			isis::data::Image::voxel<int32_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<uint32_t>::staticID:
			isis::data::Image::voxel<uint32_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<float>::staticID:
			isis::data::Image::voxel<float>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<double>::staticID:
			isis::data::Image::voxel<double>( first, second, third, fourth ) = value;
			return true;
			break;
		default:
			return false;
		}
	}

	bool _setVoxel( const isis::util::ivector4 &coord, const float &value ) {
		return _setVoxel( coord[0], coord[1], coord[2], coord[3], value );
	}

	std::list<isis::data::Chunk> _getChunksAsVector( void ) {
		std::list<isis::data::Chunk> retChunkList;
		std::vector<boost::shared_ptr<isis::data::Chunk> > chunkList( this->getChunksAsVector() );
		BOOST_FOREACH( std::vector<boost::shared_ptr<isis::data::Chunk> >::reference ref, chunkList ) {
			retChunkList.push_back( *ref );
		}
		return retChunkList;
	}

	const isis::util::ivector4 _getSizeAsVector( ) {
		return this->getSizeAsVector();
	}

	isis::data::Chunk _getChunk( const isis::util::ivector4 &coord, bool copy_metadata ) {
		return this->getChunk( coord[0], coord[1], coord[2], coord[3], copy_metadata );
	}

	isis::data::Chunk _getChunkAs( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const std::string &type ) {
		isis::data::Chunk ret = this->getChunk( first, second, third, fourth ); // get a cheap copy
		ret.convertToType( util::getTransposedTypeMap( true, true )[type] );
		return ret;
	}

	isis::data::Chunk _getChunkAs( const isis::util::ivector4 &coord, const std::string &type ) {
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
			return this->spliceDownTo( isis::data::sliceDim );
		} else if ( boost::iequals( dim, "timeDim" ) ) {
			return this->spliceDownTo( isis::data::timeDim );
		} else if ( boost::iequals( dim, "rowDim" ) ) {
			return this->spliceDownTo( isis::data::rowDim );
		} else if ( boost::iequals( dim, "columnDim" ) ) {
			return this->spliceDownTo( isis::data::columnDim );
		} else {
			LOG( Runtime, error ) << dim << " is an unknown dimension. Possible dimensions are rowDim, columnDim, sliceDim and timeDim.";
			return 0;
		}
	}
	isis::data::Image _deepCopy( void ) {
		switch( this->getMajorTypeID() ) {
		case data::ValuePtr<int8_t>::staticID:
			return isis::data::MemImage<int8_t>( *this );
			break;
		case data::ValuePtr<uint8_t>::staticID:
			return isis::data::MemImage<uint8_t>( *this );
			break;
		case data::ValuePtr<int16_t>::staticID:
			return isis::data::MemImage<int16_t>( *this );
			break;
		case data::ValuePtr<uint16_t>::staticID:
			return isis::data::MemImage<uint16_t>( *this );
			break;
		case data::ValuePtr<int32_t>::staticID:
			return isis::data::MemImage<int32_t>( *this );
			break;
		case data::ValuePtr<uint32_t>::staticID:
			return isis::data::MemImage<uint32_t>( *this );
			break;
		case data::ValuePtr<float>::staticID:
			return isis::data::MemImage<float>( *this );
			break;
		case data::ValuePtr<double>::staticID:
			return isis::data::MemImage<double>( *this );
			break;
		default:
			LOG( Runtime, error ) << "Unregistered pixel type " << getTypeMap()[this->getMajorTypeID()] << ".";
		}
	}

	isis::data::Image _deepCopy( std::string type ) {
		if( type[type.size() - 1] != '*' ) {
			type.append( "*" );
		}

		isis::data::Image retImage = _deepCopy();

		if ( ! isis::util::getTransposedTypeMap()[type] ) {
			LOG( isis::python::Runtime, isis::error ) << "Unable to convert to type "
					<< type << ". Keeping type.";
		} else {
			retImage.convertToType( isis::util::getTransposedTypeMap()[type] );
		}

		return retImage;
	}

	isis::data::Image _cheapCopy( void ) {
		return *this;
	}

	isis::util::PropertyMap &_getPropMap() {
		isis::util::PropertyMap &retMap = static_cast<isis::util::PropertyMap &>( *this );
		return retMap;
	}

private:
	PyObject *self;

};


class _ImageList : public std::list<isis::data::Image>
{


};

}
}

#endif /* _IMAGE_HPP_ */
