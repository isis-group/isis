#include "GLTextureHandler.hpp"

namespace isis
{
namespace viewer
{


std::map<boost::shared_ptr<ImageHolder>, GLuint> GLTextureHandler::copyAllImagesToTextures( const DataContainer &data, const bool withAlpha, GLTextureHandler::InterpolationType interpolation )
{
	// here we only copy the first timestep of each image. Would take a while to do this for all timesteps
	std::map<boost::shared_ptr<ImageHolder>, GLuint> retIDList;

	BOOST_FOREACH( DataContainer::const_reference image, data ) {
		retIDList[image.second] = copyImageToTexture( image.second, 0, withAlpha, interpolation );
	}

	return retIDList;

}

GLuint GLTextureHandler::copyImageToTexture(  const boost::shared_ptr<ImageHolder> image, size_t timestep, const bool withAlpha, GLTextureHandler::InterpolationType interpolation )
{


	//check if there is an image with this parameters
	if ( image->getImageSize()[3] <= timestep )  {
		LOG( Runtime, error ) << "Trying to copy image " << image->getID() << " with timestep " << timestep << " to an openGL texture. But there is no such timestep in this image!";
		return 0;
	}

	//check if we have already copied this volume to texture. If we have already copied the image return its texture id
	if ( m_ImageMap[image].find( timestep ) != m_ImageMap[image].end() ) {
		LOG( Debug, verbose_info ) << "Texture for volume " << image->getID() << " and timestep " << timestep  << " already exists. Wont copy it.";
		return m_ImageMap[image][timestep];
	}

	return internCopyImageToTexture<GLTextureHandler::TYPE>( GL_UNSIGNED_BYTE, image, timestep, withAlpha, interpolation );


}
bool GLTextureHandler::forceReloadingAllOfType( ImageHolder::ImageType imageType, InterpolationType interpolation, bool withAlpha )
{
	typedef std::map< size_t, GLuint> TimeStepMap;
	BOOST_FOREACH( ImageMapType::const_reference image, m_ImageMap ) {
		if( image.first->getImageState().imageType == imageType ) {
			BOOST_FOREACH( TimeStepMap::const_reference timestep, image.second ) {
				internCopyImageToTexture<GLTextureHandler::TYPE>( GL_UNSIGNED_BYTE, image.first, timestep.first, withAlpha, interpolation );
			}
		}
	}
}


}
} // end namespace