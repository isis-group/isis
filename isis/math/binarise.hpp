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

#ifndef BINARISE_H
#define BINARISE_H

#include "histogram.hpp"

namespace isis{
namespace math{
namespace _internal{

template<typename T, typename IType> T otsu_thres_impl(const IType &image){
	static_assert( std::is_integral<T>::value , "cannot compute threshold from non-integer types" );
	auto p=math::normalized_histogram(image);
	typedef decltype(p) hist_type;

	hist_type omega;
	std::partial_sum(p.begin(),p.end(),omega.begin()); 
	
	hist_type range,prod,mu;
	std::iota(range.begin(),range.end(),1); 
	prod = p * range;
	std::partial_sum(prod.begin(),prod.end(),mu.begin()); 
	double mu_t = mu.back();
	
	hist_type one;one.fill(1);
	const auto v1 = (omega*mu_t - mu) * (omega*mu_t - mu);
	const auto v2 = omega * (one - omega);
	hist_type sigma_b_squared = v1 / v2;

	double max = *std::max_element(sigma_b_squared.begin(),sigma_b_squared.end());
	
	if(std::isfinite(max)){
		//M: idx = mean(find(sigma_b_squared == maxval));
		const size_t idx1 = std::distance(sigma_b_squared.begin(), std::find(sigma_b_squared.begin(),sigma_b_squared.end(),max));
		const size_t idx2 = std::distance(std::find(sigma_b_squared.rbegin(),sigma_b_squared.rend(),max), sigma_b_squared.rend());
		const size_t idx = idx1+ (idx2-idx1)/2;
		return idx;
	} else
		return 0;
}
}

template<typename T> T otsu_thres(const data::TypedImage<T> &image){
	return _internal::otsu_thres_impl(image);
}
template<typename T> T otsu_thres(const data::TypedChunk<T> &image){
	return _internal::otsu_thres_impl(image);
}
}
}
#endif // BINARISE_H
