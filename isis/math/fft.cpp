#include "fft.hpp"
#include "common.hpp"

#ifdef HAVE_GSL
#include "gsl/fft.hxx"
#endif // HAVE_GSL

#ifdef HAVE_CLFFT
#include "details/clfft.hxx"
#endif //HAVE_CLFFT

#ifdef HAVE_FFTW
#include "details/fftw.hxx"
#endif //HAVE_FFTW

isis::data::Chunk isis::math::fft(isis::data::Chunk data, bool inverse, double scale)
{
	switch(data.getTypeID()){
	case data::ValueArray<uint8_t>::staticID():
	case data::ValueArray<uint16_t>::staticID():
	case data::ValueArray<int8_t>::staticID():
	case data::ValueArray<int16_t>::staticID():
	case data::ValueArray<float>::staticID():
	case data::ValueArray<std::complex< float >>::staticID():
		return fft_single(data,inverse,scale);
	default:
		return fft_double(data,inverse,scale);
	}
}

isis::data::TypedChunk< std::complex< float > > isis::math::fft_single(isis::data::MemChunk< std::complex< float > > data, bool inverse, float scale)
{
#ifdef HAVE_CLFFT
	LOG(Runtime,info) << "Using single precision clfft to transform " << data.getSizeAsString() << " data";
	cl::fft(data,inverse,scale);
#elif HAVE_FFTW
	LOG(Runtime,info) << "Using single precision fftw to transform " << data.getSizeAsString() << " data";
	fftw::fft(data,inverse,scale);
#else
	LOG(Runtime,error) << "Sorry, no single precision fft support compiled in (enable clFFT and/or fftw)";
#endif
	return data;
}

isis::data::TypedChunk< std::complex< double > > isis::math::fft_double(isis::data::MemChunk< std::complex< double > > data, bool inverse, double scale)
{
#ifdef HAVE_FFTW
	LOG(Runtime,info) << "Using double precision fftw to transform " << data.getSizeAsString() << " data";
	fftw::fft(data,inverse,scale);
#elif HAVE_GSL
	LOG(Runtime,info) << "Using double precision gsl_fft_complex_transform to transform " << data.getSizeAsString() << " data";
	gsl::fft(data,inverse,scale);
#else
	LOG(Runtime,error) << "Sorry, no double precision fft support compiled in (enable gsl and/or fftw)";
#endif
	return data;
}
