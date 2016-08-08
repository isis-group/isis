#include <clFFT.h>
#include "../core/data/chunk.hpp"
#include "common.hpp"
#include "details/details_fft.hxx"
#include "details/opencl/cldevice.hxx"

namespace isis{
namespace math{
namespace _internal{
class CLFFTPlan{
    clfftPlanHandle planHandle;
	cl_int err;
	OpenCLDevice &device;
	cl_command_queue queue;
	cl_mem buffer;
public:
	CLFFTPlan(OpenCLDevice &dev, data::_internal::NDimensional<4> shape):device(dev){
		clfftSetupData fftSetup;
		const clfftDim dim = (clfftDim)shape.getRelevantDims();
		queue = dev.clCreateCommandQueue( );

		/* Prepare OpenCL memory objects and place data inside them. */
		buffer = clCreateBuffer( device, CL_MEM_READ_WRITE, sizeof(std::complex< float >)*shape.getVolume(), NULL, &err );

		err = clfftInitSetupData(&fftSetup);
		err = clfftSetup(&fftSetup);

		/* Create a default plan for a complex FFT. */
		err = clfftCreateDefaultPlan(&planHandle, dev, dim, shape.getSizeAsVector().data());

		/* Set plan parameters. */
		err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);
		err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
		err = clfftSetResultLocation(planHandle, CLFFT_INPLACE);

		/* Bake the plan. */
		err = clfftBakePlan(planHandle, 1, &queue, NULL, NULL);
	}
	cl_int transform(data::ValueArray< std::complex< float > > &data,clfftDirection direction){
		size_t buffer_size  = data.bytesPerElem()*data.getLength();
		std::shared_ptr< void > X= data.getRawAddress();


		err = clEnqueueWriteBuffer( queue, buffer, CL_TRUE, 0, buffer_size, X.get(), 0, NULL, NULL );
		/* Execute the plan. */
		err = clfftEnqueueTransform(planHandle, direction, 1, &queue, 0, NULL, NULL, &buffer, NULL, NULL);

		/* Wait for calculations to be finished. */
		err = clFinish(queue);

		/* Fetch results of calculations. */
		err = clEnqueueReadBuffer( queue, buffer, CL_TRUE, 0, buffer_size, X.get(), 0, NULL, NULL );
		return err;
	}
	~CLFFTPlan(){
		/* Release OpenCL memory objects. */
		clReleaseMemObject( buffer );

		/* Release the plan. */
		clfftDestroyPlan( &planHandle );

		/* Release clFFT library. */
		clfftTeardown( );

		/* Release OpenCL working objects. */
		clReleaseCommandQueue( queue );
	}
};
}
data::TypedChunk< std::complex< float > > fft(data::MemChunk< std::complex< float > > data, bool inverse, float scale)
{
    cl_int err;
    cl_mem bufX;

	_internal::OpenCLDevice dev;


    /* Setup clFFT. */
	_internal::CLFFTPlan plan(dev,data);

	// handle data
	_internal::halfshift(data);

	plan.transform(data.asValueArray<std::complex< float >>(),inverse?CLFFT_BACKWARD:CLFFT_FORWARD);

	_internal::halfshift(data);
    return data;
}
}
}
