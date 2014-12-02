#include "light.h"
using namespace optix;
Material DefineDiffuseLight( Context& m_context)
{
	Material diffuse_light = m_context->createMaterial();
	Program diffuse_em = m_context->createProgramFromPTXFile( my_ptxpath( "light.cu" ), "diffuseEmitter" );
	diffuse_light->setClosestHitProgram( 0, diffuse_em );

	return diffuse_light;
}
