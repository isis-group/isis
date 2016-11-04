#ifndef OPENCL_HPP
#define OPENCL_HPP

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.h>
#else
	#include <CL/cl.h>
#endif

#include <string>
#include <vector>
#include "../../common.hpp"

namespace isis{
namespace math{
namespace _internal{

class OpenCLPlatform{
private:
	cl_int err;
	cl_context ctx = 0;
public:
	static std::string getErrorString(cl_int err);

	std::vector<cl_device_id> getMyDeviceIDs();
	cl_context createContext(cl_platform_id platform,cl_device_type  device_type=CL_DEVICE_TYPE_ALL);
	OpenCLPlatform();
	~OpenCLPlatform();
	bool good();

	cl_command_queue clCreateCommandQueue();

	cl_mem clCreateBuffer(cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret );
	cl_int clEnqueueWriteBuffer(cl_command_queue queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t buffer_size, const void *ptr, cl_uint num_events_in_wait_list=0, const cl_event *event_wait_list=nullptr, cl_event *event=nullptr);
	operator cl_context();
};

std::string getPlatformName(cl_platform_id platform);
std::string getDeviceName(cl_device_id device);
std::vector<cl_platform_id> getPlatformIDs();
std::vector<cl_device_id> getDeviceIDs(cl_platform_id platform,cl_device_type device_type=CL_DEVICE_TYPE_ALL);

template<typename T> T getDeviceInfo(cl_device_id device,cl_device_info info){
	T ret;
	const cl_int err=clGetDeviceInfo(device, info, sizeof(ret), &ret, NULL);
	LOG_IF(err!=CL_SUCCESS,Runtime,error) << "clGetDeviceInfo failed with " << OpenCLPlatform::getErrorString(err);
	return ret;
}
template<typename T> T getPlatformInfo(cl_platform_id platform,cl_platform_info info){
	T ret;
	const cl_int err=clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(ret), ret, NULL);
	LOG_IF(err!=CL_SUCCESS,Runtime,error) << "clGetPlatformInfo failed with " << OpenCLPlatform::getErrorString(err);
	return ret;
}

}
}
}

#endif // OPENCL_HPP
