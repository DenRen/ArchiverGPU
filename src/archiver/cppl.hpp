#pragma once

/*
 * Common OpenCL C++ wrappers
*/

#define CL_HPP_TARGET_OPENCL_VERSION 220
#define CL_HPP_ENABLE_EXCEPTIONS

#include <CL/opencl.hpp>
#include <iostream>
#include <vector>

namespace cppl {

class DeviceProvider {
    std::string opencl_version_;

    cl::Platform default_platform_;
    cl::Device default_device_;

public:
    DeviceProvider (cl_device_type device_type = CL_DEVICE_TYPE_GPU);

    cl::Platform getDefaultPlatform () const;
    cl::Device getDefaultDevice () const;

    std::string getDefaultPlatformName () const;
    std::string getDefaultDeviceName () const;

    static std::string dumpAll ();

private:
    void initDefaultDeviceAndPlatform (const std::vector <std::string>& vendorPriority,
                                       cl_device_type device_type); // Device type (CPU, GPU, ...)
    bool setDefaultDeviceAndPlatformByVendorPriority (
        const std::vector <std::tuple <std::string, cl::Platform, cl::Device>>& vendorDevices,
        const std::vector <std::string>& vendorPriority
    );
}; // class DeviceProvider

class ClAccelerator {
protected:
    cl::Device device_;
    cl::Context context_;
    cl::CommandQueue cmd_queue_;
    cl::Program program_;

    template <typename T>
    cl::Buffer
    sendBuffer (const std::vector <T>& vec,
                cl_mem_flags flags = CL_MEM_READ_ONLY,
                cl_bool blocking = CL_FALSE)
    {
        const auto size_in_bytes = sizeof (T) * vec.size ();
        cl::Buffer buf (context_, flags, size_in_bytes);
        cmd_queue_.enqueueWriteBuffer (buf, blocking, 0,
                                       size_in_bytes, vec.data (),
                                       nullptr, nullptr);

        return buf;
    }

public:
    ClAccelerator (const cl::Device& device,
                   const std::string& source,
                   cl::QueueProperties cmd_queue_prop = cl::QueueProperties::None);
}; // class ClAccelerator

cl::Program
buildProgram (cl::Context context,
              std::string name_kernel_func);

const std::string_view getErrorString (cl_int error);
void printError (cl::Error& error);

} // namespace cppl