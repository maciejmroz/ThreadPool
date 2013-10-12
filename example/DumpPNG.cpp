#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "png.h"

void dumpPNG(int width, int height, unsigned char *buffer)
{
	//write png
	FILE *fp=fopen("output.png","wb");
	if(!fp)
	{
		return;
	}
	png_structp png_ptr=NULL;
	png_infop info_ptr=NULL;

	png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if(!png_ptr)
	{
		fclose(fp);
		return;
	}
	info_ptr=png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		png_destroy_write_struct(&png_ptr,png_infopp_NULL);
		fclose(fp);
		return;
	}
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr,&info_ptr);
		fclose(fp);
		return;
	}
	png_init_io(png_ptr,fp);
	png_set_IHDR(png_ptr,info_ptr,width,
		height,8,
		PNG_COLOR_TYPE_RGB_ALPHA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	unsigned char **rows=(unsigned char**)malloc(height*sizeof(unsigned char*));
	unsigned int row_byte_size=width*4;
	int i=0;
	for(i=0;i<height;i++)
	{
		rows[i]=buffer+i*row_byte_size;
	}
	png_set_rows(png_ptr,info_ptr,rows);
	png_write_png(png_ptr,info_ptr,0,png_voidp_NULL);
	free(rows);
	png_destroy_write_struct(&png_ptr,&info_ptr);
	fclose(fp);
}
