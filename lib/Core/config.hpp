#ifndef CORE_CONFIG_HPP
#define CORE_CONFIG_HPP

#if defined(__GNUC__) && __GNUC__ >= 4 && !defined(__MINGW32__)
#define API_EXCLUDE_BEGIN
#define API_EXCLUDE_END
#else
#define API_EXCLUDE_BEGIN _Pragma("GCC visibility push(hidden)")
#define API_EXCLUDE_END   _Pragma("GCC visibility pop")
#endif

#include <cstddef>

#ifdef _MSC_VER
typedef boost::int8_t   int8_t;
typedef boost::int16_t  int16_t;
typedef boost::int32_t  int32_t;
typedef boost::uint8_t  uint8_t;
typedef boost::uint16_t uint16_t;
typedef boost::uint32_t uint32_t;
typedef boost::int64_t  int64_t;
typedef boost::uint64_t uint64_t;
#else
#include <stdint.h>
#endif

using std::size_t;

#endif //CORE_CONFIG_HPP
