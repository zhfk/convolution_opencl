#include "gFreeImage.h"

gFreeImage::gFreeImage(void)
{
	FreeImage_Initialise();
	bitmap = NULL;
	imageData = NULL;
	imageSize=0;
	width = 0;
	height = 0;
	imagetype=FIF_UNKNOWN;
}
gFreeImage::~gFreeImage(void)
{
	if(bitmap)
		FreeImage_Unload(bitmap);
	if(imageData!=NULL)
		delete imageData;
	FreeImage_DeInitialise();
}


cl_bool gFreeImage::LoadImage_RGBA(const char *filename)
{
	imagetype = FIF_UNKNOWN;
	imagetype = FreeImage_GetFileType(filename,0);
		// 设法根据图片文件类型猜测图像格式
	if(imagetype == FIF_UNKNOWN)
		imagetype = FreeImage_GetFIFFromFilename(filename);
	if(imagetype == FIF_UNKNOWN)
	{
			//printf("Unknown image format\n");
		debug_msg(ERROR,"Unknown image format\n");
		return CL_FALSE;
	}
		// 检测是否FreeImage是否支持该文件格式
	if((imagetype != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(imagetype))
	{
			bitmap = FreeImage_Load(imagetype,filename,0);
			if(!bitmap)
			{
				//printf("can't load image\n");
				debug_msg(ERROR,"can't load image\n");
				return CL_FALSE;
			}
	}
	else
	{
			//printf("format can't be supported.\n");
			debug_msg(ERROR,"format can't be supported.\n");
			return CL_FALSE;
	}
	bitmap = FreeImage_ConvertTo32Bits(bitmap);

	int x,y;
	RGBQUAD m_rgb;
	this->width = FreeImage_GetWidth(bitmap);
	this->height = FreeImage_GetHeight(bitmap);
	this->imageSize=(this->width)*(this->height)*3;
	imageData = new int[this->imageSize];
	//获取图片数据
		//按RGBA格式保存到数组中
		for(y=0;y<height;y++)
		{
			for(x=0;x<width;x++)
			{
				//获取像素值
				FreeImage_GetPixelColor(bitmap,x,y,&m_rgb);

				//将RGB值存入数组
				imageData[y*width*3+x*3+2] = m_rgb.rgbRed;
				imageData[y*width*3+x*3+1] = m_rgb.rgbGreen;
				imageData[y*width*3+x*3+0] = m_rgb.rgbBlue;

			}
		}

		FreeImage_Unload(bitmap);
		return CL_TRUE;
}

int* gFreeImage::getImageData_RGBA(int& width,int& height)
{
	width=this->width;
	height=this->height;
	return this->imageData;
}

cl_bool gFreeImage::SaveImage_RGBA(const char *filename,int *buf_B)
{
	FIBITMAP* bitmap =FreeImage_Allocate(this->width,this->height,32,8,8,8);

	    for(int n=0;n<height;n++)
	    {
	        BYTE *bits =FreeImage_GetScanLine(bitmap,n);

	        for(int m=0;m<width;m++)
	        {
	            bits[0] = buf_B[width*3*n+m*3+0];
	            bits[1] = buf_B[width*3*n+m*3+1];
	            bits[2] = buf_B[width*3*n+m*3+2];
	            bits[3] = 255;
	            bits+=4;
	        }
	    }

	    //保存图片为PNG格式
	    if(false ==FreeImage_Save(FIF_PNG, bitmap,filename, PNG_DEFAULT))
	    {
	    	debug_msg(ERROR,"save image %s error !",filename);
	    	return CL_FALSE;
	    }

	    FreeImage_Unload(bitmap);
	    return CL_TRUE;
}
