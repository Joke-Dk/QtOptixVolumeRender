#pragma once
#include "Sampler.h"
#include <ObjLoader.h>
#include <string>
#include <iostream>
class VolumeData
{
public:
	VolumeData(){}

	void setup(optix::Context& optixCtx, int kindVolume, const std::string & filename, optix::int3& indexXYZ);
private:
	optix::int3 _indexXYZ;
	void ReadKind0Pbrt(optix::Context& optixCtx, const std::string & filename);
	void ReadKind1Dat(optix::Context& optixCtx, const std::string & filename);
};