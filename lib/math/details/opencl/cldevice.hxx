#ifndef OPENCL_HPP
#define OPENCL_HPP

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.h>
#else
	#include <CL/cl.h>
#endif

#include <string>
#include <vector>

namespace isis{
namespace math{
namespace _internal{


class OpenCLDevice{
private:
	cl_int err;
	cl_context ctx = 0;
public:
	static std::string getPlatformName(cl_platform_id platform);
	static std::string getDeviceName(cl_device_id device);
	static std::vector<cl_platform_id> getPlatformIDs();
	static std::vector<cl_device_id> getDeviceIDs(cl_platform_id platform,cl_device_type device_type=CL_DEVICE_TYPE_ALL);
	static std::string getErrorString(cl_int err);
	std::vector<cl_device_id> getMyDeviceIDs();
	cl_context createContext(cl_platform_id platform,cl_device_type  device_type=CL_DEVICE_TYPE_ALL);
	OpenCLDevice();
	~OpenCLDevice();
	bool good();

	cl_command_queue clCreateCommandQueue();

	cl_mem clCreateBuffer(cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret );
	cl_int clEnqueueWriteBuffer(cl_command_queue queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t buffer_size, const void *ptr, cl_uint num_events_in_wait_list=0, const cl_event *event_wait_list=nullptr, cl_event *event=nullptr);
	operator cl_context();
};

}
}
}

#endif // OPENCL_HPP
