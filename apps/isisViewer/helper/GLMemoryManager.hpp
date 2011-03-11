#ifndef GLMEMORYMANAGER_HPP
#define GLMEMORYMANAGER_HPP

#define GL_GLEXT_PROTOTYPES

#include "DataContainer.hpp"
#include "ViewerCoreBase.hpp"
#include "GL/gl.h"

namespace isis
{
namespace viewer
{

class GLMemoryManager
{
public:
	typedef std::map<size_t, std::map<size_t, GLuint > > ImageMapType;

	GLMemoryManager();

	void copyAllImagesToTextures( const DataContainer &data );
	GLuint copyImageToTexture( const DataContainer &data , size_t imageID, size_t timestep );
	ImageMapType getImageMap() const { return m_ImageMap; }

private:
	ImageMapType m_ImageMap;

	template<typename TYPE>
	GLuint internCopyImageToTexture( const DataContainer &data, GLenum format, size_t imageID, size_t timestep ) {
		GLuint texture;
		util::FixedVector<size_t, 4> size = data[imageID].getImageSize();
		TYPE *dataPtr = static_cast<TYPE *>( data.getImageWeakPointer( imageID, timestep ).lock().get() );
		assert( dataPtr != 0 );
		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_3D, texture );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP );
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