#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#define GL_GLEXT_PROTOTYPES

#include <QtOpenGL/QGLWidget>
#include "ViewerCore.hpp"
#include <iostream>
#include <DataStorage/chunk.hpp>

namespace isis {
namespace viewer {
	

class QGLWidgetImplementation : public QGLWidget
{
	Q_OBJECT
public:
	enum PlaneType { axial, sagittal, coronal };
	
	QGLWidgetImplementation( ViewerCore* core, QWidget* parent = 0, PlaneType plane = axial ); 
	
	void paint();
	void initializeGL();
	void setPlaneType( PlaneType plane ) { m_PlaneType = plane;}
	
	
private:
	boost::shared_ptr<ViewerCore> m_ViewerCore;
	GLuint m_CurrentTextureID;

public Q_SLOTS:
	
	void recreateTextureVector();
	GLuint copyImageToTexture( unsigned short imageID, size_t timestep );
	
	protected:
	virtual void mouseMoveEvent(QMouseEvent* e );
	
	

	
protected:
Q_SIGNALS:
	void redraw();
	
private:
	void connectSignals();
	
	template<typename TYPE>
	GLuint internCopyImageToTexture( GLenum format, unsigned int imageID, size_t timestep )
	{
		GLuint texture;
		util::FixedVector<size_t, 4> size = m_ViewerCore->getDataContainer()[imageID].getImageSize();
		TYPE* dataPtr = static_cast<TYPE*>( m_ViewerCore->getImageWeakPointer(imageID, timestep).lock().get() );
		assert( dataPtr != 0 );
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_3D, texture);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexImage3D(GL_TEXTURE_3D, 0, 3, 
				size[0], 
				size[1], 
				size[2], 0, GL_LUMINANCE, format, 
				dataPtr);
		return texture;
	}
	
	std::vector<GLuint> m_TextureIDVec;
	PlaneType m_PlaneType;
		
};



}}//end namespace

#endif