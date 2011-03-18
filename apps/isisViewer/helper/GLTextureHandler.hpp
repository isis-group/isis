#ifndef GLMEMORYMANAGER_HPP
#define GLMEMORYMANAGER_HPP

#define GL_GLEXT_PROTOTYPES

#include "DataContainer.hpp"
#include "ViewerCoreBase.hpp"
#include "GL/gl.h"
#include <DataStorage/typeptr.hpp>
#include <DataStorage/image.hpp>

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
public:
	typedef std::map<size_t, std::map<size_t, GLuint > > ImageMapType;

	///Convinient function to copy all in DataContainer available volumes to a GL_TEXTURE_3D.
	std::map<size_t, GLuint> copyAllImagesToTextures( const DataContainer &data );
	
	///Copies the given timestep of an image with the given imageID to a GL_TEXTURE_3D. Return the texture id.
	GLuint copyImageToTexture( const DataContainer &data , size_t imageID, size_t timestep );
	
	///The image map is a mapping of the imageID and timestep to the texture of the GL_TEXTURE_3D.
	ImageMapType getImageMap() const { return m_ImageMap; }

private:
  
	ImageMapType m_ImageMap;

	template<typename TYPE>
	GLuint internCopyImageToTexture( const DataContainer &data, GLenum format, size_t imageID, size_t timestep, TYPE offset, float scaling ) {
		GLuint texture;
		util::FixedVector<size_t, 4> size = data[imageID].getImageSize();
		TYPE *dataPtr = static_cast<TYPE *>( data.getImageWeakPointer( imageID, timestep ).lock().get() );
		assert( dataPtr != 0 );
		glShadeModel(GL_FLAT);
		glEnable(GL_DEPTH_TEST);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_3D, texture );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
		glTexImage3D( GL_TEXTURE_3D, 0, 3,
					  size[0],
					  size[1],
					  size[2], 0, GL_LUMINANCE, format,
					  dataPtr );
		m_ImageMap[imageID].insert( std::make_pair<size_t, GLuint >( timestep, texture ) );
		return texture;
	}

};

}
}

#endif