#include "QGLWidgetImplementation.hpp"
#include <QVBoxLayout>
#include <QMouseEvent>

namespace isis {
namespace viewer {

QGLWidgetImplementation::QGLWidgetImplementation( ViewerCore* core, QWidget* parent, PlaneType plane )
	: QGLWidget( parent ),
	m_ViewerCore( boost::shared_ptr<ViewerCore>(core) ),
	m_PlaneType(plane)
{
	setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));
	(new QVBoxLayout(parent))->addWidget(this);
	setMouseTracking(true);
	connectSignals();
}
	
void QGLWidgetImplementation::connectSignals()
{
	connect(this, SIGNAL(redraw()), SLOT(updateGL()));
	
}
void QGLWidgetImplementation::initializeGL()
{
	copyAllImageToTexture();
	
}

void QGLWidgetImplementation::copyAllImageToTexture()
{
	for ( size_t imageID = 0; imageID < m_ViewerCore->getDataContainer().size(); imageID++ )
	{
		copyImageToTexture(imageID, 0);
	}
	
}
void QGLWidgetImplementation::mouseMoveEvent(QMouseEvent* e)
{
	//TODO debug
	float pos = float(e->x()) / 300;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_3D, m_CurrentTextureID);
	glBegin(GL_QUADS);
	glTexCoord3f(pos, 0.0,0.0); glVertex3f(-1.0, -1.0, 0);
	glTexCoord3f(pos, 0.0,1.0); glVertex3f(-1.0, 1.0, 0);
	glTexCoord3f(pos, 1.0,1.0); glVertex3f(1.0, 1.0, 0);
	glTexCoord3f(pos, 1.0,0.0); glVertex3f(1.0, -1.0, 0);
	glEnd();
	glFlush();
	glDisable(GL_TEXTURE_3D);
	redraw();

}


GLuint QGLWidgetImplementation::copyImageToTexture( size_t imageID, size_t timestep )
{
	LOG(Debug, verbose_info) << "Copy volume with ID " << imageID << " and timestep " << timestep << " to GLTexture.";
	unsigned short typeID = m_ViewerCore->getDataContainer()[imageID].getMajorTypeID();
	switch( typeID )
	{
		case data::ValuePtr<int8_t>::staticID:
			return internCopyImageToTexture<GLbyte>( GL_BYTE, imageID, timestep );
			break;
		case data::ValuePtr<uint8_t>::staticID:
			return internCopyImageToTexture<GLubyte>( GL_UNSIGNED_BYTE, imageID, timestep );
			break;
		case data::ValuePtr<int16_t>::staticID:
			return internCopyImageToTexture<GLshort>( GL_SHORT, imageID, timestep );
			break;
		case data::ValuePtr<uint16_t>::staticID:
			return internCopyImageToTexture<GLushort>( GL_UNSIGNED_SHORT, imageID, timestep );
			break;
		case data::ValuePtr<int32_t>::staticID:
			return internCopyImageToTexture<GLint>( GL_INT, imageID, timestep );
			break;
		case data::ValuePtr<uint32_t>::staticID:
			return internCopyImageToTexture<GLuint>( GL_UNSIGNED_INT, imageID, timestep );
			break;
		case data::ValuePtr<float>::staticID:
			return internCopyImageToTexture<GLfloat>( GL_FLOAT, imageID, timestep );
			break;
		case data::ValuePtr<double>::staticID:
			return internCopyImageToTexture<GLdouble>( GL_DOUBLE, imageID, timestep );
			break;
		default:
			LOG(Runtime, error) << "I do not know any type with ID " << typeID << "!";
			return 0;		

	}
}



}} // end namespace