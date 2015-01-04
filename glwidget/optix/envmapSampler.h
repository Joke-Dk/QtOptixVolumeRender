#pragma once
#include "Sampler.h"
#include <ObjLoader.h>
#include <ImageLoader.h>
class EnvironmentMap
{
public:
	EnvironmentMap(){}

	void setup(optix::Context&);

};