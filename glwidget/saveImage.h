#pragma once
#include "optix/Sampler.h"
#include <ObjLoader.h>
#include <iostream>
#include <cassert>
#include"thirdParty/FreeImage.h"
#pragma comment(lib,"thirdParty/FreeImage.lib")

class SaveImage
{
public:
	SaveImage();
	void Save( optix::Context& optixCtx, int id=0);
	std::string pathHead;
};
