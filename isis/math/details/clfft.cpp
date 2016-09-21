#include <clFFT.h>
#include "clfft.hxx"
#include "../common.hpp"
#include "details_fft.hxx"
#include "opencl/cldevice.hxx"

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
	std::string getErrorString(cl_int err){
		if(err < CLFFT_BUGCHECK)
			return OpenCLDevice::getErrorString(err);

		switch(err){
		case CLFFT_BUGCHECK:return "CLFFT_BUGCHECK";break;
		case CLFFT_NOTIMPLEMENTED:return "CLFFT_NOTIMPLEMENTED";
		case CLFFT_TRANSPOSED_NOTIMPLEMENTED:return "CLFFT_TRANSPOSED_NOTIMPLEMENTED";break;
		case CLFFT_FILE_NOT_FOUND:return "CLFFT_FILE_NOT_FOUND";break;
		case CLFFT_FILE_CREATE_FAILURE:return "CLFFT_FILE_CREATE_FAILURE";break;
		case CLFFT_VERSION_MISMATCH:return "CLFFT_VERSION_MISMATCH";break;
		case CLFFT_INVALID_PLAN:return "CLFFT_INVALID_PLAN";break;
		case CLFFT_DEVICE_NO_DOUBLE:return "CLFFT_DEVICE_NO_DOUBLE";break;
		case CLFFT_DEVICE_MISMATCH:return "CLFFT_DEVICE_MISMATCH";break;
		}
		return "Unknown OpenCL error";
	}
	CLFFTPlan(OpenCLDevice &dev, data::_internal::NDimensional<4> shape):device(dev){
		clfftSetupData fftSetup;
		const clfftDim dim = (clfftDim)shape.getRelevantDims();
		queue = dev.clCreateCommandQueue( );

		/* Prepare OpenCL memory objects and place data inside them. */
		buffer = clCreateBuffer( device, CL_MEM_READ_WRITE, sizeof(std::complex< float >)*shape.getVolume(), NULL, &err );
		LOG_IF(err!=CL_SUCCESS,Runtime,error) << "clCreateBuffer failed with " << getErrorString(err);

		if((err = clfftInitSetupData(&fftSetup)) != CL_SUCCESS || (err = clfftSetup(&fftSetup)) != CL_SUCCESS){
			LOG(Runtime,error) << "clfft setup failed with " << getErrorString(err);
		}

		/* Create a default plan for a complex FFT. */
		err = clfftCreateDefaultPlan(&planHandle, dev, dim, shape.getSizeAsVector().data());

		/* Set plan parameters. */
		err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);
		err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
		err = clfftSetResultLocation(planHandle, CLFFT_INPLACE);

		/* Bake the plan. */
		err = clfftBakePlan(planHandle, 1, &queue, NULL, NULL);
		LOG_IF(err!=CL_SUCCESS,Runtime,error) << "clfft plan creation failed with " << getErrorString(err);
	}
	cl_int transform(data::ValueArray< std::complex< float > > &data,clfftDirection direction){
		size_t buffer_size  = data.bytesPerElem()*data.getLength();
		std::shared_ptr< void > X= data.getRawAddress();


		err = clEnqueueWriteBuffer( queue, buffer, CL_TRUE, 0, buffer_size, X.get(), 0, NULL, NULL );
		LOG_IF(err!=CL_SUCCESS,Runtime,error) << "transferring of " << float(buffer_size)/1024/1024 << "MBytes to OpenCL failed with " << getErrorString(err);

		/* Execute the plan. */
		if((err = clfftEnqueueTransform(planHandle, direction, 1, &queue, 0, NULL, NULL, &buffer, NULL, NULL))!=CL_SUCCESS || (err = clFinish(queue))!=CL_SUCCESS){
			LOG(Runtime,error) << "executing clfft on " << float(buffer_size)/1024/1024 << "MBytes failed with " << getErrorString(err);
		}

		/* Fetch results of calculations. */
		err = clEnqueueReadBuffer( queue, buffer, CL_TRUE, 0, buffer_size, X.get(), 0, NULL, NULL );
		LOG_IF(err!=CL_SUCCESS,Runtime,error) << "getting result data from clfft failed with " << getErrorString(err);

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

void cl::fft(data::TypedChunk< std::complex< float > > &data, bool inverse)
{
	// handle data
	_internal::halfshift(data);

	_internal::OpenCLDevice dev;


    /* Setup clFFT. */
	_internal::CLFFTPlan plan(dev,data);


	plan.transform(data.asValueArray<std::complex< float >>(),inverse?CLFFT_BACKWARD:CLFFT_FORWARD);

	_internal::halfshift(data);
}
}
}
