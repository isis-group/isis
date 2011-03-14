#include "DataContainer.hpp"

namespace isis
{
namespace viewer
{

bool DataContainer::addImage( const data::Image &image )
{
	ImageHolder tmpHolder;
	unsigned short majorTypeID = image.getMajorTypeID();

	switch ( majorTypeID ) {
	case data::ValuePtr<int8_t>::staticID:
		tmpHolder.setImage<int8_t>( image );
		break;
	case data::ValuePtr<uint8_t>::staticID:
		tmpHolder.setImage<uint8_t>( image );
		break;
	case data::ValuePtr<int16_t>::staticID:
		tmpHolder.setImage<int16_t>( image );
		break;
	case data::ValuePtr<uint16_t>::staticID:
		tmpHolder.setImage<uint16_t>( image );
		break;
	case data::ValuePtr<int32_t>::staticID:
		tmpHolder.setImage<int32_t>( image );
		break;
	case data::ValuePtr<uint32_t>::staticID:
		tmpHolder.setImage<uint32_t>( image );
		break;
	case data::ValuePtr<int64_t>::staticID:
		tmpHolder.setImage<int64_t>( image );
		break;
	case data::ValuePtr<uint64_t>::staticID:
		tmpHolder.setImage<uint64_t>( image );
		break;
	case data::ValuePtr<float>::staticID:
		tmpHolder.setImage<float>( image );
		break;
	case data::ValuePtr<double>::staticID:
		tmpHolder.setImage<double>( image );
		break;
	default:
		LOG( Runtime, error ) << "I do not know any type of name " << image.getMajorTypeName() << " !";
		return false;
	}

	push_back( tmpHolder );
	return true;
}

	
bool DataContainer::isImage( size_t imageID, size_t timestep, size_t slice ) const
{
	if ( size() < imageID ) {
		return false;
	} else if ( operator[](imageID).getImageVector().size() < timestep )
	{
		return false;
	}
	return true;
	
}

}
} // end namespace