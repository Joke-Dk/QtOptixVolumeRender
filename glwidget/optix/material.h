#pragma once

#include "Sampler.h"
#include <ObjLoader.h>

optix::Material DefineDiffuseMaterial( optix::Context& m_context);

optix::Material DefineGlassMaterial( optix::Context& m_context);

optix::Material makeMaterialPrograms( optix::Context& m_context, const std::string& filename, 
								   const std::string& ch_program_name,
								   const std::string& ah_program_name );
