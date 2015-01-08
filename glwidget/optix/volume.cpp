#include "volume.h"

std::string VolumeData::UpdateFilename( std::string & filename)
{
	_filename = filename;
	int tmpIndex = _filename.find_last_of('_');
	_filenameHead = _filename.substr( 0, tmpIndex+1);
	tmpIndex = _filename.find_last_of('.');
	_filenameTail = _filename.substr( tmpIndex, _filename.length()-tmpIndex);
	tmpIndex = _filename.find_last_of('/');
	_filenamePath = _filename.substr( 0, tmpIndex+1);
	return _filenamePath;
}

void VolumeData::UpdateID( int id)
{
	_id = id;
	char tmpID [10];
	itoa(id,tmpID,10);
	_filename = _filenameHead+ std::string(tmpID)+_filenameTail;
}

void VolumeData::setup(optix::Context& optixCtx, int kindVolume, optix::int3& indexXYZ)
{
	switch(kindVolume)
	{
	case 0:
		ReadKind0Pbrt( optixCtx);break;
	default:
	case 1:
		ReadKind1Dat(optixCtx);break;
	case 2:
		ReadKind1Dat2(optixCtx);break;
	}
	optixCtx["index_x" ]->setInt(_indexXYZ.x );
	optixCtx["index_y" ]->setInt(_indexXYZ.y );
	optixCtx["index_z" ]->setInt(_indexXYZ.z );  
	indexXYZ = _indexXYZ;
}


void VolumeData::ReadKind0Pbrt(optix::Context& optixCtx)
{
	_indexXYZ.x = 100;
	_indexXYZ.y = 100;
	_indexXYZ.z = 40;	
	//load volume data
	int index_N = _indexXYZ.x*_indexXYZ.y*_indexXYZ.z;
	optix::Buffer vol_data = optixCtx->createBuffer(RT_BUFFER_INPUT);
	vol_data->setFormat(RT_FORMAT_FLOAT);
	vol_data->setSize(index_N);
	float* temp_data = (float*)vol_data->map();
	//read .vol file
	//char* filename = "optix/volume/density_render.70.pbrt";
	FILE* fin;
	fopen_s(&fin, const_cast<const char *>(_filename.c_str()), "r");
	if (!fin)
	{
		std::cout << "Could not load Volume file \n";
		exit(1);
	}
	//float* m_data = new float[index_N];
	for(int i=0; i<index_N; i++)
		fscanf_s(fin,"%f",&temp_data[i]);
	vol_data->unmap();
	optixCtx["volume_density"]->setBuffer( vol_data );
}


void VolumeData::ReadKind1Dat(optix::Context& optixCtx)
{
	FILE* fin;
	fin = fopen( const_cast<const char *>(_filename.c_str()), "r");
	if (!fin)	{std::cout<<"Could not open it!"<<std::endl; }
	
	HANDLE m_file = CreateFileA(const_cast<const char *>(_filename.c_str()), GENERIC_READ, 
				FILE_SHARE_READ, NULL, OPEN_EXISTING, 
				FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_file == INVALID_HANDLE_VALUE)
		std::cout<<"Could not open it"<<std::endl;
	HANDLE m_fileMapping = CreateFileMappingA(m_file, NULL, PAGE_READONLY, 0, 0, NULL);
	if (m_fileMapping == NULL)
		std::cout<<"CreateFileMapping(): failed."<<std::endl;
	int* m_data_int = (int *)MapViewOfFile(m_fileMapping, FILE_MAP_READ, 0, 0, 0);
	float* m_data_float = (float *)MapViewOfFile(m_fileMapping, FILE_MAP_READ, 0, 0, 0);
	_indexXYZ.x = m_data_int[0];
	_indexXYZ.y = m_data_int[1];
	_indexXYZ.z = m_data_int[2];
	//load volume data
	int index_N = _indexXYZ.x*_indexXYZ.y*_indexXYZ.z;
	optix::Buffer vol_data = optixCtx->createBuffer(RT_BUFFER_INPUT);
	vol_data->setFormat(RT_FORMAT_FLOAT);
	vol_data->setSize(index_N);
	float* temp_data = (float*)vol_data->map();
	for(int i=0; i<index_N; i++)
		temp_data[i] = 0.f;
	int size = m_data_int[3];
	m_data_int+=4;
	m_data_float+=4;
	//set the data array
	for (int i = 0; i<size; ++i)
	{
		int index = m_data_int[i*2];
		float density = m_data_float[i*2+1];
		temp_data[index] = density;
	}

	UnmapViewOfFile(m_data_int);
	UnmapViewOfFile(m_data_float);
	CloseHandle(m_fileMapping);
	CloseHandle(m_file);
	vol_data->unmap();
	optixCtx["volume_density"]->setBuffer( vol_data );

}

void VolumeData::ReadKind1Dat2(optix::Context& optixCtx)//sort: yxz
{
	FILE* fin;
	fin = fopen( const_cast<const char *>(_filename.c_str()), "r");
	if (!fin)	{std::cout<<"Could not open it!"<<std::endl; }

	HANDLE m_file = CreateFileA(const_cast<const char *>(_filename.c_str()), GENERIC_READ, 
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_file == INVALID_HANDLE_VALUE)
		std::cout<<"Could not open it"<<std::endl;
	HANDLE m_fileMapping = CreateFileMappingA(m_file, NULL, PAGE_READONLY, 0, 0, NULL);
	if (m_fileMapping == NULL)
		std::cout<<"CreateFileMapping(): failed."<<std::endl;
	int* m_data_int = (int *)MapViewOfFile(m_fileMapping, FILE_MAP_READ, 0, 0, 0);
	float* m_data_float = (float *)MapViewOfFile(m_fileMapping, FILE_MAP_READ, 0, 0, 0);
	_indexXYZ.x = m_data_int[1];
	_indexXYZ.y = m_data_int[0];
	_indexXYZ.z = m_data_int[2];
	//load volume data
	int index_N = _indexXYZ.x*_indexXYZ.y*_indexXYZ.z;
	optix::Buffer vol_data = optixCtx->createBuffer(RT_BUFFER_INPUT);
	vol_data->setFormat(RT_FORMAT_FLOAT);
	vol_data->setSize(index_N);
	float* temp_data = (float*)vol_data->map();
	for(int i=0; i<index_N; i++)
		temp_data[i] = 0.f;
	int size = m_data_int[3];
	m_data_int+=4;
	m_data_float+=4;
	//set the data array
	for (int i = 0; i<size; ++i)
	{
		int index = m_data_int[i*2];
		float density = m_data_float[i*2+1];
		temp_data[yxz2xyz(index)] = density;
	}

	UnmapViewOfFile(m_data_int);
	UnmapViewOfFile(m_data_float);
	CloseHandle(m_fileMapping);
	CloseHandle(m_file);
	vol_data->unmap();
	optixCtx["volume_density"]->setBuffer( vol_data );

}

int VolumeData::yxz2xyz(int i)
{
	optix::int3 yxz;
	yxz.y = i%(_indexXYZ.y);
	yxz.x = i/(_indexXYZ.y)%(_indexXYZ.x);
	yxz.z = i/(_indexXYZ.y)/(_indexXYZ.x);

	return yxz.z*(_indexXYZ.x)*(_indexXYZ.y)+ yxz.y*_indexXYZ.x+yxz.x;
}