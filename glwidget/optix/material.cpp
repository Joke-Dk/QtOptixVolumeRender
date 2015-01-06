#include "material.h"
using namespace optix;
Material DefineDiffuseMaterial( Context& m_context)
{
	Material& diffuseMaterial = makeMaterialPrograms( m_context, "diffuse.cu", "diffuse", "shadow");

	diffuseMaterial["diffuse_color"] ->setFloat(1.f,1.f,1.f);
	return diffuseMaterial;
}

Material DefineGlassMaterial( Context& m_context)
{
	// Set up glass material
	Material& glassMaterial = makeMaterialPrograms( m_context, "glass.cu", "glass_fresnel", "glass_any_hit_shadow");

	bool m_green_glass = 0;
	glassMaterial["index_of_refraction"  ]->setFloat( 1.41f );
	glassMaterial["glass_color"  ]->setFloat( 1.f,1.f,1.f );
	glassMaterial["importance_cutoff"  ]->setFloat( 0.01f );
	glassMaterial["cutoff_color"       ]->setFloat( 0.2f, 0.2f, 0.2f );
	glassMaterial["fresnel_exponent"   ]->setFloat( 4.0f );
	glassMaterial["fresnel_minimum"    ]->setFloat( 0.1f );
	glassMaterial["fresnel_maximum"    ]->setFloat( 1.0f );
	glassMaterial["refraction_index"   ]->setFloat( 1.4f );
	glassMaterial["refraction_color"   ]->setFloat( 0.99f, 0.99f, 0.99f );
	glassMaterial["reflection_color"   ]->setFloat( 0.99f, 0.99f, 0.99f );
	glassMaterial["refraction_maxdepth"]->setInt( 10 );
	glassMaterial["reflection_maxdepth"]->setInt( 5 );
	float3 extinction = m_green_glass ? make_float3(.80f, .89f, .75f) : make_float3(1);
	glassMaterial["extinction_constant"]->setFloat( log(extinction.x), log(extinction.y), log(extinction.z) );
	glassMaterial["shadow_attenuation"]->setFloat( 1.0f, 1.0f, 1.0f );

	return glassMaterial;
}

Material DefineMirrorMaterial( Context& m_context)
{
	// Set up glass material
	Material& mirrorMaterial = makeMaterialPrograms( m_context, "mirror.cu", "mirror_fresnel", "shadow");

	return mirrorMaterial;
}


optix::Material DefineFogMaterial( optix::Context& m_context, int kindBound)
{
	// Set up glass material
	Material& fogMaterial = makeMaterialPrograms( m_context, "fog.cu", "fog__closest_hit_radiance", "fog_shadow");
	fogMaterial["glass_color"] ->setFloat(  1.f,1.f,1.f);
	fogMaterial["index_of_refraction"]->setFloat(1.f);
	fogMaterial["fresnel_exponent"   ]->setFloat( 4.0f );
	fogMaterial["fresnel_minimum"    ]->setFloat( 0.1f );
	fogMaterial["fresnel_maximum"    ]->setFloat( 1.0f );
	fogMaterial["boundMaterial"    ]->setInt( kindBound );
	
	return fogMaterial;
}

Material makeMaterialPrograms( Context& m_context, const std::string& filename, 
							  const std::string& ch_program_name,
							  const std::string& ah_program_name )
{
	Material material = m_context->createMaterial();
	Program ch_program = m_context->createProgramFromPTXFile( my_ptxpath( filename), ch_program_name );
	material->setClosestHitProgram( 0, ch_program );
	if( ah_program_name!="")
	{
		Program ah_program = m_context->createProgramFromPTXFile( my_ptxpath( filename), ah_program_name );
		material->setAnyHitProgram( 1, ah_program );
	}

	return material;
}

