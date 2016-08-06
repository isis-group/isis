#include <clFFT.h>
#include "../core/data/chunk.hpp"
#include "common.hpp"
#include "details/details_fft.hxx"
#include "details/opencl/cldevice.hxx"

namespace isis{
namespace math{

data::TypedChunk< std::complex< float > > fft(data::MemChunk< std::complex< float > > data, bool inverse, float scale)
{
    cl_int err;
    cl_command_queue queue = 0;
    cl_mem bufX;

    /* FFT library realted declarations */
    clfftPlanHandle planHandle;
    clfftDim dim = (clfftDim)data.getRelevantDims();
	clfftDirection direction= inverse?CLFFT_BACKWARD:CLFFT_FORWARD;
	if(scale==0)
		scale = inverse ? 1./2:1./(data.getVolume()/2);


	_internal::OpenCLDevice dev;
	
    queue = dev.clCreateCommandQueue( );

    /* Setup clFFT. */
    clfftSetupData fftSetup;
    err = clfftInitSetupData(&fftSetup);
    err = clfftSetup(&fftSetup);

	// handle data
	_internal::halfshift(data);
    size_t buffer_size  = data.getBytesPerVoxel()*data.getVolume();
	std::shared_ptr< void > X= data.asValueArrayBase().getRawAddress();

    /* Prepare OpenCL memory objects and place data inside them. */
    bufX = clCreateBuffer( dev, CL_MEM_READ_WRITE, buffer_size, NULL, &err );

    err = clEnqueueWriteBuffer( queue, bufX, CL_TRUE, 0, buffer_size, X.get(), 0, NULL, NULL );

    /* Create a default plan for a complex FFT. */
    err = clfftCreateDefaultPlan(&planHandle, dev, dim, data.getSizeAsVector().data());

    /* Set plan parameters. */
    err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);
    err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
    err = clfftSetResultLocation(planHandle, CLFFT_INPLACE);
	err = clfftSetPlanScale(planHandle, direction,scale);

    /* Bake the plan. */
    err = clfftBakePlan(planHandle, 1, &queue, NULL, NULL);

    /* Execute the plan. */
    err = clfftEnqueueTransform(planHandle, direction, 1, &queue, 0, NULL, NULL, &bufX, NULL, NULL);

    /* Wait for calculations to be finished. */
    err = clFinish(queue);

    /* Fetch results of calculations. */
    err = clEnqueueReadBuffer( queue, bufX, CL_TRUE, 0, buffer_size, X.get(), 0, NULL, NULL );


    /* Release OpenCL memory objects. */
    clReleaseMemObject( bufX );

    /* Release the plan. */
    err = clfftDestroyPlan( &planHandle );

    /* Release clFFT library. */
    clfftTeardown( );

    /* Release OpenCL working objects. */
    clReleaseCommandQueue( queue );

	_internal::halfshift(data);
    return data;
}
}
}
