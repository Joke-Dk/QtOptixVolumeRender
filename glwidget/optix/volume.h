#pragma once
#include "Sampler.h"
#include <ObjLoader.h>
#include <string>
#include <iostream>
class VolumeData
{
public:
	std::string UpdateFilename( std::string & filename);
	void UpdateID( int id);

	void setup(optix::Context& optixCtx, int kindVolume, optix::int3& indexXYZ);
	int _id;
private:
	optix::int3 _indexXYZ;
	std::string _filename;
	std::string _filenameHead;
	std::string _filenameTail;
	std::string _filenamePath;
	void ReadKind0Pbrt(optix::Context& optixCtx);
	void ReadKind1Dat(optix::Context& optixCtx);
	void ReadKind1Dat2(optix::Context& optixCtx);

	int yxz2xyz(int );
};