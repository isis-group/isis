#ifndef CORE_CONFIG_HPP
#define CORE_CONFIG_HPP

#if defined(__GNUC__) && __GNUC__ >= 4 && !defined(__MINGW32__)
#define API_EXCLUDE_BEGIN _Pragma("GCC visibility push(hidden)")
#define API_EXCLUDE_END   _Pragma("GCC visibility pop")
#else
#define API_EXCLUDE_BEGIN
#define API_EXCLUDE_END
#endif

#include <cstddef>

#include <stdint.h>

using std::size_t;

#endif //CORE_CONFIG_HPP
