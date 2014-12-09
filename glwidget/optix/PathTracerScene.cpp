//------------------------------------------------------------------------------
//
// PathCamera.cpp: render cornell box using path tracing.
//
//------------------------------------------------------------------------------

#include "PathTracerScene.h"
#include "material.h"
#include "light.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <PPMLoader.h>
//#include "random.h"
//#include "helpers.h"
#include <ImageLoader.h>
using namespace std;



using namespace optix;
void PathTracerScene::updateParameter( std::string str, float value)
{ 
	m_context[str.c_str()]->setFloat(value); 
}

void PathTracerScene::initScene( InitialCameraData& camera_data )
{
	m_context->setPrintEnabled(true);


	m_context->setRayTypeCount( 3 );
	m_context->setEntryPointCount( 1 );
	m_context->setStackSize( 1800 );

	m_context["scene_epsilon"]->setFloat( 1.e-3f );
	m_context["max_depth"]->setUint(m_max_depth);
	m_context["pathtrace_ray_type"]->setUint(0u);
	m_context["pathtrace_shadow_ray_type"]->setUint(1u);
	m_context["pathtrace_bsdf_shadow_ray_type"]->setUint(2u);
	m_context["rr_begin_depth"]->setUint(m_rr_begin_depth);


	// Setup output buffer
	Variable output_buffer = m_context["output_buffer"];
	Buffer buffer = createOutputBuffer( RT_FORMAT_FLOAT4, m_width, m_height );
	output_buffer->set(buffer);


	// Set up camera
	camera_data = InitialCameraData( make_float3( 0.0f, 0.0f, 30.0f ), // eye
		make_float3( 0.f, 0.0f, 0.0f ),    // lookat
		make_float3( 0.0f, 1.0f,  0.0f ),       // up
		45.0f );                                // vfov

	// Declare these so validation will pass
	m_context["eye"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
	m_context["U"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
	m_context["V"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
	m_context["W"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );

	m_context["sqrt_num_samples"]->setUint( m_sqrt_num_samples );
	m_context["bad_color"]->setFloat( 0.0f, 1.0f, 0.0f );
	m_context["bg_color"]->setFloat( make_float3(0.0f) );

	// Setup programs
	std::string ptx_path = my_ptxpath("PathCamera.cu" );
	Program ray_gen_program = m_context->createProgramFromPTXFile( ptx_path, "pathtrace_camera" );
	m_context->setRayGenerationProgram( 0, ray_gen_program );
	Program exception_program = m_context->createProgramFromPTXFile( ptx_path, "exception" );
	m_context->setExceptionProgram( 0, exception_program );
	m_context->setMissProgram( 0, m_context->createProgramFromPTXFile( ptx_path, "miss" ) );

	m_context["frame_number"]->setUint(1);

	// Index of sampling_stategy (BSDF, light, MIS)
	m_sampling_strategy = 0;
	m_context["sampling_stategy"]->setInt(m_sampling_strategy);

	// Create scene geometry

	createEnvironmentScene(1);

	// Finalize
	m_context->validate();
	m_context->compile();
}

bool PathTracerScene::keyPressed( unsigned char key, int x, int y )
{
	return false;
}

void PathTracerScene::trace( const RayGenCameraData& camera_data )
{
	m_context["eye"]->setFloat( camera_data.eye );
	m_context["U"]->setFloat( camera_data.U );
	m_context["V"]->setFloat( camera_data.V );
	m_context["W"]->setFloat( camera_data.W );

	Buffer buffer = m_context["output_buffer"]->getBuffer();
	RTsize buffer_width, buffer_height;
	buffer->getSize( buffer_width, buffer_height );

	if( m_camera_changed ) {
		m_camera_changed = false;
		m_frame = 1;
	}

	m_context["frame_number"]->setUint( m_frame++ );

	m_context->launch( 0,static_cast<unsigned int>(buffer_width),static_cast<unsigned int>(buffer_height));
}

//-----------------------------------------------------------------------------

Buffer PathTracerScene::getOutputBuffer()
{
	return m_context["output_buffer"]->getBuffer();
}

GeometryGroup PathTracerScene::createObjloader( const std::string& path, const optix::Matrix4x4 m0, const Material& material0)
{
	GeometryGroup geomgroup = m_context->createGeometryGroup();
	//std::string prog_path = std::string(sutilSamplesPtxDir()) + "/glass_generated_triangle_mesh_iterative.cu.ptx";
	Program mesh_intersect = m_context->createProgramFromPTXFile( my_ptxpath( "TriangleMesh.cu" ), "mesh_intersect"  );
	ObjLoader objloader0( (path).c_str(), m_context, geomgroup, material0 );
	objloader0.setIntersectProgram( mesh_intersect );
	objloader0.load( m0 );
	return geomgroup;
}

GeometryInstance PathTracerScene::createParallelogram( const float3& anchor,
													  const float3& offset1,
													  const float3& offset2)
{
	Geometry parallelogram = m_context->createGeometry();
	parallelogram->setPrimitiveCount( 1u );
	parallelogram->setIntersectionProgram( m_pgram_intersection );
	parallelogram->setBoundingBoxProgram( m_pgram_bounding_box );

	float3 normal = normalize( cross( offset1, offset2 ) );
	float d = dot( normal, anchor );
	float4 plane = make_float4( normal, d );

	float3 v1 = offset1 / dot( offset1, offset1 );
	float3 v2 = offset2 / dot( offset2, offset2 );

	parallelogram["plane"]->setFloat( plane );
	parallelogram["anchor"]->setFloat( anchor );
	parallelogram["v1"]->setFloat( v1 );
	parallelogram["v2"]->setFloat( v2 );

	GeometryInstance gi = m_context->createGeometryInstance();
	gi->setGeometry(parallelogram);
	return gi;
}

GeometryInstance PathTracerScene::createSphere( const float3& p, float r)
{
	Geometry sphere = m_context->createGeometry();
	sphere->setPrimitiveCount( 1u );
	sphere->setBoundingBoxProgram( m_context->createProgramFromPTXFile( my_ptxpath("sphere.cu" ), "bounds" ) );
	sphere->setIntersectionProgram( m_context->createProgramFromPTXFile( my_ptxpath( "sphere.cu" ), "robust_intersect" ) );
	sphere["sphere"]->setFloat( p.x, p.y, p.z, r );
	GeometryInstance gi = m_context->createGeometryInstance();
	gi->setGeometry(sphere);
	return gi;
}

//GeometryInstance PathTracerScene::createLightParallelogram( const float3& anchor,
//                                                            const float3& offset1,
//                                                            const float3& offset2,
//                                                            int lgt_instance)
//{
//  Geometry parallelogram = m_context->createGeometry();
//  parallelogram->setPrimitiveCount( 1u );
//  parallelogram->setIntersectionProgram( m_pgram_intersection );
//  parallelogram->setBoundingBoxProgram( m_pgram_bounding_box );
//
//  float3 normal = normalize( cross( offset1, offset2 ) );
//  float d = dot( normal, anchor );
//  float4 plane = make_float4( normal, d );
//
//  float3 v1 = offset1 / dot( offset1, offset1 );
//  float3 v2 = offset2 / dot( offset2, offset2 );
//
//  parallelogram["plane"]->setFloat( plane );
//  parallelogram["anchor"]->setFloat( anchor );
//  parallelogram["v1"]->setFloat( v1 );
//  parallelogram["v2"]->setFloat( v2 );
//  parallelogram["lgt_instance"]->setInt( lgt_instance );
//
//  GeometryInstance gi = m_context->createGeometryInstance();
//  gi->setGeometry(parallelogram);
//  return gi;
//}

void PathTracerScene::setMaterial( GeometryInstance& gi,
								  Material material,
								  const std::string& color_name,
								  const float3& color)
{
	gi->addMaterial(material);
	gi[color_name]->setFloat(color);
}




void PathTracerScene::createEnvironmentScene(int sceneKind)
{
	// create geometry instances
	std::vector<GeometryInstance> gis;

	// Generate material
	Material& areaMaterial = DefineDiffuseLight( m_context);
	Material& diffuseMaterial = DefineDiffuseMaterial( m_context);
	Material &fogMaterial = DefineFogMaterial( m_context);
	Material &glassMaterial = DefineGlassMaterial( m_context);
	Material &mirrorMaterial = DefineMirrorMaterial( m_context);
	m_pgram_bounding_box = m_context->createProgramFromPTXFile( my_ptxpath( "parallelogram.cu" ), "bounds" );
	m_pgram_intersection = m_context->createProgramFromPTXFile( my_ptxpath( "parallelogram.cu" ), "intersect" );

	// Create shadow group (no light)
	GeometryGroup shadow_group = m_context->createGeometryGroup(gis.begin(), gis.end());
	shadow_group->setAcceleration( m_context->createAcceleration("Bvh","Bvh") );
	m_context["top_shadower"]->set( shadow_group );

	//////////////////////////////////////////////////////////////////////////
	// global parameter setting
	float alpha_value = 0.95f;
	float sigma_t=5.f;//0.1f;
	int isCurve = 1;

	if (sceneKind == 0)
	{
		sigma_t = 2.f;
		alpha_value = 0.85f;
		isCurve = 0;
	}

	int index_x = 100;
	int index_y = 100;
	int index_z = 40;	
	m_context["alpha_value"    ]->setFloat( alpha_value );
	m_context["index_x" ]->setInt(index_x );
	m_context["index_y" ]->setInt(index_y );
	m_context["index_z" ]->setInt(index_z );  
	m_context["sigma_t"    ]->setFloat( sigma_t );
	m_context["isCurve"]->setInt(isCurve);

	const float3 white = make_float3( 0.8f, 0.8f, 0.8f );
	const float3 black = make_float3( 0.2f, 0.2f, 0.2f );
	const float3 green = make_float3( 0.05f, 0.3f, 0.05f );
	const float3 red   = make_float3( 0.8f, 0.05f, 0.05f );
	

	if(sceneKind==1)
	{
		m_context->setMissProgram( 0, m_context->createProgramFromPTXFile( my_ptxpath("PathCamera.cu" ), "envmap_miss" ) );
		const float3 default_color = make_float3(1.0f, 1.0f, 1.0f);

		std::string envmapPath = "optix/hdr/";
		envmapPath += "CedarCity.hdr";//	DH037LL.hdr		grace_ll.hdr	CedarCity.hdr	octane_studio4.hdr
		m_context["envmap"]->setTextureSampler( loadTexture( m_context, envmapPath, default_color) );
	}

	//////////////////////////////////////////////////////////////////////////
	// Light buffer
	float3 light_em = make_float3( 15.0f, 15.0f, 15.0f );
	if (sceneKind==0)
	{
		light_em *= 2.f;
	}
	else
	{
		light_em *= 0.f;
	}
	ParallelogramLight light;
	light.corner   = make_float3( 2.0f, 10.499f, -2.0f);
	light.v1       = make_float3( -4.0f, 0.0f, 0.0f);
	light.v2       = make_float3( 0.0f, 0.0f, 4.0f);
	light.normal   = normalize( cross(light.v1, light.v2) );
	light.emission = light_em;

	Buffer light_buffer = m_context->createBuffer( RT_BUFFER_INPUT );
	light_buffer->setFormat( RT_FORMAT_USER );
	light_buffer->setElementSize( sizeof( ParallelogramLight ) );
	light_buffer->setSize( 1u );
	memcpy( light_buffer->map(), &light, sizeof( light ) );
	light_buffer->unmap();
	m_context["lights"]->setBuffer( light_buffer );
	if (fmaxf(light_em)>0.f)
	{
		// Light shape
		gis.push_back( createParallelogram( light.corner,
			light.v1,
			light.v2 ) );
		setMaterial(gis.back(), areaMaterial, "emission_color", light_em);
	}
	
	
	//////////////////////////////////////////////////////////////////////////
	// cornel wall
	float3 p0 = make_float3(-10.5f, -10.5f, -10.5f);
	float3 p1 = make_float3(10.5f, 10.5f, 10.5f);
	float3 dp = p1-p0;
	if(sceneKind==0)
	{
		//floor
		gis.push_back( createParallelogram( p0, make_float3( 0.0f, 0.0f, dp.z),make_float3( dp.x, 0.0f, 0.0f) ) );
		setMaterial(gis.back(), diffuseMaterial, "diffuse_color", white);
		//ceiling
		gis.push_back( createParallelogram( p1, -make_float3( 0.0f, 0.0f, dp.z),-make_float3( dp.x, 0.0f, 0.0f) ) );
		setMaterial(gis.back(), diffuseMaterial, "diffuse_color", white);
		//left
		gis.push_back( createParallelogram( p0, make_float3( 0.0f, 0.0f, dp.z),make_float3( 0.f , dp.y, 0.0f) ) );
		setMaterial(gis.back(), diffuseMaterial, "diffuse_color", green);
		//right
		gis.push_back( createParallelogram( p1, -make_float3( 0.0f, 0.0f, dp.z),-make_float3( 0.f , dp.y, 0.0f) ) );
		setMaterial(gis.back(), diffuseMaterial, "diffuse_color", red);
		//behind
		gis.push_back( createParallelogram( p0, make_float3( dp.x, 0.0f, 0.f),make_float3( 0.f , dp.y, 0.0f) ) );
		setMaterial(gis.back(), diffuseMaterial, "diffuse_color", white);
		//front
		//gis.push_back( createParallelogram( p1, -make_float3( dp.x, 0.0f, 0.f), -make_float3( 0.f , dp.y, 0.0f) ) );
		//setMaterial(gis.back(), diffuseMaterial, "diffuse_color", white);
	}
	//////////////////////////////////////////////////////////////////////////
	// fog
	p0 = make_float3(-10.49f, -10.49f, -4.f);
	p1 = make_float3(10.49f, 10.49f, 4.f);
	m_context["P0"]->setFloat(p0.x, p0.y, p0.z );
	m_context["P1"]->setFloat(p1.x, p1.y, p1.z );
	dp = p1-p0;
	//floor
	gis.push_back( createParallelogram( p0, make_float3( 0.0f, 0.0f, dp.z),make_float3( dp.x, 0.0f, 0.0f) ) );
	setMaterial(gis.back(), fogMaterial, "diffuse_color", white);
	//ceiling
	gis.push_back( createParallelogram( p1, -make_float3( 0.0f, 0.0f, dp.z),-make_float3( dp.x, 0.0f, 0.0f) ) );
	setMaterial(gis.back(), fogMaterial, "diffuse_color", white);
	//left
	gis.push_back( createParallelogram( p0, make_float3( 0.0f, 0.0f, dp.z),make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis.back(), fogMaterial, "diffuse_color", white);
	//right
	gis.push_back( createParallelogram( p1, -make_float3( 0.0f, 0.0f, dp.z),-make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis.back(), fogMaterial, "diffuse_color", white);
	//behind
	gis.push_back( createParallelogram( p0, make_float3( dp.x, 0.0f, 0.f),make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis.back(), fogMaterial, "diffuse_color", white);
	//front
	gis.push_back( createParallelogram( p1, -make_float3( dp.x, 0.0f, 0.f), -make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis.back(), fogMaterial, "diffuse_color", white);
	//load volume data
	int index_N = index_x*index_y*index_z;
	optix::Buffer vol_data = m_context->createBuffer(RT_BUFFER_INPUT);
	vol_data->setFormat(RT_FORMAT_FLOAT);
	vol_data->setSize(index_N);
	float* temp_data = (float*)vol_data->map();
	//read .vol file
	char* filename = "optix/volume/density_render.70.pbrt";
	FILE* fin;
	fopen_s(&fin, filename, "r");
	if (!fin)
	{
		std::cerr << "Could not load Volume file \n";
		exit(1);
	}
	//float* m_data = new float[index_N];
	for(int i=0; i<index_N; i++)
		fscanf_s(fin,"%f",&temp_data[i]);
	vol_data->unmap();
	m_context["volume_density"]->setBuffer( vol_data );

	//////////////////////////////////////////////////////////////////////////
	// Sphere

	gis.push_back( createSphere( make_float3(-6.f,0.f,6.f), 1.f));
	//setMaterial(gis.back(), diffuse, "diffuse_color", white);
	setMaterial(gis.back(), mirrorMaterial, "glass_color", make_float3(1.f));


	// Obj 1
	const float matrix_1[4*4] = { 0.3,  0,  0,  3, 
		0,  0.3,  0,  -6, 
		0,  0,  0.3, 7, 
		0,  0,  0,  1 };
	const optix::Matrix4x4 m1( matrix_1 );
	std::string obj_path1 = ("optix/mesh/cognacglass.obj");
	GeometryGroup& objgroup1 = createObjloader( obj_path1, m1, glassMaterial);


	// Create geometry group
	GeometryGroup geometry_group = m_context->createGeometryGroup(gis.begin(), gis.end());
	int count_geom = geometry_group->getChildCount();

	geometry_group->setChildCount( count_geom+1 );
	geometry_group->setChild( count_geom, objgroup1->getChild(0) );

	geometry_group->setAcceleration( m_context->createAcceleration("Bvh","Bvh") );
	m_context["top_object"]->set( geometry_group );
}