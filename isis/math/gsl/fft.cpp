/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  Enrico Reimer <reimer@cbs.mpg.de>
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
 *
 */

#include "fft.hxx"
#include "../common.hpp"

#include <math.h>
#include <gsl/gsl_fft_complex.h>
#include "../details/details_fft.hxx"

void fft_impl(isis::data::ValueArray<std::complex<double> > &data,bool inverse,size_t stride=1){
	const size_t elements=data.getLength()/stride;
	std::shared_ptr<gsl_fft_complex_wavetable> wavetable(gsl_fft_complex_wavetable_alloc (elements),gsl_fft_complex_wavetable_free);
	std::shared_ptr<gsl_fft_complex_workspace> workspace(gsl_fft_complex_workspace_alloc (elements),gsl_fft_complex_workspace_free);

	std::shared_ptr< double > ptr=std::static_pointer_cast<double>(data.getRawAddress());

	for(size_t offset=0;offset<stride;offset++){
		gsl_fft_complex_transform(ptr.get()+offset*2, stride, elements, wavetable.get(), workspace.get(),inverse?gsl_fft_backward:gsl_fft_forward);
	}

}

void isis::math::gsl::fft(isis::data::TypedChunk< std::complex< double > > &data, bool inverse, double scale)
{
	_internal::halfshift(data);
	data::ValueArray< std::complex< double > > &array=data.asValueArray<std::complex<double> >();

	for(size_t rank=0;rank<data.getRelevantDims();rank++){
		std::array<size_t,4> dummy_index={0,0,0,0};
		dummy_index[rank]=1;
		//splice into lines of dimsize elements
		size_t stride=data.getLinearIndex(dummy_index);
		std::vector< data::ValueArrayBase::Reference > lines= (rank<data.getRelevantDims()-1) ? //do not call splice for the top rank (full volume)
			array.splice(data.getDimSize(rank)*stride):
			std::vector<data::ValueArrayBase::Reference>(1,array); 

		for(data::ValueArrayBase::Reference line_ref:lines)
			fft_impl(line_ref->castToValueArray<std::complex<double> >(),inverse,stride);
	}

	_internal::halfshift(data);
	
	if(scale==0)
		scale=sqrt(1./data.getVolume());

	if(scale!=1)
		for(std::complex< double > &v:data)
			v*=scale;
}
