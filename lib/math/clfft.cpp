#include <clFFT.h>
#include "../core/data/chunk.hpp"
#include "common.hpp"

namespace isis{
namespace math{
namespace _internal{
	
class OpenCLDevice{
private:
	cl_int err;
	cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
	cl_device_id device = 0;
	cl_context ctx = 0;
public:
	OpenCLDevice(){
		/* Setup OpenCL environment. */
		cl_platform_id platform;
		if(
			(err=clGetPlatformIDs( 1, &platform, nullptr )) != CL_SUCCESS ||
			(err=clGetDeviceIDs( platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL ))!= CL_SUCCESS
		){
			LOG(Runtime,error) << "Failed to get available platforms (error was " << err << ")";
		} else {
			char platform_name[128];
			char device_name[128];
			
			clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
			clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
			
			LOG(Runtime,notice) << "Using " << device_name << " on platform "<< platform_name;
			
				props[1] = (cl_context_properties)platform;
				ctx = clCreateContext( props, 1, &device, NULL, NULL, &err );
		}
		
	}
	~OpenCLDevice(){
		clReleaseContext( ctx );
	}
	bool good(){return err==CL_SUCCESS;}
	
	cl_command_queue clCreateCommandQueue(){
		return ::clCreateCommandQueue( ctx, device, 0, &err );
	}

	cl_mem clCreateBuffer(cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret ){
		return ::clCreateBuffer( ctx, flags, size, host_ptr, errcode_ret );
	}
	cl_int clEnqueueWriteBuffer(cl_command_queue queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t buffer_size, const void *ptr, cl_uint num_events_in_wait_list=0, const cl_event *event_wait_list=nullptr, cl_event *event=nullptr){
		return ::clEnqueueWriteBuffer(queue, buffer, blocking_write, offset, buffer_size, ptr, num_events_in_wait_list, event_wait_list, event );
	}
	operator cl_context(){return ctx;}
};
}

data::MemChunk< std::complex< float > > fft(data::MemChunk< std::complex< float > > data, bool inverse)
{
    cl_int err;
    cl_command_queue queue = 0;
    cl_mem bufX;
    float *X;

    const size_t N0 = 100, N1 = 100, N2 = 100;
    char platform_name[128];
    char device_name[128];

    /* FFT library realted declarations */
    clfftPlanHandle planHandle;
    clfftDim dim = CLFFT_3D;
    size_t clLengths[3] = {N0, N1, N2};

	_internal::OpenCLDevice dev;
	
    queue = dev.clCreateCommandQueue( );

    /* Setup clFFT. */
    clfftSetupData fftSetup;
    err = clfftInitSetupData(&fftSetup);
    err = clfftSetup(&fftSetup);

    /* Allocate host & initialize data. */
    /* Only allocation shown for simplicity. */
    size_t buffer_size  = N0 * N1 * N2 * 2 * sizeof(*X);
    X = (float *)malloc(buffer_size);

    /* print input array just using the
     * indices to fill the array with data */
    printf("\nPerforming fft on an two dimensional array of size N0 x N1 x N2 : %u x %u x %u\n", (unsigned long)N0, (unsigned long)N1, (unsigned long)N2);
    size_t i, j, k;
    i = j = k = 0;
    for (i=0; i<N0; ++i) {
        for (j=0; j<N1; ++j) {
            for (k=0; k<N2; ++k) {
                float x = 0.0f;
                float y = 0.0f;
                if (i==0 && j==0 && k==0) {
                    x = y = 0.5f;
                }
				size_t idx = 2*(k+j*N1+i*N0*N1);
                X[idx] = x;
                X[idx+1] = y;
//                 printf("(%f, %f) ", X[idx], X[idx+1]);
            }
//             printf("\n");
        }
//         printf("\n");
    }

    /* Prepare OpenCL memory objects and place data inside them. */
    bufX = clCreateBuffer( dev, CL_MEM_READ_WRITE, buffer_size, NULL, &err );

    err = clEnqueueWriteBuffer( queue, bufX, CL_TRUE, 0, buffer_size, X, 0, NULL, NULL );

    /* Create a default plan for a complex FFT. */
    err = clfftCreateDefaultPlan(&planHandle, dev, dim, clLengths);

    /* Set plan parameters. */
    err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);
    err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
    err = clfftSetResultLocation(planHandle, CLFFT_INPLACE);

    /* Bake the plan. */
    err = clfftBakePlan(planHandle, 1, &queue, NULL, NULL);

    /* Execute the plan. */
    err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &queue, 0, NULL, NULL, &bufX, NULL, NULL);

    /* Wait for calculations to be finished. */
    err = clFinish(queue);

    /* Fetch results of calculations. */
    err = clEnqueueReadBuffer( queue, bufX, CL_TRUE, 0, buffer_size, X, 0, NULL, NULL );

    /* print output array */
//     printf("\n\nfft result: \n");
//     i = j = k = 0;
//     for (i=0; i<N0; ++i) {
//         for (j=0; j<N1; ++j) {
//             for (k=0; k<N2; ++k) {
// 				size_t idx = 2*(k+j*N1+i*N0*N1);
//                 printf("(%f, %f) ", X[idx], X[idx+1]);
//             }
//             printf("\n");
//         }
//         printf("\n");
//     }
//     printf("\n");

    /* Release OpenCL memory objects. */
    clReleaseMemObject( bufX );

    free(X);

    /* Release the plan. */
    err = clfftDestroyPlan( &planHandle );

    /* Release clFFT library. */
    clfftTeardown( );

    /* Release OpenCL working objects. */
    clReleaseCommandQueue( queue );

    return data;
}
}
}
