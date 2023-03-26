#pragma once

#include <CL/cl.h>
#include <vector>
#include <fstream>
#include <chrono>
using namespace std;
class Filter
{
public:
	Filter()
	{
		numPlatforms = 0;
		cl_int err = clGetPlatformIDs(0, nullptr, &numPlatforms);
		if (err != CL_SUCCESS)
		{
			std::cout << "Filter ERROR1";
			return;
		}

		platforms = new cl_platform_id[numPlatforms];
		err = clGetPlatformIDs(numPlatforms, platforms, nullptr);
		if (err != CL_SUCCESS)
		{
			std::cout << "Filter ERROR2";
			return;
		}

		devices = new cl_device_id* [numPlatforms];
		numDevices = new cl_uint [numPlatforms];
		for (int i = 0; i < numPlatforms; ++i)
		{
			err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices[i]);
			if (err != CL_SUCCESS)
			{
				std::cout << "Filter ERROR3";
				return;
			}

			devices[i] = new cl_device_id[numDevices[i]];
			err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, numDevices[i], devices[i], nullptr);
			if (err != CL_SUCCESS)
			{
				std::cout << "Filter ERROR4";
				return;
			}
		}

		context = clCreateContext(nullptr, 1, &devices[0][0], nullptr, nullptr, &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Filter ERROR5";
			return;
		}

		commandQueue = clCreateCommandQueue(context, devices[0][0], 0, &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Filter ERROR6";
			return;
		}
	}
	~Filter()
	{
		delete[] platforms;
		delete[] numDevices;
		for (int i = 0; i < numPlatforms; ++i) 
			delete[] devices[i];
	}

	void showDevices()
	{
		std::cout << "Found " << numPlatforms << " OpenCL devices:" << std::endl;
		for (cl_uint i = 0; i < numPlatforms; ++i)
		{
			char platformName[1024];
			cl_int err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platformName), platformName, nullptr);
			if (err != CL_SUCCESS)
			{
				std::cout << "Error getting platform name: " << err << std::endl;
				return;
			}
			std::cout << " Platform " << i << ": " << platformName << std::endl;

			std::cout << "   Found " << numDevices[i] << " OpenCL devices:" << std::endl;
			for (cl_uint j = 0; j < numDevices[i]; ++j)
			{
				char deviceName[1024];
				err = clGetDeviceInfo(devices[i][j], CL_DEVICE_NAME, sizeof(deviceName), deviceName, nullptr);
				if (err != CL_SUCCESS)
				{
					std::cout << "    Error getting device name: " << err << std::endl;
					return;
				}
				std::cout << "    Device " << j << ": " << deviceName << std::endl;
			}
		}
	}

protected:
	cl_command_queue commandQueue = nullptr;
	cl_platform_id* platforms     = nullptr;
	cl_device_id** devices        = nullptr;
	cl_context context            = nullptr;
	cl_uint* numDevices           = nullptr;
	cl_uint numPlatforms          = 0;

};

class NegativeFilter: public Filter
{
public:
	NegativeFilter()
	{
		std::ifstream file("negativ.cl");
		if(!file)
		{
			std::cout << "Negative ERROR1" << std::endl;
			return;
		}
		std::string sourceKernel = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

		cl_int err;
		const char* sK = sourceKernel.c_str();
		cl_program program = clCreateProgramWithSource(context, 1, &sK, nullptr, &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR2" << std::endl;
			return;
		}

		clBuildProgram(program, 1, &devices[0][0], nullptr, nullptr, nullptr);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR3" << std::endl;
			return;
		}

		kernel = clCreateKernel(program, "Main", &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR4" << std::endl;
			return;
		}
	}

	std::vector<unsigned char> processing(std::vector<unsigned char>& input)
	{
		std::vector<unsigned char> output(input.size());

		cl_int err;
		cl_mem bufferIn = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, input.size() * sizeof(unsigned char), input.data(), &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR5" << std::endl;
			return output;
		}
		cl_mem bufferOut = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, output.size() * sizeof(unsigned char), output.data(), &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR52" << std::endl;
			return output;
		}

		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&bufferIn);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR6" << std::endl;
			return output;
		}
		err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&bufferOut);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR6" << std::endl;
			return output;
		}

		size_t globalSize[1]{ input.size() };
		size_t localSize[1]{ 20 };
		err = clEnqueueNDRangeKernel(commandQueue, kernel, 1, nullptr, globalSize, localSize, 0, nullptr, nullptr);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR7" << err << std::endl;
			return output;
		}

		err = clEnqueueReadBuffer(commandQueue, bufferOut, CL_TRUE, 0, output.size() * sizeof(unsigned char), &output[0], 0, nullptr, nullptr);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR8" << std::endl;
			return output;
		}

		clReleaseMemObject(bufferIn);
		clReleaseMemObject(bufferOut);

		return output;
	}

private:
	cl_kernel kernel = nullptr;

};


class MedianFilter: public Filter
{
public:
	MedianFilter()
	{
		std::ifstream file("median.cl");
		if (!file)
		{
			std::cout << "Median ERROR1" << std::endl;
			return;
		}
		std::string sourceKernel = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

		cl_int err;
		const char* sK = sourceKernel.c_str();
		cl_program program = clCreateProgramWithSource(context, 1, &sK, nullptr, &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR2" << std::endl;
			return;
		}

		clBuildProgram(program, 1, &devices[0][0], nullptr, nullptr, nullptr);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR3" << std::endl;
			return;
		}

		kernel = clCreateKernel(program, "Main", &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR4" << std::endl;
			return;
		}
	}

	std::vector<unsigned char> processing(std::vector<unsigned char>& input, unsigned int w, unsigned int h, unsigned int ker)
	{
		std::vector<unsigned char> output(input.size());

		cl_int err;
		cl_mem bufferIn = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, input.size() * sizeof(unsigned char), input.data(), &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR51" << std::endl;
			return output;
		}
		cl_mem bufferOut = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, output.size() * sizeof(unsigned char), output.data(), &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR52" << std::endl;
			return output;
		}

		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&bufferIn);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR61" << std::endl;
			return output;
		}
		err = clSetKernelArg(kernel, 1, sizeof(int), (void*)&w);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR62" << std::endl;
			return output;
		}
		err = clSetKernelArg(kernel, 2, sizeof(int), (void*)&h);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR63" << std::endl;
			return output;
		}
		err = clSetKernelArg(kernel, 3, sizeof(int), (void*)&ker);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR64" << std::endl;
			return output;
		}
		err = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&bufferOut);
		if (err != CL_SUCCESS)
		{
			std::cout << "Median ERROR65" << std::endl;
			return output;
		}

		size_t globalSize[2]{ w * 3, h };
		size_t localSize[2]{ 10, 10 };
		err = clEnqueueNDRangeKernel(commandQueue, kernel, 2, nullptr, globalSize, localSize, 0, nullptr, nullptr);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR7 " << err << std::endl;
			return output;
		}

		err = clEnqueueReadBuffer(commandQueue, bufferOut, CL_TRUE, 0, output.size() * sizeof(unsigned char), &output[0], 0, nullptr, nullptr);
		if (err != CL_SUCCESS)
		{
			std::cout << "Negative ERROR8" << std::endl;
			return output;
		}

		clReleaseMemObject(bufferIn);
		clReleaseMemObject(bufferOut);

		return output;
	}

private:
	cl_kernel kernel = nullptr;

};
