#include "common.hpp"
#include "common.hxx"

using namespace isis;

data::Image isis::itk4::resample(data::Image src, util::vector4<size_t> newsize)
{
	switch ( src.getMajorTypeID() ) {
	case data::ValueArray<int8_t>::staticID():
		return resample_impl<int8_t>( src, newsize);
		break;
	case data::ValueArray<uint8_t>::staticID():
		return resample_impl<uint8_t>( src, newsize );
		break;
	case data::ValueArray<int16_t>::staticID():
		return resample_impl<int16_t>( src, newsize );
		break;
	case data::ValueArray<uint16_t>::staticID():
		return resample_impl<uint16_t>( src, newsize );
		break;
	case data::ValueArray<int32_t>::staticID():
		return resample_impl<int32_t>( src, newsize );
		break;
	case data::ValueArray<uint32_t>::staticID():
		return resample_impl<uint32_t>( src, newsize );
		break;
	case data::ValueArray<float>::staticID():
		return resample_impl<float>( src, newsize );
		break;
	case data::ValueArray<double>::staticID():
	default:
		return resample_impl<double>( src, newsize);
		break;
	}
}

