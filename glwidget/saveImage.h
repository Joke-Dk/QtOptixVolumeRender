#pragma once
#include "optix/helpers.h"
#include <OptixMesh.h>
#include <iostream>
#include <cassert>
#include"thirdParty/FreeImage.h"

#ifdef DEBUG
#pragma comment(lib,"thirdParty/FreeImaged.lib")
#else
#pragma comment(lib,"thirdParty/FreeImage.lib")
#endif

class SaveImage
{
public:
	SaveImage();
	void Save( optix::Context& optixCtx, int id=0);
	std::string pathHead;
};
