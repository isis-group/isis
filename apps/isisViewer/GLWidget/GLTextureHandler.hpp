#ifndef GLMEMORYMANAGER_HPP
#define GLMEMORYMANAGER_HPP

#define GL_GLEXT_PROTOTYPES

#include "DataContainer.hpp"
#include "ViewerCoreBase.hpp"
#include <DataStorage/typeptr.hpp>
#include <DataStorage/image.hpp>

#include <limits>

#ifdef WIN32
#include <windows.h>								// Header File For Windows
#include <gl\gl.h>									// Header File For The OpenGL32 Library
#include <gl\glu.h>									// Header File For The GLu32 Library
#include <gl\glaux.h>								// Header File For The GLaux Library
#else
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else 
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif
namespace isis
{
namespace viewer
{

/**
 * This class is responsible for copying the image data to a GL_TEXTURE_3D.
 * It gets the data as a boost::weak_ptr and copies it to the texture.
 */

class GLTextureHandler
{
	typedef uint8_t TYPE;
public:

	enum InterpolationType { neares_neighbor, linear };

	GLTextureHandler() { m_Alpha = false; }

	///The image map is a mapping of the imageID and timestep to the texture of the GL_TEXTURE_3D.
	typedef std::map<boost::shared_ptr<ImageHolder>, std::map<size_t, GLuint > > ImageMapType;

	///Convinient function to copy all in DataContainer available volumes to a GL_TEXTURE_3D.
	std::map<boost::shared_ptr<ImageHolder>, GLuint> copyAllImagesToTextures( const DataContainer &data, const bool withAlpha = true, InterpolationType interpolation = neares_neighbor );

	///Copies the given timestep of an image with the given imageID to a GL_TEXTURE_3D. Return the texture id.
	GLuint copyImageToTexture( const boost::shared_ptr<ImageHolder> image, size_t timestep, const bool withAlpha = true, InterpolationType interpolation = neares_neighbor );

	///The image map is a mapping of the imageID and timestep to the texture of the GL_TEXTURE_3D.
	ImageMapType getImageMap() const { return m_ImageMap; }

	bool forceReloadingAllOfType( ImageHolder::ImageType imageType, InterpolationType interpolation = neares_neighbor, bool withAlpha = true ) ;

	void setAlphaEnabled( bool enabled ) { m_Alpha = enabled; }

private:
	ImageMapType m_ImageMap;
	//this is only needed if one specifies the manual scaling
	bool m_Alpha;

	template<typename TYPE>
	GLuint internCopyImageToTexture( GLenum format, const boost::shared_ptr<ImageHolder> image, size_t timestep, bool alpha = true, InterpolationType interpolation = neares_neighbor  ) {
		LOG( Debug, info ) << "Copy image " << image->getID() << " with timestep " << timestep << " to texture";

		util::FixedVector<size_t, 4> size = image->getImageSize();
		size_t volume = size[0] * size[1] * size[2];
		TYPE *dataPtr = static_cast<TYPE *>( image->getImageWeakPointer( timestep ).lock().get() );
		assert( dataPtr != 0 );
		GLint interpolationType = 0;

		switch ( interpolation ) {
		case neares_neighbor:
			interpolationType = GL_NEAREST;
			break;
		case linear:
			interpolationType = GL_LINEAR;
			break;
		}

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		GLuint texture;

		//look if this texture already exists
		if( m_ImageMap[image].find( timestep ) == m_ImageMap[image].end() ) {
			glGenTextures( 1, &texture );
		} else {
			texture = m_ImageMap[image].at( timestep );
		}

		glBindTexture( GL_TEXTURE_3D, texture );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, interpolationType );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, interpolationType );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
		GLint internalFormat;
		GLenum dataFormat;

		if( alpha ) {
			TYPE *dataWithAplpha = ( TYPE * ) calloc( volume * 2, sizeof( TYPE ) );
			size_t index = 0;

			for ( size_t i = 0; i < volume * 2; i += 2 ) {
				dataWithAplpha[i] = dataPtr[index++];
				dataWithAplpha[i + 1] = std::numeric_limits<TYPE>::max();
			}

			glTexImage3D( GL_TEXTURE_3D, 0, GL_LUMINANCE12_ALPHA4,
						  size[0],
						  size[1],
						  size[2], 0, GL_LUMINANCE_ALPHA, format,
						  dataWithAplpha );
		} else {
			glTexImage3D( GL_TEXTURE_3D, 0, GL_LUMINANCE,
						  size[0],
						  size[1],
						  size[2], 0, GL_LUMINANCE, format,
						  dataPtr );
		}

		GLenum glErrorCode = glGetError();

		if( glErrorCode ) {
			LOG( Runtime, error ) << "Error during loading image " << image->getID()
								  << " with timestep " << timestep << " to glTexture3D. Error code is " << glErrorCode;
			return 0;
		} else {
			m_ImageMap[image].insert( std::make_pair<size_t, GLuint >( timestep, texture ) );
			return texture;
		}
	}

};

}
}

#endif