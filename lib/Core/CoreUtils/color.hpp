#ifndef COLOR_HPP_INCLUDED
#define COLOR_HPP_INCLUDED

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
		return ! operator==( other );
	}
};

typedef color<uint8_t> color24;
typedef color<uint16_t> color48;
}
}

namespace std
{
///Streaming output for color using isis::util::listToOStream
template<typename charT, typename traits, typename T>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::color<T>& s )
{
	const T *const begin = &s.r;
	isis::util::listToOStream( begin, begin + 3, out );
	return out;
}
}
#endif //COLOR_HPP_INCLUDED
