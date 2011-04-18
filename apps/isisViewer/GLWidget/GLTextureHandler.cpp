#include "GLTextureHandler.hpp"

namespace isis
{
namespace viewer
{


std::map<ImageHolder, GLuint> GLTextureHandler::copyAllImagesToTextures( const DataContainer &data, GLTextureHandler::InterpolationType interpolation )
{
	// here we only copy the first timestep of each image. Would take a while to do this for all timesteps
	std::map<ImageHolder, GLuint> retIDList;

	BOOST_FOREACH( DataContainer::const_reference image, data ) {
		retIDList[image] = copyImageToTexture( data, image, 0, interpolation );
	}

	return retIDList;

}

GLuint GLTextureHandler::copyImageToTexture( const DataContainer &data, const ImageHolder &image, size_t timestep, const bool withAlpha, GLTextureHandler::InterpolationType interpolation )
{
	typedef uint8_t TYPE;

	//check if there is an image with this parameters
	if ( image.getImageSize()[3] <= timestep )  {
		LOG( Runtime, error ) << "Trying to copy image " << image.getID() << " with timestep " << timestep << " to an openGL texture. But there is no such timestep in this image!";
		return 0;
	}

	//check if we have already copied this volume to texture. If we have already copied the image return its texture id
	if ( m_ImageMap[image].find( timestep ) != m_ImageMap[image].end() ) {
		LOG( Debug, verbose_info ) << "Texture for volume " << image.getID() << " and timestep " << timestep  << " already exists. Wont copy it.";
		return m_ImageMap[image][timestep];
	}

	return internCopyImageToTexture<TYPE>( data, GL_UNSIGNED_BYTE, image, timestep, withAlpha, interpolation );


}
}
} // end namespace