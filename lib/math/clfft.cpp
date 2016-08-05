#include <clFFT.h>
#include "../core/data/chunk.hpp"
#include "common.hpp"

namespace isis{
namespace math{
namespace _internal{

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

void halfshift(isis::data::Chunk &data){
	isis::data::ValueArrayBase &array=data.asValueArrayBase();
	for(size_t rank=0;rank<data.getRelevantDims();rank++){
		std::array<size_t,4> dummy_index={0,0,0,0};
		dummy_index[rank]=1;
		//splice into lines of dimsize elements
		size_t stride=data.getLinearIndex(dummy_index);
		std::vector< data::ValueArrayBase::Reference > lines= (rank<data.getRelevantDims()-1) ? //do not call splice for the top rank (full volume)
			array.splice(data.getDimSize(rank)*stride):
			std::vector<data::ValueArrayBase::Reference>(1,array);

		for(data::ValueArrayBase::Reference line:lines){
			halfshift(*line);
		}
	}
}

class OpenCLDevice{
private:
	cl_int err;
	cl_context ctx = 0;
public:
	static std::string getPlatformName(cl_platform_id platform){
		char platform_name[128];
		clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
		return platform_name;
	}
	static std::string getDeviceName(cl_device_id device){
		char device_name[128];
		clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
		return device_name;
	}
	static std::vector<cl_platform_id> getPlatformIDs(){
		cl_platform_id platforms[10];
		cl_uint platform_nr;
		cl_int err;
		if((err=clGetPlatformIDs( 10, platforms, &platform_nr)) != CL_SUCCESS){
			LOG(Runtime,error) << "Failed to get available platforms (error was " << err << ")";
			return std::vector<cl_platform_id> ();
		} else
			return std::vector<cl_platform_id>(platforms,platforms+platform_nr);
	}
	static std::vector<cl_device_id> getDeviceIDs(cl_platform_id platform,cl_device_type device_type=CL_DEVICE_TYPE_ALL){
		cl_device_id devices[10];
		cl_uint device_nr;
		cl_int err;
		if((err=clGetDeviceIDs( platform, device_type, 10, devices, &device_nr ))!= CL_SUCCESS){
			LOG(Runtime,error)
				<< "Failed to get available devices for platform " << getPlatformName(platform)
				<< " (error was " << err << ")";
			return std::vector<cl_device_id>();
		} else
			return std::vector<cl_device_id>(devices,devices+device_nr);
	}
	std::vector<cl_device_id> getMyDeviceIDs(){
		cl_device_id devices[10];
		size_t device_nr;
		cl_int err;
		if((err=clGetContextInfo( ctx, CL_CONTEXT_DEVICES, 10*sizeof(cl_device_id), devices, &device_nr ))!= CL_SUCCESS){
			LOG(Runtime,error)
				<< "Failed to get current available devices (error was " << err << ")";
			return std::vector<cl_device_id>();
		} else
			return std::vector<cl_device_id>(devices,devices+device_nr);
	}
	cl_context createContext(cl_platform_id platform,cl_device_type  device_type=CL_DEVICE_TYPE_ALL){
		if(getDeviceIDs(platform,device_type).size()){
			cl_context_properties props[3] = {CL_CONTEXT_PLATFORM,(cl_context_properties)platform,0};
			cl_context ret=clCreateContextFromType (props,device_type,nullptr,nullptr,&err);
			if(err==CL_SUCCESS){
				LOG(Runtime,notice) << "Using " << getDeviceIDs(platform,device_type).size() << " OpenCL device(s) from platform " << getPlatformName(platform);
				return ret;
			} else
				LOG(Runtime,error) << "Failed to create OpenCL context on platform " << getPlatformName(platform);
		} else {
			LOG(Runtime,error) << "No devices available for platform " << getPlatformName(platform);
		}
		return 0;
	}
	OpenCLDevice(){
		/* Setup OpenCL environment. */
		std::pair<cl_platform_id,int> biggest;
		for(cl_platform_id p:getPlatformIDs()){
			std::size_t dev_cnt=getDeviceIDs(p).size();
			if(biggest.second<dev_cnt)
				biggest = {p,dev_cnt};
		}
		if(biggest.first)
			ctx=createContext(biggest.first);
	}
	~OpenCLDevice(){
		clReleaseContext( ctx );
	}
	bool good(){return ctx!=nullptr && err==CL_SUCCESS;}
	
	cl_command_queue clCreateCommandQueue(){
		return ::clCreateCommandQueue( ctx, getMyDeviceIDs().front(), 0, &err );
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
