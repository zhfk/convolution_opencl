/*
 * main.c
 *
 *  Created on: 2016年10月12日
 *      Author: zhfk
 */
//For clarity,error checking has been omitted.
//#pragma warning( disable : 4996 )

#include "tool.h"
#include "gFreeImage.h"
using namespace std;
#define RESULT_LENTH 20
int main(int argc, char* argv[])
{
	double start_time,end_time;
	cl_uint numOfDevice;
	cl_event events[2];
	cl_program program;
	cl_int    status;
	/**Step 1: Getting platforms and choose an available one(first).*/
	cl_platform_id platform;
	cout<<"program running---->"<<endl;
	getPlatform(platform);
	/**Step 2:Query the platform and choose the first GPU device if has one.*/
	cl_device_id *devices = getCl_device_id(platform,numOfDevice);

	/**Step 3: Create context.*/
	cl_context context = clCreateContext(NULL, 1, devices, NULL, NULL, NULL);

	/**Step 4: Creating command queue associate with the context.*/
	cl_command_queue commandQueue = clCreateCommandQueue(context, *devices, 0, NULL);

	//此处添加读取二进制 kernel文件
	  // Create the program for all device. Use the first device as the
	  // representative device (assuming all device are of the same type).

	const char *cl_kernel_file="CL_convolution";
	string binary_file = getBoardBinaryFile(cl_kernel_file, devices[0]);
	printf("%-15s ===> %s \n","Using AOCX",binary_file.c_str());
	program= createProgramFromBinary(context, binary_file.c_str(), devices, numOfDevice);


	 // debug("Kernel execute scale calculate Matrix Multiple [%d x %d]",SIZE,SIZE);
	  // Build the program that was just created.
	status = clBuildProgram(program, 0, NULL, "", NULL, NULL);
	// Shows the log
	ShowBuildLog(program, devices);

	/**Step 7: Initial input,output for the host and create memory objects for the kernel*/
	//装入图像,
	//装入图像,
	const char* filename="../lenna.jpg";
	int* src_image=NULL;
	int* out_image=NULL;
	int image_width,image_height;
	int kernel_width=5,kernel_height=5;
	static int buf_kernel[] = {
	    1,1,1,1,1,
	    1,4,4,4,1,
	    1,4,12,4,1,
	    1,4,4,4,1,
	    1,1,1,1,1
	};
	gFreeImage img;
	if(!img.LoadImage_RGBA(filename))
	{
		//printf("can‘t load lenna.jpg\n");
		debug_msg(ERROR,"can't load %s\n",filename);
		exit(-1);
	}
	else
		src_image = img.getImageData_RGBA(image_width,image_height);

	int mem_size = img.imageSize;
	int kernel_size = kernel_width*kernel_height;

	int sum=0;
	for(int i=0;i<kernel_size;i++)
	{
		sum += buf_kernel[i];
	}

	debug_msg(INFO,"Kernel execute start...");
	start_time=getCurrentTimestamp();
	printf("%-15s ===> %.3f %s \n","start_time",start_time*1e3," Ms");
	//cout << "clCreateBuffer---------->" << endl << endl;
	cl_mem inputBuffer_a = clCreateBuffer(context, CL_MEM_READ_ONLY , mem_size*sizeof(int), NULL, NULL);
	cl_mem kernelBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY , kernel_size*sizeof(int), NULL, NULL);
	cl_mem outputBuffer_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, mem_size*sizeof(int), NULL, NULL);

	cl_event writeEvt[2];
	status = clEnqueueWriteBuffer (commandQueue,inputBuffer_a, CL_TRUE,0, mem_size*sizeof(int), (void *)src_image,0, NULL, &writeEvt[0]);
	//等待数据传输完成再继续往下执行
	status = clWaitForEvents(1,&writeEvt[0]);
	if (status != CL_SUCCESS)
	{
		//cout <<"Error: Waiting for kernel run to finish.(clWaitForEvents)"<<endl;
		debug_msg(status, "Waiting for clEnqueueWriteBuffer run to finish.(clWaitForEvents)");
		return 0;
	}
	status = clReleaseEvent(writeEvt[0]);
	status = clEnqueueWriteBuffer (commandQueue,kernelBuffer, CL_TRUE,0, kernel_size*sizeof(int), (void *)buf_kernel,0, NULL, &writeEvt[1]);
		//等待数据传输完成再继续往下执行
		status = clWaitForEvents(1,&writeEvt[1]);
		if (status != CL_SUCCESS)
		{
			//cout <<"Error: Waiting for kernel run to finish.(clWaitForEvents)"<<endl;
			debug_msg(status, "Waiting for clEnqueueWriteBuffer run to finish.(clWaitForEvents)");
			return 0;
		}
		status = clReleaseEvent(writeEvt[1]);
	/**Step 8: Create kernel object */
	//cout << "clCreateKernel---------->" << endl << endl;
	const char* KernelFunction="convolution";
	cl_kernel kernel = clCreateKernel(program, KernelFunction, NULL);

	/**Step 9: Sets Kernel arguments.*/
	//cout << "clSetKernelArg---------->" << endl << endl;
	//设置Kernel参数
	clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *)&inputBuffer_a);
	clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *)&outputBuffer_c);
	clSetKernelArg(kernel, 2, sizeof(cl_mem),  (void *)&kernelBuffer);
	clSetKernelArg(kernel, 3, sizeof(cl_int),  (void *)&sum);
	clSetKernelArg(kernel, 4, sizeof(cl_int), (void *)&image_width);
	clSetKernelArg(kernel, 5, sizeof(cl_int), (void *)&kernel_width);


	/**Step 10: Running the kernel.*/
	size_t globalThreads[] = {image_width, image_height};
	size_t localThreads[] = {32, 32}; // localx*localy应该是64的倍数
	//printf("global_work_size =(%d,%d), local_work_size=(16, 16)\n",W,H);
	printf("%-15s ===> <%d,%d> \n","globalworksize",globalThreads[0],globalThreads[1]);
	printf("%-15s ===> <%d,%d> \n","localworksize",localThreads[0],localThreads[1]);
	//	const size_t local_ws = 512;    // Number of work-items per work-group
	//	cl_event enentPoint;
	//cout << "clEnqueueNDRangeKernel---------->" << endl << endl;
	status = clEnqueueNDRangeKernel(commandQueue, kernel, MatrixDim, NULL, globalThreads, localThreads, 0, NULL, &events[0]);
	status = clWaitForEvents(1,&events[0]);
	if (status != CL_SUCCESS)
	{
		//cout <<"Error: Waiting for kernel run to finish.(clWaitForEvents)"<<endl;
		debug_msg(status, "Waiting for kernel run to finish.(clWaitForEvents)");
		return 0;
	}
	status = clReleaseEvent(events[0]);
	//计算kerenl执行时间
	//cl_ulong startTime, endTime;
	//clGetEventProfilingInfo(events[0], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
	//clGetEventProfilingInfo(events[0],  CL_PROFILING_COMMAND_END,sizeof(cl_ulong), &endTime, NULL);
	//cl_ulong kernelExecTimeNs = endTime-startTime;
	//printf("kernal exec time :%8.6f ms\n ", kernelExecTimeNs*1e-6 );
	//printf("%-15s ===> %.3f %s \n ", "ker_exec_time", kernelExecTimeNs*1e-6,"Ms");
	//将结果拷贝到主机端
	end_time = getCurrentTimestamp();
	//cout << "end_time :" << end_time*1e3<<" Ms" << endl;
	printf("%-15s ===> %.3f %s \n","end_time",end_time*1e3," Ms");
	//cout << "took time :" << ((end_time - start_time) * 1e3) << " Ms"<< endl;
	printf("%-15s ===> %.3f %s \n","ker_took_time",((end_time - start_time) * 1e3)," Ms");
	debug_msg(INFO,"Kernel execute finish !");

	/**Step 11: Read the cout put back to host memory.*/
	//cout << "clEnqueueReadBuffer---------->" << endl << endl;
	//out_image=(unsigned char*)malloc(mem_size);
	//status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, mem_size, out_image, 0, NULL, &events[1]);
	out_image = (int *) clEnqueueMapBuffer(commandQueue,outputBuffer_c,CL_TRUE,CL_MAP_READ,0,mem_size*sizeof(int),0,NULL, &events[1], NULL );
	status = clWaitForEvents(1, &events[1]);
	if (status != CL_SUCCESS)
	{
		//cout <<"Error: Waiting for read buffer call to finish. (clWaitForEvents)"<<endl;
		debug_msg(status, "Waiting for clEnqueueMapBuffer call to finish. (clWaitForEvents)");
		return 0;
	}
	status = clReleaseEvent(events[1]);
	const char *outfileName="FGPA_convolution_lenna.jpg";
	img.SaveImage_RGBA(outfileName,out_image);
    cl_event unmap_event;
	clEnqueueUnmapMemObject(commandQueue,outputBuffer_c,out_image,0,NULL,&unmap_event );
	status = clWaitForEvents(1, &unmap_event);
	if (status != CL_SUCCESS)
	{
		//cout <<"Error: Waiting for read buffer call to finish. (clWaitForEvents)"<<endl;
		debug_msg(status, "Waiting for clEnqueueUnmapMemObject call to finish. (clWaitForEvents)");
		return 0;
	}
	status = clReleaseEvent(unmap_event);

	debug_msg(INFO,"image %s saved in current directory",outfileName);
	/**Step 12: Clean the resources.*/
	//cout << "clRelease---------->" << endl << endl;
	status = clReleaseKernel(kernel);//*Release kernel.
	status = clReleaseProgram(program);    //Release the program object.
	status = clReleaseMemObject(inputBuffer_a);//Release mem object.
	status = clReleaseMemObject(kernelBuffer);//Release mem object.
	status = clReleaseMemObject(outputBuffer_c);
	status = clReleaseCommandQueue(commandQueue);//Release  Command queue.
	status = clReleaseContext(context);//Release context.

	if (devices)
	{
		free(devices);
		devices = NULL;
	}
	cout << "program over--------->" << endl << endl;
	return 0;
}

