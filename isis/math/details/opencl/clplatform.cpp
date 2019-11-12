#include "clplatform.hxx"

namespace isis{
namespace math{
namespace _internal{

template<> std::string getDeviceInfo<std::string>(cl_device_id device,cl_device_info info){
	char info_text[128];
	const cl_int err=clGetDeviceInfo(device, info, sizeof(info_text), info_text, NULL);
	LOG_IF(err!=CL_SUCCESS,Runtime,error) << "clGetDeviceInfo failed with " << OpenCLPlatform::getErrorString(err);
	return info_text;

}
template<> std::string getPlatformInfo<std::string>(cl_platform_id platform,cl_platform_info info){
	char platform_info[128];
	const cl_int err=clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_info), platform_info, NULL);
	LOG_IF(err!=CL_SUCCESS,Runtime,error) << "clGetPlatformInfo failed with " << OpenCLPlatform::getErrorString(err);
	return platform_info;
}

std::string getPlatformName(cl_platform_id platform){
	return getPlatformInfo<std::string>(platform,CL_PLATFORM_NAME);
}

std::string getDeviceName(cl_device_id device){
	return getDeviceInfo<std::string>(device,CL_DEVICE_NAME);
}

std::vector<cl_platform_id> getPlatformIDs(){
	cl_platform_id platforms[10];
	cl_uint platform_nr;
	cl_int err;
	if((err=clGetPlatformIDs( 10, platforms, &platform_nr)) != CL_SUCCESS){
		LOG(Runtime,error) << "Failed to get available platforms (error was " << OpenCLPlatform::getErrorString(err) << ")";
		return std::vector<cl_platform_id> ();
	} else
		return std::vector<cl_platform_id>(platforms,platforms+platform_nr);
}

std::vector<cl_device_id> getDeviceIDs(cl_platform_id platform, cl_device_type device_type){
	cl_device_id devices[10];
	cl_uint device_nr;
	cl_int err;
	if((err=clGetDeviceIDs( platform, device_type, 10, devices, &device_nr ))!= CL_SUCCESS){
		LOG(Runtime,error)
				<< "Failed to get available devices for platform " << getPlatformName(platform)
				<< " (error was " << OpenCLPlatform::getErrorString(err) << ")";
		return std::vector<cl_device_id>();
	} else
		return std::vector<cl_device_id>(devices,devices+device_nr);
}

std::string OpenCLPlatform::getErrorString(cl_int err)
{
	switch(err){
	// run-time and JIT compiler errors
	case 0: return "CL_SUCCESS";
	case -1: return "CL_DEVICE_NOT_FOUND";
	case -2: return "CL_DEVICE_NOT_AVAILABLE";
	case -3: return "CL_COMPILER_NOT_AVAILABLE";
	case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case -5: return "CL_OUT_OF_RESOURCES";
	case -6: return "CL_OUT_OF_HOST_MEMORY";
	case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case -8: return "CL_MEM_COPY_OVERLAP";
	case -9: return "CL_IMAGE_FORMAT_MISMATCH";
	case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case -11: return "CL_BUILD_PROGRAM_FAILURE";
	case -12: return "CL_MAP_FAILURE";
	case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case -15: return "CL_COMPILE_PROGRAM_FAILURE";
	case -16: return "CL_LINKER_NOT_AVAILABLE";
	case -17: return "CL_LINK_PROGRAM_FAILURE";
	case -18: return "CL_DEVICE_PARTITION_FAILED";
	case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

	// compile-time errors
	case -30: return "CL_INVALID_VALUE";
	case -31: return "CL_INVALID_DEVICE_TYPE";
	case -32: return "CL_INVALID_PLATFORM";
	case -33: return "CL_INVALID_DEVICE";
	case -34: return "CL_INVALID_CONTEXT";
	case -35: return "CL_INVALID_QUEUE_PROPERTIES";
	case -36: return "CL_INVALID_COMMAND_QUEUE";
	case -37: return "CL_INVALID_HOST_PTR";
	case -38: return "CL_INVALID_MEM_OBJECT";
	case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case -40: return "CL_INVALID_IMAGE_SIZE";
	case -41: return "CL_INVALID_SAMPLER";
	case -42: return "CL_INVALID_BINARY";
	case -43: return "CL_INVALID_BUILD_OPTIONS";
	case -44: return "CL_INVALID_PROGRAM";
	case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
	case -46: return "CL_INVALID_KERNEL_NAME";
	case -47: return "CL_INVALID_KERNEL_DEFINITION";
	case -48: return "CL_INVALID_KERNEL";
	case -49: return "CL_INVALID_ARG_INDEX";
	case -50: return "CL_INVALID_ARG_VALUE";
	case -51: return "CL_INVALID_ARG_SIZE";
	case -52: return "CL_INVALID_KERNEL_ARGS";
	case -53: return "CL_INVALID_WORK_DIMENSION";
	case -54: return "CL_INVALID_WORK_GROUP_SIZE";
	case -55: return "CL_INVALID_WORK_ITEM_SIZE";
	case -56: return "CL_INVALID_GLOBAL_OFFSET";
	case -57: return "CL_INVALID_EVENT_WAIT_LIST";
	case -58: return "CL_INVALID_EVENT";
	case -59: return "CL_INVALID_OPERATION";
	case -60: return "CL_INVALID_GL_OBJECT";
	case -61: return "CL_INVALID_BUFFER_SIZE";
	case -62: return "CL_INVALID_MIP_LEVEL";
	case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
	case -64: return "CL_INVALID_PROPERTY";
	case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
	case -66: return "CL_INVALID_COMPILER_OPTIONS";
	case -67: return "CL_INVALID_LINKER_OPTIONS";
	case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

	// extension errors
	case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
	case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
	case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
	case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	default: return "Unknown OpenCL error";
	}
}

std::vector<cl_device_id> OpenCLPlatform::getMyDeviceIDs(){
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

cl_context OpenCLPlatform::createContext(cl_platform_id platform, cl_device_type device_type){
	if(getDeviceIDs(platform,device_type).size()){
		cl_context_properties props[3] = {CL_CONTEXT_PLATFORM,(cl_context_properties)platform,0};
		cl_context ret=clCreateContextFromType (props,device_type,nullptr,nullptr,&err);
		if(err==CL_SUCCESS){
			LOG(Runtime,notice) << "Using " << getDeviceIDs(platform,device_type).size() << " OpenCL device(s) from platform " << getPlatformName(platform);
			return ret;
		} else
			LOG(Runtime,error) << "Failed to create OpenCL context on platform " << getPlatformName(platform) << " error was " << getErrorString(err);
	} else {
		LOG(Runtime,error) << "No devices available for platform " << getPlatformName(platform);
	}
	return 0;
}

OpenCLPlatform::OpenCLPlatform(){
	/* Setup OpenCL environment. */
	std::pair<cl_platform_id,int> biggest;
	for(cl_platform_id p:getPlatformIDs()){
		std::size_t dev_cnt=getDeviceIDs(p).size();
		if(biggest.second<dev_cnt)
			biggest = {p,dev_cnt};
	}
	if(biggest.first){
		for(cl_device_id d:getDeviceIDs(biggest.first)){
			LOG(Runtime,info)
				<< "Selected device " << getDeviceName(d)
				<< " has max worker size " << getDeviceInfo<size_t>(d,CL_DEVICE_MAX_WORK_GROUP_SIZE)
				<< " and " << getDeviceInfo<cl_uint>(d,CL_DEVICE_MAX_COMPUTE_UNITS) << " computing units";
		}
		ctx=createContext(biggest.first);
	}
}

OpenCLPlatform::~OpenCLPlatform(){
	clReleaseContext( ctx );
}

bool OpenCLPlatform::good(){return ctx!=nullptr && err==CL_SUCCESS;}

cl_command_queue OpenCLPlatform::clCreateCommandQueue(){
	const cl_command_queue ret = ::clCreateCommandQueue( ctx, getMyDeviceIDs().front(), 0, &err );
	LOG_IF(err!=CL_SUCCESS,Runtime,error) << "clCreateCommandQueue failed with " << getErrorString(err);
	return ret;
}

cl_mem OpenCLPlatform::clCreateBuffer(cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret){
	return ::clCreateBuffer( ctx, flags, size, host_ptr, errcode_ret );
}

cl_int OpenCLPlatform::clEnqueueWriteBuffer(cl_command_queue queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t buffer_size, const void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event){
	return ::clEnqueueWriteBuffer(queue, buffer, blocking_write, offset, buffer_size, ptr, num_events_in_wait_list, event_wait_list, event );
}

OpenCLPlatform::operator cl_context(){return ctx;}

}
}
}