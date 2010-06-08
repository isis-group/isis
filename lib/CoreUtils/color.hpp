#ifndef COLOR_HPP_INCLUDED
#define COLOR_HPP_INCLUDED
#include <stdint.h>
#include <ostream>
#include "common.hpp"

namespace isis
{
namespace util
{

///Class to store a rgb tripel of type T.
template<typename T> struct color {
	T r, g, b;
	bool operator==( const color &other )const {
		return r == other.r && g == other.g && b == other.b;
	}
	bool operator!=( const color &other )const {
		return not operator==( other );
	}
};

typedef color<u_int8_t> rgb_color24;
typedef color<u_int16_t> rgb_color48;
}
}

namespace std
{
///Streaming output for color using isis::util::write_list
template<typename charT, typename traits, typename T>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::color<T>& s )
{
	const T *const begin = &s.r;
	isis::util::write_list( begin, begin + 3, out );
	return out;
}
}
#endif //COLOR_HPP_INCLUDED
