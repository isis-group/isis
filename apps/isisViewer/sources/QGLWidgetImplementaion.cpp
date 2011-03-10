#include "QGLWidgetImplementation.hpp"
#include <QVBoxLayout>
#include <QMouseEvent>

namespace isis {
namespace viewer {

QGLWidgetImplementation::QGLWidgetImplementation( ViewerCore* core, QWidget* parent )
	: QGLWidget( parent ),
	m_ViewerCore( boost::shared_ptr<ViewerCore>(core) )
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
	GLubyte* dataPtr = static_cast<GLubyte*>( m_ViewerCore->getImageWeakPointer().lock().get() );
	
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_3D, m_TextureID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexImage3D(GL_TEXTURE_3D, 0, 3, 
				m_ViewerCore->getDataContainer()[0].getImageSize()[0], 
				m_ViewerCore->getDataContainer()[0].getImageSize()[1], 
				m_ViewerCore->getDataContainer()[0].getImageSize()[2], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
				dataPtr);
}

void QGLWidgetImplementation::paint()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_3D, m_TextureID);
	glBegin(GL_QUADS);
	glTexCoord3f(0, 0.0,0.0); glVertex3f(-1.0, -1.0, 0.0);
	glTexCoord3f(0, 0.0,1.0); glVertex3f(-1.0, 1.0, 0.0);
	glTexCoord3f(0, 1.0,1.0); glVertex3f(1.0, 1.0, 0.0);
	glTexCoord3f(0, 1.0,0.0); glVertex3f(1.0, -1.0, 0.0);
	
	glEnd();
	glFlush();
	glDisable(GL_TEXTURE_3D);
	redraw();
}

void QGLWidgetImplementation::mouseMoveEvent(QMouseEvent* e)
{
	//TODO debug
	float pos = float(e->globalX()) / 300;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_3D, m_TextureID);
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

}} // end namespace