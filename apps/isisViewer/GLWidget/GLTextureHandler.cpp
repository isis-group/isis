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
	typedef uint8_t TYPE;

	//check if there is an image with this parameters
	if ( !data.isImage( imageID, timestep ) ) {
		LOG( Runtime, error ) << "Trying to copy image with id " << imageID << " and timestep " << timestep
							  << " to an openGL texture. But there is no such image!";
		return 0;
	}

	//check if we have already copied this volume to texture. If we have already copied the image return its texture id
	if ( m_ImageMap[imageID].find( timestep ) != m_ImageMap[imageID].end() ) {
		LOG( Debug, verbose_info ) << "Texture for volume " << imageID << " and timestep " << timestep  << " already exists. Wont copy it.";
		return m_ImageMap[imageID][timestep];
	}

	LOG( Debug, verbose_info ) << "Copy volume with ID " << imageID << " and timestep " << timestep << " to GLTexture.";

	return internCopyImageToTexture<TYPE>( data, GL_UNSIGNED_BYTE, imageID, timestep );


}
}
} // end namespace