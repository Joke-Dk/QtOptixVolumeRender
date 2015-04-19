#pragma once
#include "helpers.h"
#include <OptixMesh.h>
#include <ImageLoader.h>
class EnvironmentMap
{
public:
	EnvironmentMap(){}

	void setup(optix::Context&);

};