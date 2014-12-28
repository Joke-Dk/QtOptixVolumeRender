#include "envmapSampler.h"
#include "discrete.h"
EnvironmentMap::EnvironmentMap( )	
{
}

void EnvironmentMap::setup( optix::Context& optixCtx)
{
	optix::Buffer _buffer = optixCtx["envmap"]->getTextureSampler()->getBuffer( 0u, 0u);
	optix::float4* _dataColor = (optix::float4*)_buffer->map();
	RTsize hdrWidth, hdrHeight;
	_buffer->getSize( hdrWidth, hdrHeight);
	int width = static_cast<int>(hdrWidth), height = static_cast<int>(hdrHeight);

	float* _data = new float [width*height];
	for( int i=0; i<width*height; ++i)
	{
		_data[ i] = luminanceCIE( make_float3( _dataColor[i]));
	}
	optixCtx["envmapWidth"]->setUint(optix::uint(width));
	optixCtx["envmapHeight"]->setUint(optix::uint(height));
	
	Distribution2D dist2(optixCtx, _data,static_cast<unsigned int>(width),static_cast<unsigned int>(height));
	optixCtx["cdfMarginal"]->set(dist2.pMarginal);
	optixCtx["cdfConditional"]->set(dist2.pConditionalV);
	_buffer->unmap();

	optix::Buffer pixelIsSampled = optixCtx->createBuffer(RT_BUFFER_INPUT_OUTPUT);
	pixelIsSampled->setFormat(RT_FORMAT_FLOAT);
	pixelIsSampled->setSize(width*height);
	optixCtx["gridFluence"]->setBuffer( pixelIsSampled );
}