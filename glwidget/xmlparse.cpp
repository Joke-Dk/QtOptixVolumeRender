#include "xmlparse.h"
#include <iostream>
using namespace tinyxml2;

optix::float3 Chars2Float3( const char* str0)
{
	optix::float3 ret;
	std::string str( str0);
	std::string tmpStr;
	std::size_t pos1;
	// float - x
	pos1 = str.find_first_of(",");
	tmpStr = str.substr( 0, pos1);
	ret.x = atof(const_cast<const char *>(tmpStr.c_str()));
	str = str.substr(pos1+1);
	// float - y
	pos1 = str.find_first_of(",");
	tmpStr = str.substr( 0, pos1);
	ret.y = atof(const_cast<const char *>(tmpStr.c_str()));
	str = str.substr(pos1+1);
	// float - z
	pos1 = str.find_first_of(",");
	tmpStr = str.substr( 0, pos1);
	ret.z = atof(const_cast<const char *>(tmpStr.c_str()));
	return ret;
}

XmlParse::XmlParse(const char* xmlFilename):
	m_xmlFilename( xmlFilename)
{
}

void XmlParse::setup( optix::Context& optixCtx)
{
	XMLDocument doc;
	doc.LoadFile(m_xmlFilename);
	XMLElement *scene=doc.RootElement();
	XMLElement *element=scene->FirstChildElement();
	while(element)
	{
		ParseElementType( optixCtx, element);
		element = element->NextSiblingElement();
	}

}


void XmlParse::ParseElementType( optix::Context& optixCtx, XMLElement *element)
{
	std::string elementName = std::string(element->Name());
	if (elementName == std::string("sensor"))
	{
		XMLElement *childElement=element->FirstChildElement();
		while(childElement)
		{
			ParseSensor( optixCtx, childElement);
			childElement = childElement->NextSiblingElement();
		}
	}
}

void XmlParse::ParseSensor( optix::Context& optixCtx, tinyxml2::XMLElement *element)
{
	std::string elementName = std::string(element->Name());
	if (elementName == std::string("float"))
	{
		optixCtx[element->Attribute( "name")] ->setFloat( element->FloatAttribute( "value"));
	}
	else if( elementName == std::string("int"))
	{
		optixCtx[element->Attribute( "name")] ->setInt( element->IntAttribute( "value"));
	}
	else if( elementName == std::string("float3"))
	{
		optixCtx[element->Attribute( "name")] ->setFloat( Chars2Float3(element->Attribute( "value")));
	}
}