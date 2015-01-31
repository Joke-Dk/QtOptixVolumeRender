#pragma once
#include "thirdParty/tinyxml2.h"
#include "optix/helpers.h"
#include <ObjLoader.h>
#include <string>
#include <map>

class XmlParse
{
public:
	XmlParse( const char* xmlFilename);
	void setup( optix::Context& optixCtx);
private:
	void ParseElementType( optix::Context& optixCtx, tinyxml2::XMLElement *element);
	void ParseSensor( optix::Context& optixCtx, tinyxml2::XMLElement *element);
	std::map< std::string, RTvariable> sensorMap;
	const char* m_xmlFilename;
};