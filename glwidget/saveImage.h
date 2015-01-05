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
	SaveImage(){}
	void Save( optix::Context& optixCtx, const std::string& filename=std::string("result01.png"));
};
