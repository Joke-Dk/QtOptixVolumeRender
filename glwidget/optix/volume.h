#pragma once
#include "helpers.h"
#include <OptixMesh.h>
#include <string>
#include <iostream>
class VolumeData
{
public:
	std::string UpdateFilename( std::string & filename);
	void UpdateID( int id);

	void setup(optix::Context& optixCtx, int kindVolume);
	int _id;
	optix::int3 _indexXYZ;
private:
	std::string _filename;
	std::string _filenameHead;
	std::string _filenameTail;
	std::string _filenamePath;
	void ReadKind0Pbrt(optix::Context& optixCtx);
	void ReadKind1Dat(optix::Context& optixCtx, int xyzKind=1);

	int yxz2xyz(int );
};