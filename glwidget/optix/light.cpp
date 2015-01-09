#include "light.h"
using namespace optix;
Material DefineDiffuseLight( Context& m_context)
{
	Material diffuse_light = m_context->createMaterial();
	Program diffuse_em = m_context->createProgramFromPTXFile( my_ptxpath( "light.cu" ), "diffuseEmitter" );
	diffuse_light->setClosestHitProgram( 0, diffuse_em );
	//Program diffuse_em_shadow = m_context->createProgramFromPTXFile( my_ptxpath( "light.cu" ), "diffuseEmitterShadow" );
	//diffuse_light->setAnyHitProgram( 1, diffuse_em_shadow );
	return diffuse_light;
}
