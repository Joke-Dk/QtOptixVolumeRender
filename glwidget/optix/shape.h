#pragma once
#include "helpers.h"
#include <OptixMesh.h>
class Shape
{
public:
	Shape(){}
	optix::GeometryInstance getGeometryInstance();
	optix::GeometryGroup getGeometryGroup();
protected:
	optix::GeometryInstance _gi;
	optix::Geometry _geometry;
	optix::GeometryGroup _gg;
	virtual bool isGroups()=0;
	void init( optix::Context& optixCtx);
	void setGeometryInstance( );
	virtual void setup( optix::Context& optixCtx)=0;
};