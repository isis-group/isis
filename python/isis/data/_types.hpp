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
enum image_types { BOOL = ValuePtr<bool>::staticID,
				   INT8_T = ValuePtr<int8_t>::staticID,
				   UINT8_T = ValuePtr<uint8_t>::staticID,
				   INT16_T = ValuePtr<int16_t>::staticID,
				   UINT16_T = ValuePtr<uint16_t>::staticID,
				   INT32_T = ValuePtr<int32_t>::staticID,
				   UINT32_T = ValuePtr<uint32_t>::staticID,
				   INT64_T = ValuePtr<int64_t>::staticID,
				   UINT64_T = ValuePtr<uint64_t>::staticID,
				   FLOAT = ValuePtr<float>::staticID,
				   DOUBLE = ValuePtr<double>::staticID
				 };


}
}
}
#endif