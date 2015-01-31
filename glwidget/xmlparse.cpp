#include "xmlparse.h"
#include <iostream>
using namespace tinyxml2;

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
	else if( elementName == std::string("integer"))
	{
		optixCtx[element->Attribute( "name")] ->setFloat( element->IntAttribute( "value"));
	}
	else if( elementName == std::string("lookAt"))
	{

	}
}