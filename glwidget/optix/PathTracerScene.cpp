//------------------------------------------------------------------------------
//
// PathCamera.cpp: render Area box using path tracing.
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
#include "../widget.h"
using namespace std;



//using namespace optix;
void PathTracerScene::updateParameter( std::string str, float value)
{ 
	m_context[str.c_str()]->setFloat(value); 
}

void PathTracerScene::updateParameter( std::string str, int value)
{ 
	m_context[str.c_str()]->setInt(value); 
}

void PathTracerScene::updateParameter( std::string str, unsigned int value)
{ 
	m_context[str.c_str()]->setUint(value); 
}

float PathTracerScene::getParameter( std::string str)
{ 
	return m_context[str.c_str()]->getFloat(); 
}

void PathTracerScene::SaveImageButton( int id)
{
	saveImage.Save( m_context, id);
}

void PathTracerScene::initScene( InitialCameraData& camera_data )
{
	//m_context->setPrintEnabled(true);
	//m_context->setPrintBufferSize(4096);
	//m_context["MCWoodcock"]->setInt(0);

	m_context->setRayTypeCount( 3 );
	m_context->setEntryPointCount( 2 );
	m_context->setStackSize( 1800 );

	m_context["scene_epsilon"]->setFloat( 1.e-3f );
	m_context["max_depth"]->setUint(m_max_depth);
	m_context["pathtrace_ray_type"]->setUint(0u);
	m_context["pathtrace_shadow_ray_type"]->setUint(1u);
	m_context["pathtrace_bsdf_shadow_ray_type"]->setUint(2u);
	m_context["rr_begin_depth"]->setUint(m_rr_begin_depth);


	// Setup output buffer
	optix::Variable output_buffer = m_context["output_buffer"];
	optix::Buffer buffer = createOutputBuffer( RT_FORMAT_FLOAT4, m_width, m_height );
	output_buffer->set(buffer);


	// Set up camera
	camera_data = InitialCameraData( optix::make_float3( 0.0f, 0.0f, 30.0f ), // eye
		optix::make_float3( 0.f, 0.0f, 0.0f ),    // lookat
		optix::make_float3( 0.0f, 1.0f,  0.0f ),       // up
		45.0f );                                // vfov

	// Declare these so validation will pass
	m_context["eye"]->setFloat( optix::make_float3( 0.0f, 0.0f, 0.0f ) );
	m_context["U"]->setFloat( optix::make_float3( 0.0f, 0.0f, 0.0f ) );
	m_context["V"]->setFloat( optix::make_float3( 0.0f, 0.0f, 0.0f ) );
	m_context["W"]->setFloat( optix::make_float3( 0.0f, 0.0f, 0.0f ) );

	m_context["sqrt_num_samples"]->setUint( m_sqrt_num_samples );
	m_context["bad_color"]->setFloat( 0.0f, 1.0f, 0.0f );
	m_context["bg_color"]->setFloat( optix::make_float3(0.0f) );

	// Setup programs 1
	std::string ptx_path = my_ptxpath("PathCamera.cu" );
	optix::Program ray_gen_program = m_context->createProgramFromPTXFile( ptx_path, "pathtrace_camera" );
	m_context->setRayGenerationProgram( 0, ray_gen_program );
	optix::Program exception_program = m_context->createProgramFromPTXFile( ptx_path, "exception" );
	m_context->setExceptionProgram( 0, exception_program );

	miss_program_noHDR = m_context->createProgramFromPTXFile( ptx_path, "miss" );
	miss_program_hasHDR = m_context->createProgramFromPTXFile( ptx_path, "envmap_miss" );
	switchHasHDR(m_context["hasHDR"]->getFloat()<0.5f? false:true);
	m_context["frame_number"]->setUint(1);

	// Index of sampling_stategy (BSDF, light, MIS)
	m_sampling_strategy = 0;
	m_context["sampling_stategy"]->setInt(m_sampling_strategy);

	// Create scene geometry

	createEnvironmentScene();

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
	if(!m_context["doRendering"]->getInt())
		return;
	if(m_context["_init_"]->getInt()==1)
	{
		int SequenceCurID = m_context["SequenceCurID"]->getInt();
		UpdateID( SequenceCurID);
		float progresValue = float(SequenceCurID-m_context["SequenceMinID"]->getInt())/float(m_context["SequenceMaxID"]->getInt()-m_context["SequenceMinID"]->getInt())*100.f;
		_widget->addProgressBar(int(progresValue));
		m_context["SequenceCurID"]->setInt( SequenceCurID+1);
		m_context["_init_"]->setInt(0);
	}

	updateParameter( "isPreCompution", 0.f);
	updateParameter("isRayMarching", 1.f);

	if(getParameter( "isFLDMethod")<0.5f)
	{
		updateParameter("isRayMarching", 0.f);
	}
	m_context["eye"]->setFloat( camera_data.eye );
	m_context["U"]->setFloat( camera_data.U );
	m_context["V"]->setFloat( camera_data.V );
	m_context["W"]->setFloat( camera_data.W );

	optix::Buffer buffer = m_context["output_buffer"]->getBuffer();
	RTsize buffer_width, buffer_height;
	buffer->getSize( buffer_width, buffer_height );

	if( m_camera_changed ) {
		m_camera_changed = false;
		m_frame = 1;
	}
	printf ("%d\r", m_frame);
	m_context["frame_number"]->setUint( m_frame++ );

	m_context->launch( 0,static_cast<unsigned int>(buffer_width),static_cast<unsigned int>(buffer_height));

	if (m_context["_init_"]->getInt()==0 && m_frame>=m_context["SequenceMaxSample"]->getInt())
	{
		m_context["_init_"]->setInt(1);
		m_camera_changed = true;
		SaveImageButton( volumeData._id);
	}
}

