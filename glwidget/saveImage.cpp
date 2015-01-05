#include "saveImage.h"

void SaveImage::Save( optix::Context& optixCtx, const std::string& filename)
{
	optix::Buffer buffer = optixCtx["output_buffer"]->getBuffer();
	optix::uint buffer_width, buffer_height;
	buffer->getSize( buffer_width, buffer_height );
	optix::float4* cpuBuffer=(optix::float4*)buffer->map();

	//////////////////////////////////////////////////////////////////////////
	// FreeImage lib operation
	FreeImage_Initialise(TRUE);
	FIBITMAP *dib = FreeImage_Allocate(buffer_width, buffer_height, 32, FI_RGBA_RED_MASK,
		FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
	int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
	for(int y = 0; y < FreeImage_GetHeight(dib); ++y) 
	{
		BYTE *bits = FreeImage_GetScanLine(dib, y);
		for(int x = 0; x < FreeImage_GetWidth(dib); ++x) 
		{
			// Set pixel color to green with a transparency of 128
			bits[FI_RGBA_RED] = 255.f*cpuBuffer->x;
			bits[FI_RGBA_GREEN] = 255.f*cpuBuffer->y;
			bits[FI_RGBA_BLUE] = 255.f*cpuBuffer->z;
			bits[FI_RGBA_ALPHA] = 255.f*(1.f-cpuBuffer->w);
			// jump to next pixel
			bits += bytespp;
			cpuBuffer++;
		}
	}
	buffer->unmap();
	if( dib)
	{
		FreeImage_Save(FIF_PNG, dib, const_cast<const char *>(filename.c_str()), PNG_DEFAULT);
		std::cout<<"Save Image File Success!"<<std::endl;
	}
	else
	{
		std::cout<<"Save Image File Failure!"<<std::endl;
	}
	FreeImage_Unload(dib);
	FreeImage_DeInitialise();
}