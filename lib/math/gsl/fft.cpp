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

#include "fft.hpp"
#include "../common.hpp"

#include <math.h>
#include <gsl/gsl_fft_complex.h>

void fft_impl(isis::data::ValueArray<std::complex<double> > &data,bool inverse,size_t stride=1){
	const size_t elements=data.getLength()/stride;
	std::shared_ptr<gsl_fft_complex_wavetable> wavetable(gsl_fft_complex_wavetable_alloc (elements),gsl_fft_complex_wavetable_free);
	std::shared_ptr<gsl_fft_complex_workspace> workspace(gsl_fft_complex_workspace_alloc (elements),gsl_fft_complex_workspace_free);

	std::shared_ptr< double > ptr=std::static_pointer_cast<double>(data.getRawAddress());

	for(size_t offset=0;offset<stride;offset++){
		if(inverse)
			gsl_fft_complex_inverse(ptr.get()+offset*2, stride, elements, wavetable.get(), workspace.get());
		else
			gsl_fft_complex_forward(ptr.get()+offset*2, stride, elements, wavetable.get(), workspace.get());
	}

}

void halfshift(isis::data::ValueArrayBase &src){
	//shift backwards
	assert(src.getLength()%2==0);
	const size_t shiftsize=src.getLength()/2*src.bytesPerElem();
	std::shared_ptr<uint8_t> begin=std::static_pointer_cast<uint8_t>(src.getRawAddress());

	std::shared_ptr<uint8_t> buffer((uint8_t*)malloc(shiftsize));
	memcpy(buffer.get(),         begin.get(),          shiftsize);
	memcpy(begin.get(),          begin.get()+shiftsize,shiftsize);
	memcpy(begin.get()+shiftsize,buffer.get(),         shiftsize);
}

isis::data::TypedChunk<std::complex< double >> isis::math::gsl::fft(isis::data::MemChunk< std::complex< double > > data, bool inverse,std::complex<double> scale)
{
	data::ValueArray< std::complex< double > > &array=data.asValueArray<std::complex<double> >();

	for(size_t rank=0;rank<data.getRelevantDims();rank++){
		std::array<size_t,4> dummy_index={0,0,0,0};
		dummy_index[rank]=1;
		//splice into lines of dimsize elements
		size_t stride=data.getLinearIndex(dummy_index);
		std::vector< data::ValueArrayBase::Reference > lines= (rank<data.getRelevantDims()-1) ? //do not call splice for the top rank (full volume)
			array.splice(data.getDimSize(rank)*stride):
			std::vector<data::ValueArrayBase::Reference>(1,array); 

		for(size_t i=0;i<lines.size();i++){
			data::ValueArray< std::complex< double > > &line=lines[i]->castToValueArray<std::complex<double> >();
			halfshift(line);
			fft_impl(line,inverse,stride);
			halfshift(line);
		}
	}
	if(scale==std::complex<double>(0))
		scale = inverse ? data.getVolume()/2:1./(data.getVolume()/2);

	for(std::complex< double > &v:data){
		v*=scale;
	}

	return data;
}
