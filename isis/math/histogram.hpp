/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2018  Enrico Reimer <reimer@cbs.mpg.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "../data/image.hpp"
#include <type_traits>
#include <cmath>

namespace isis{
namespace math{
namespace _internal{
	
template<typename T, typename IType> std::array<size_t,(size_t)std::exp2(sizeof(T)*8)> histogram_impl(IType chunk){
	static_assert( std::is_integral<T>::value , "cannot compute histogram from non-integer types" );
	constexpr size_t arraysize=std::exp2(sizeof(T)*8);
	std::array<size_t,arraysize> ret;
	ret.fill(0);
	for(const T v:chunk)
		ret[v]++;
	return ret;
}

template<typename T, typename IType> std::array<double,(size_t)std::exp2(sizeof(T)*8)> normalized_histogram_impl(IType chunk){
	constexpr size_t arraysize=std::exp2(sizeof(T)*8);
	std::array<double,arraysize> ret;
	const std::array<size_t,arraysize> hist=histogram_impl<T,IType>(chunk);
	const uint64_t sum=std::accumulate(hist.begin(),hist.end(),0);

	std::transform(hist.begin(),hist.end(),ret.begin(),[sum](size_t v){return double(v)/sum;});

	return ret;
}
}

template<typename T> std::array<size_t,(size_t)std::exp2(sizeof(T)*8)> histogram(data::TypedChunk<T> chunk){
	return _internal::histogram_impl<T,data::TypedChunk<T>>(chunk);
}

template<typename T> std::array<size_t,(size_t)std::exp2(sizeof(T)*8)> histogram(isis::data::TypedImage<T> image){
	return _internal::histogram_impl<T,data::TypedImage<T>>(image);
}

template<typename T> std::array<double,(size_t)std::exp2(sizeof(T)*8)> normalized_histogram(data::TypedChunk<T> chunk){
	return _internal::normalized_histogram_impl<T,data::TypedChunk<T>>(chunk);
}
template<typename T> std::array<double,(size_t)std::exp2(sizeof(T)*8)> normalized_histogram(data::TypedImage<T> image){
	return _internal::normalized_histogram_impl<T,data::TypedChunk<T>>(image);
}
}
}

#endif // HISTOGRAM_H
