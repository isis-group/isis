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

data::Image isis::itk4::rotate(data::Image src, std::pair<int,int> rotation_plain, float angle, bool pix_center)
{
	switch ( src.getMajorTypeID() ) {
	case data::ValueArray<int8_t>::staticID():
		return rotate_impl<int8_t>( src, rotation_plain, angle, pix_center);
		break;
	case data::ValueArray<uint8_t>::staticID():
		return rotate_impl<uint8_t>( src, rotation_plain, angle, pix_center);
		break;
	case data::ValueArray<int16_t>::staticID():
		return rotate_impl<int16_t>( src, rotation_plain, angle, pix_center);
		break;
	case data::ValueArray<uint16_t>::staticID():
		return rotate_impl<uint16_t>( src, rotation_plain, angle, pix_center);
		break;
	case data::ValueArray<int32_t>::staticID():
		return rotate_impl<int32_t>( src, rotation_plain, angle, pix_center);
		break;
	case data::ValueArray<uint32_t>::staticID():
		return rotate_impl<uint32_t>( src, rotation_plain, angle, pix_center);
		break;
	case data::ValueArray<float>::staticID():
		return rotate_impl<float>( src, rotation_plain, angle, pix_center);
		break;
	case data::ValueArray<double>::staticID():
	default:
		return rotate_impl<double>( src, rotation_plain, angle, pix_center);
		break;
	}
}

data::Image isis::itk4::translate(data::Image src, util::fvector3 translation)
{
	switch ( src.getMajorTypeID() ) {
	case data::ValueArray<int8_t>::staticID():
		return translate_impl<int8_t>( src, translation);
		break;
	case data::ValueArray<uint8_t>::staticID():
		return translate_impl<uint8_t>( src, translation);
		break;
	case data::ValueArray<int16_t>::staticID():
		return translate_impl<int16_t>( src, translation);
		break;
	case data::ValueArray<uint16_t>::staticID():
		return translate_impl<uint16_t>( src, translation);
		break;
	case data::ValueArray<int32_t>::staticID():
		return translate_impl<int32_t>( src, translation);
		break;
	case data::ValueArray<uint32_t>::staticID():
		return translate_impl<uint32_t>( src, translation);
		break;
	case data::ValueArray<float>::staticID():
		return translate_impl<float>( src, translation);
		break;
	case data::ValueArray<double>::staticID():
	default:
		return translate_impl<double>( src, translation);
		break;
	}
}
