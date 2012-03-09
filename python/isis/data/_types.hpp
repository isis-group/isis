#ifndef TYPES_DATA_HPP
#define TYPES_DATA_HPP

#include "DataStorage/typeptr.hpp"

namespace isis
{
namespace python
{
namespace data
{

using namespace isis::data;
enum image_types { BOOL = ValueArray<bool>::staticID,
				   INT8_T = ValueArray<int8_t>::staticID,
				   UINT8_T = ValueArray<uint8_t>::staticID,
				   INT16_T = ValueArray<int16_t>::staticID,
				   UINT16_T = ValueArray<uint16_t>::staticID,
				   INT32_T = ValueArray<int32_t>::staticID,
				   UINT32_T = ValueArray<uint32_t>::staticID,
				   INT64_T = ValueArray<int64_t>::staticID,
				   UINT64_T = ValueArray<uint64_t>::staticID,
				   FLOAT = ValueArray<float>::staticID,
				   DOUBLE = ValueArray<double>::staticID
				 };


}
}
}
#endif