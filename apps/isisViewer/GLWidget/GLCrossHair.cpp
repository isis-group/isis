#include "GLCrossHair.hpp"
#include "QGLWidgetImplementation.hpp"


namespace isis
{
namespace viewer
{

void GLCrossHair::draw( float _x, float _y )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); //TODO remove that...only for debug
	glMatrixMode( GL_PROJECTION );
	glTranslated( _x, -_y, 0 );
	glColor4f( 0, 0, 0, 0 );
	glColor4f( 1, 0, 0, 0 );
	//  glEnable(GL_LINE_STIPPLE);
	//  glLineStipple(1,0x1C47);
	glLineWidth( 1.0 );
	glBegin( GL_LINES );
	glVertex3d( 0, -2, -1 );
	glVertex3f( 0, -0.1, -1 );
	glVertex3f( 0, 0.1, -1 );
	glVertex3d( 0, 2, -1 );

	glVertex3d( -2, 0, -1 );
	glVertex3f( -0.1, 0, -1 );
	glVertex3f( 0.1, 0, -1 );
	glVertex3d( 2, 0, -1 );
	glEnd();
	glPointSize( 2.0 );
	glBegin( GL_POINTS );
	glVertex3d( 0, 0, -1 );
	glEnd();
	//  glDisable(GL_LINE_STIPPLE);
	glFlush();
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
}


}
}