void PathTracerScene::PreCompution()
{
	updateParameter( "numSampling", 5);
	updateParameter( "isSingle", 0.f);
	updateParameter("isRayMarching", 0.f);
	updateParameter( "isPreCompution", 1.f);
	updateParameter( "curIterator", 0);
	optix::Buffer buffer = m_context["gridBuffer"]->getBuffer();
	RTsize buffer_x;
	buffer->getSize( buffer_x);
	int maxCompution = 100;
	for (int i=1; i<maxCompution; ++i)
	{
		updateParameter( "numCompution", unsigned int(i));
		m_context->launch( 1,static_cast<unsigned int>(buffer_x));
	}

	m_context->setRayGenerationProgram( 1, ray_gen_program_multi );
	for (int i=1; i<200; ++i)
	{
		updateParameter( "curIterator", i);
		m_context->launch( 1,static_cast<unsigned int>(buffer_x));
	}
	//updateParameter( "isPreCompution", 0.f);
	//updateParameter("isRayMarching", 1.f);
}

//-----------------------------------------------------------------------------

optix::Buffer PathTracerScene::getOutputBuffer()
{
	return m_context["output_buffer"]->getBuffer();
}

optix::GeometryGroup PathTracerScene::createObjloader( const std::string& path, const optix::Matrix4x4 m0, const optix::Material& material0)
{
	optix::GeometryGroup geomgroup = m_context->createGeometryGroup();
	//std::string prog_path = std::string(sutilSamplesPtxDir()) + "/glass_generated_triangle_mesh_iterative.cu.ptx";
	optix::Program mesh_intersect = m_context->createProgramFromPTXFile( my_ptxpath( "TriangleMesh.cu" ), "mesh_intersect"  );
	ObjLoader objloader0( (path).c_str(), m_context, geomgroup, material0 );
	objloader0.setIntersectProgram( mesh_intersect );
	objloader0.load( m0 );
	return geomgroup;
}

