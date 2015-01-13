#pragma once
#include "Sampler.h"
#include <ObjLoader.h>
class Shape
{
private:
	optix::GeometryInstance _gi;
public:
	Shape(){}
	void setGeometryInstance( optix::GeometryInstance gi);
	optix::GeometryInstance getGeometryInstance();
};