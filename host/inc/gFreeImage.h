#ifndef _GFREEIMAGE_H_
#define _GFREEIMAGE_H_
#include <cstdlib>
#include <cstdio>
#include "FreeImage.h"
#include "tool.h"
class gFreeImage
{
public:
	gFreeImage(void);
	~gFreeImage(void);
public:
    FIBITMAP *bitmap;
    int imageSize;
	int *imageData;
	FREE_IMAGE_FORMAT imagetype;
	int width;
	int height;
	cl_bool LoadImage_RGBA(const char *filename);
	int* getImageData_RGBA(int& width,int& height);
	cl_bool SaveImage_RGBA(const char *filename,int *buf_B);
};
#endif