optix::GeometryInstance PathTracerScene::createParallelogram( const optix::float3& anchor,
													  const optix::float3& offset1,
													  const optix::float3& offset2)
{
	optix::Geometry parallelogram = m_context->createGeometry();
	parallelogram->setPrimitiveCount( 1u );
	parallelogram->setIntersectionProgram( m_pgram_intersection );
	parallelogram->setBoundingBoxProgram( m_pgram_bounding_box );

	optix::float3 normal = normalize( cross( offset1, offset2 ) );
	float d = dot( normal, anchor );
	optix::float4 plane = make_float4( normal, d );

	optix::float3 v1 = offset1 / dot( offset1, offset1 );
	optix::float3 v2 = offset2 / dot( offset2, offset2 );

	parallelogram["plane"]->setFloat( plane );
	parallelogram["anchor"]->setFloat( anchor );
	parallelogram["v1"]->setFloat( v1 );
	parallelogram["v2"]->setFloat( v2 );

	optix::GeometryInstance gi = m_context->createGeometryInstance();
	gi->setGeometry(parallelogram);
	return gi;
}

optix::GeometryInstance PathTracerScene::createSphere( const optix::float3& p, float r)
{
	optix::Geometry sphere = m_context->createGeometry();
	sphere->setPrimitiveCount( 1u );
	sphere->setBoundingBoxProgram( m_context->createProgramFromPTXFile( my_ptxpath("sphere.cu" ), "bounds" ) );
	sphere->setIntersectionProgram( m_context->createProgramFromPTXFile( my_ptxpath( "sphere.cu" ), "robust_intersect" ) );
	sphere["sphere"]->setFloat( p.x, p.y, p.z, r );
	optix::GeometryInstance gi = m_context->createGeometryInstance();
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

void PathTracerScene::setMaterial( optix::GeometryInstance& gi,
								  optix::Material material,
								  const std::string& color_name,
								  const optix::float3& color)
{
	gi->addMaterial(material);
	if (color_name!="")
	{
		gi[color_name]->setFloat(color);
	}
}



void PathTracerScene::updateHasAreaBox( )
{
	// Light buffer
	float light_em = m_context["light_em"]->getFloat();
	float hasArea = m_context["hasArea"]->getFloat();
	light.emission = hasArea>0.5f?optix::make_float3(light_em):optix::make_float3(0.f);
	memcpy( light_buffer->map(), &light, sizeof( light ) );
	light_buffer->unmap();
	m_context["lights"]->setBuffer( light_buffer );
}

void PathTracerScene::switchHasHDR( bool hasHDR)
{
	if(hasHDR)
	{
		m_context->setMissProgram( 0, miss_program_hasHDR );
	}
	else
	{
		m_context->setMissProgram( 0, miss_program_noHDR );
	}
}

void PathTracerScene::switchEnvironmentLight( int envId)
{
	const optix::float3 default_color = optix::make_float3(1.0f, 1.0f, 1.0f);
	std::string envmapPath = "optix/hdr/";
	switch( envId)
	{
	case 0:envmapPath += "CedarCity.hdr";break;
	case 1:envmapPath += "grace_ll.hdr";break;
	case 2:envmapPath += "rnl.hdr";break;
	case 3:envmapPath += "stpeters.hdr";break;
	case 4:envmapPath += "octane_studio4.hdr";break;
	case 5:envmapPath += "DH001LL.hdr";break;
	case 6:envmapPath += "DH037LL.hdr";break;
	case 7:envmapPath += "DH053LL.hdr";break;
	case 8:envmapPath += "ennis.hdr";break;
	case 9:envmapPath += "grace_latlong.hdr";break;
	case 10:envmapPath += "windowStudio.hdr";break;
	case 11:envmapPath += "studio019.hdr";break;
	default:envmapPath += "CedarCity.hdr";break;
	}
	m_context["envmap"]->setTextureSampler( loadTexture( m_context, envmapPath, default_color) );
	envMap.setup( m_context);
}

void PathTracerScene::updateGeometryInstance()
{
	std::vector<optix::GeometryInstance> gis;
	// 0 add volume 
	gis.insert(gis.end(), gis0volume.begin(), gis0volume.end());

	// 1 add reference
	if (getParameter("hasBackground")>0.5f)
	{
		gis.insert(gis.end(), gis1reference.begin(), gis1reference.end());
	}	

	// 2 add cornell box
	if( getParameter("hasCornell")>0.5f)
		gis.insert(gis.end(), gis2cornell.begin(), gis2cornell.end());

	// 3 add area box
	if( getParameter("hasArea")>0.5f)
		gis.insert(gis.end(), gis3arealight.begin(), gis3arealight.end());

	optix::GeometryGroup geometry_group = m_context->createGeometryGroup(gis.begin(), gis.end());
	geometry_group->setAcceleration( m_context->createAcceleration("Bvh","Bvh") );
	m_context["top_object"]->set( geometry_group );
}

std::string PathTracerScene::updateVolumeFilename( std::string filename)
{
	optix::int3 indexXYZ;
	std::string path = volumeData.UpdateFilename( filename);
	volumeData.setup(m_context, 0, indexXYZ);
	saveImage.pathHead = path;
	return path;
}

void PathTracerScene::UpdateID( int id)
{
	optix::int3 indexXYZ;
	volumeData.UpdateID( id);
	volumeData.setup(m_context, 0, indexXYZ);
}


void PathTracerScene::createEnvironmentScene()
{
	m_context["_init_"]->setInt( -1);
	// init Geometry Instance

	// set the top_shadower
	std::vector<optix::GeometryInstance> gis;
	optix::GeometryGroup shadow_group = m_context->createGeometryGroup(gis.begin(), gis.end());
	shadow_group->setAcceleration( m_context->createAcceleration("Bvh","Bvh") );
	m_context["top_shadower"]->set( shadow_group );


	// Generate material
	areaMaterial = DefineDiffuseLight( m_context);
	diffuseMaterial = DefineDiffuseMaterial( m_context);
	fogMaterial = DefineFogMaterial( m_context, 0);
	//updateParameter("isRayMarching", 0.f);
	glassMaterial = DefineGlassMaterial( m_context);
	mirrorMaterial = DefineMirrorMaterial( m_context);
	m_pgram_bounding_box = m_context->createProgramFromPTXFile( my_ptxpath( "parallelogram.cu" ), "bounds" );
	m_pgram_intersection = m_context->createProgramFromPTXFile( my_ptxpath( "parallelogram.cu" ), "intersect" );



	//////////////////////////////////////////////////////////////////////////
	// global parameter setting





	const optix::float3 white = optix::make_float3( 0.8f, 0.8f, 0.8f );
	const optix::float3 black = optix::make_float3( 0.2f, 0.2f, 0.2f );
	const optix::float3 green = optix::make_float3( 0.05f, 0.3f, 0.05f );
	const optix::float3 red   = optix::make_float3( 0.8f, 0.05f, 0.05f );
	

	//////////////////////////////////////////////////////////////////////////
	// Light buffer
	light.corner   = optix::make_float3( 2.5f, 10.499f, -2.0f);
	light.v1       = optix::make_float3( -5.0f, 0.0f, 0.0f);
	light.v2       = optix::make_float3( 0.0f, 0.0f, 4.0f);
	light.normal   = normalize( cross(light.v1, light.v2) );
	m_context["light_em"]->setFloat( 30.f);

	light_buffer = m_context->createBuffer( RT_BUFFER_INPUT );
	light_buffer->setFormat( RT_FORMAT_USER );
	light_buffer->setElementSize( sizeof( ParallelogramLight ) );
	light_buffer->setSize( 1u );
	updateHasAreaBox();

	//////////////////////////////////////////////////////////////////////////
	// GeometryInstance 3 - light  
	gis3arealight.push_back( createParallelogram( light.corner,
		light.v1,
		light.v2 ) );
	setMaterial(gis3arealight.back(), areaMaterial, "emission_color", optix::make_float3(1.f));

	
	
	//////////////////////////////////////////////////////////////////////////
	// GeometryInstance 2 - Cornell Box
	optix::float3 p0 = optix::make_float3(-13.5f, -10.5f, -10.5f);
	optix::float3 p1 = optix::make_float3(13.5f, 10.5f, 10.5f);
	optix::float3 dp = p1-p0;

	//floor
	gis2cornell.push_back( createParallelogram( p0, optix::make_float3( 0.0f, 0.0f, dp.z),optix::make_float3( dp.x, 0.0f, 0.0f) ) );
	setMaterial(gis2cornell.back(), diffuseMaterial, "diffuse_color", white);
	//ceiling
	gis2cornell.push_back( createParallelogram( p1, -optix::make_float3( 0.0f, 0.0f, dp.z),-optix::make_float3( dp.x, 0.0f, 0.0f) ) );
	setMaterial(gis2cornell.back(), diffuseMaterial, "diffuse_color", white);
	//left
	gis2cornell.push_back( createParallelogram( p0, optix::make_float3( 0.0f, 0.0f, dp.z),optix::make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis2cornell.back(), diffuseMaterial, "diffuse_color", green);
	//right
	gis2cornell.push_back( createParallelogram( p1, -optix::make_float3( 0.0f, 0.0f, dp.z),-optix::make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis2cornell.back(), diffuseMaterial, "diffuse_color", red);
	//behind
	gis2cornell.push_back( createParallelogram( p0, optix::make_float3( dp.x, 0.0f, 0.f),optix::make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis2cornell.back(), diffuseMaterial, "diffuse_color", white);
	//front
	//gis.push_back( createParallelogram( p1, -make_float3( dp.x, 0.0f, 0.f), -make_float3( 0.f , dp.y, 0.0f) ) );
	//setMaterial(gis.back(), diffuseMaterial, "diffuse_color", white);

	//load volume data
	optix::int3 indexXYZ;
	int volumeSelect = 2;
	switch( volumeSelect)
	{
	case 0:
		//////////////////////////////////////////////////////////////////////////
		// GeometryInstance 0 - Volume Box
		p0 = optix::make_float3(-10.49f, -10.49f, -4.f);
		p1 = optix::make_float3(10.49f, 10.49f, 4.f);
		volumeData.UpdateFilename( std::string("optix/volume/density_render.70.pbrt"));
		volumeData.setup(m_context, 0, indexXYZ);
		break;
	case 1:
		p0 = optix::make_float3(-6.f, -10.49f, -6.f);
		p1 = optix::make_float3(6.f, 9.49f, 6.f);
		volumeData.UpdateFilename( std::string("../../VolumeData/Output_109.dat"));
		volumeData.setup(m_context, 1, indexXYZ);
		break;
	case 2:
		p0 = optix::make_float3(-13.49f, -8.49f, -8.f);
		p1 = optix::make_float3(13.49f, 8.49f, 8.f);
		volumeData.UpdateFilename( std::string("../../VolumeData/cloud/dat/Output_200.dat"));
		volumeData.setup(m_context, 2, indexXYZ);
		break;
	}
	m_context["P0"]->setFloat(p0.x, p0.y, p0.z );
	m_context["P1"]->setFloat(p1.x, p1.y, p1.z );
	dp = p1-p0;
	//floor
	gis0volume.push_back( createParallelogram( p0, optix::make_float3( 0.0f, 0.0f, dp.z),optix::make_float3( dp.x, 0.0f, 0.0f) ) );
	setMaterial(gis0volume.back(), fogMaterial, "diffuse_color", white);
	//ceiling
	gis0volume.push_back( createParallelogram( p1, -optix::make_float3( 0.0f, 0.0f, dp.z),-optix::make_float3( dp.x, 0.0f, 0.0f) ) );
	setMaterial(gis0volume.back(), fogMaterial, "diffuse_color", white);
	//left
	gis0volume.push_back( createParallelogram( p0, optix::make_float3( 0.0f, 0.0f, dp.z),optix::make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis0volume.back(), fogMaterial, "diffuse_color", white);
	//right
	gis0volume.push_back( createParallelogram( p1, -optix::make_float3( 0.0f, 0.0f, dp.z),-optix::make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis0volume.back(), fogMaterial, "diffuse_color", white);
	//behind
	gis0volume.push_back( createParallelogram( p0, optix::make_float3( dp.x, 0.0f, 0.f),optix::make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis0volume.back(), fogMaterial, "diffuse_color", white);
	//front
	gis0volume.push_back( createParallelogram( p1, -optix::make_float3( dp.x, 0.0f, 0.f), -optix::make_float3( 0.f , dp.y, 0.0f) ) );
	setMaterial(gis0volume.back(), fogMaterial, "diffuse_color", white);

		


	//////////////////////////////////////////////////////////////////////////
	// GeometryInstance 0 - Sphere and Cup.obj

	//gis1reference.push_back( createSphere( make_float3(-6.f,0.f,6.f), 1.f));
	////setMaterial(gis.back(), diffuse, "diffuse_color", white);
	//setMaterial(gis1reference.back(), mirrorMaterial, "glass_color", make_float3(1.f));
	optix::Material fogMirrorMaterial = DefineFogMaterial( m_context, 1);
	const float matrix_1[4*4] = { 0.4,  0,  0,  0, 
		0,  0.6,  0,  -10.5, 
		0,  0,  0.4, 0, 
		0,  0,  0,  1 };
	const optix::Matrix4x4 m1( matrix_1 );
	std::string obj_path1 = ("optix/mesh/cylinder.obj");
	optix::GeometryGroup& objgroup1 = createObjloader( obj_path1, m1, fogMirrorMaterial);
	//gis1reference.push_back(objgroup1->getChild(0));

	//glass cup
	//const float matrix_2[4*4] = { 2.,  0,  0,  -6, 
	//	0,  1.9,  0,  -10.5, 
	//	0,  0,  2., -7, 
	//	0,  0,  0,  1 };
	//const optix::Matrix4x4 m2( matrix_2 );
	//std::string obj_path2 = ("optix/mesh/cup.obj");
	//GeometryGroup& objgroup2 = createObjloader( obj_path2, m2, glassMaterial);
	//gis1reference.push_back(objgroup2->getChild(0));

	// Create geometry group
	updateGeometryInstance();

	//////////////////////////////////////////////////////////////////////////
	// Setup programs 2
	std::string ptx_path2 = my_ptxpath("PreCompution.cu" );
	optix::Program ray_gen_program_single = m_context->createProgramFromPTXFile( ptx_path2, "PreCompution" );
	m_context->setRayGenerationProgram( 1, ray_gen_program_single );
	optix::Program exception_program2 = m_context->createProgramFromPTXFile( ptx_path2, "exception" );
	m_context->setExceptionProgram( 1, exception_program2 );
	//m_context->setMissProgram( 1, m_context->createProgramFromPTXFile( ptx_path2, "envmap_miss" ) );

	//////////////////////////////////////////////////////////////////////////
	// Set the parameter of the FLD 
	m_context["ee"]->setFloat( 10e-20);
	m_context["dx"]->setFloat( 0.2f);
	m_context["weight"]->setFloat( 1.3f);
	m_context["J_mean"]->setFloat( 1.f);
	//updateParameter( "numSampling", 5);
	//updateParameter( "isSingle", 0.f);
	//updateParameter("isRayMarching", 0.f);
	//updateParameter( "isPreCompution", 1.f);
	//updateParameter( "curIterator", 0);

	//FLD key step: it loaded takes much time
	//ray_gen_program_multi = m_context->createProgramFromPTXFile( ptx_path2, "MultiCompution" );

	//////////////////////////////////////////////////////////////////////////
	// Setup programs 3
	//std::string ptx_path3 = my_ptxpath("MultiCompution.cu" );
	//Program ray_gen_program3 = m_context->createProgramFromPTXFile( ptx_path3, "MultiCompution" );
	//m_context->setRayGenerationProgram( 2, ray_gen_program3 );
	//Program exception_program3 = m_context->createProgramFromPTXFile( ptx_path3, "exception" );
	//m_context->setExceptionProgram( 2, exception_program3 );
	//////////////////////////////////////////////////////////////////////////
	// Setup output buffer 2, 3
	//m_context["gridBuffer"]->set(createOutputBuffer( RT_FORMAT_FLOAT4, 100, 100, 40));


	int indexN = indexXYZ.x*indexXYZ.y*indexXYZ.z;
	optix::Buffer gridData = m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT);
	gridData->setFormat(RT_FORMAT_FLOAT3);
	gridData->setSize(indexN);
	m_context["gridBuffer"]->setBuffer( gridData );

	optix::Buffer gridFluence = m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT);
	gridFluence->setFormat(RT_FORMAT_FLOAT3);
	gridFluence->setSize(indexN);
	m_context["gridFluence"]->setBuffer( gridFluence );


}