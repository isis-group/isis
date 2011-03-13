#include "GLTextureHandler.hpp"

namespace isis
{
namespace viewer
{


std::map<size_t, GLuint> GLTextureHandler::copyAllImagesToTextures( const DataContainer &data )
{
	// here we only copy the first timestep of each image. Would take a while to do this for all timesteps
	std::map<size_t, GLuint> retIDList;
	for ( size_t imageID = 0; imageID < data.size(); imageID++ ) {
		retIDList[imageID] = copyImageToTexture( data, imageID, 0 );
	}
	return retIDList;
	
}

GLuint GLTextureHandler::copyImageToTexture( const DataContainer &data, size_t imageID, size_t timestep )
{
	//check if we have already copied this volume to texture
	if ( m_ImageMap[imageID].find( timestep ) != m_ImageMap[imageID].end() ) {
		LOG( Debug, verbose_info ) << "Texture for volume " << imageID << " and timestep " << timestep  << " already exists. Wont copy it.";
		return 0;
	}

	LOG( Debug, verbose_info ) << "Copy volume with ID " << imageID << " and timestep " << timestep << " to GLTexture.";
	unsigned short typeID = data[imageID].getMajorTypeID();

	switch( typeID ) {
	case data::ValuePtr<int8_t>::staticID:
		return internCopyImageToTexture<GLbyte>( data, GL_BYTE, imageID, timestep );
		break;
	case data::ValuePtr<uint8_t>::staticID:
		return internCopyImageToTexture<GLubyte>( data, GL_UNSIGNED_BYTE, imageID, timestep );
		break;
	case data::ValuePtr<int16_t>::staticID:
		return internCopyImageToTexture<GLshort>( data, GL_SHORT, imageID, timestep );
		break;
	case data::ValuePtr<uint16_t>::staticID:
		return internCopyImageToTexture<GLushort>( data, GL_UNSIGNED_SHORT, imageID, timestep );
		break;
	case data::ValuePtr<int32_t>::staticID:
		return internCopyImageToTexture<GLint>( data, GL_INT, imageID, timestep );
		break;
	case data::ValuePtr<uint32_t>::staticID:
		return internCopyImageToTexture<GLuint>( data, GL_UNSIGNED_INT, imageID, timestep );
		break;
	case data::ValuePtr<float>::staticID:
		return internCopyImageToTexture<GLfloat>( data, GL_FLOAT, imageID, timestep );
		break;
	case data::ValuePtr<double>::staticID:
		return internCopyImageToTexture<GLdouble>( data, GL_DOUBLE, imageID, timestep );
		break;
	default:
		LOG( Runtime, error ) << "I do not know any type with ID " << typeID << "!";
		return 0;

	}
}
}
} // end namespace