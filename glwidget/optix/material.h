#pragma once
#include "helpers.h"
#include <OptixMesh.h>

optix::Material DefineDiffuseMaterial( optix::Context& m_context);

optix::Material DefineGlassMaterial( optix::Context& m_context);

optix::Material DefineMirrorMaterial( optix::Context& m_context);

optix::Material DefineFogMaterial( optix::Context& m_context, int, float indexRefraction=1.f);

optix::Material makeMaterialPrograms( optix::Context& m_context, const std::string& filename, 
								   const std::string& ch_program_name,
								   const std::string& ah_program_name="" );
