#include "GLCrossHair.hpp"
#include "QGLWidgetImplementation.hpp"


namespace isis {
namespace viewer {
	
void GLCrossHair::draw(size_t x, size_t y)
{
	glMatrixMode(GL_PROJECTION);
	glTranslated(x,-y,0);
	glColor4f(1,0,0,0);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1,0x1C47);
	glBegin(GL_LINES);
	glVertex3d(0,-2,-1); glVertex3d(0,2,-1);
	glVertex3d(-2,0,-1); glVertex3d(2,0,-1);
	glEnd();
	glLineWidth(3.0);
	glDisable(GL_LINE_STIPPLE);
	glFlush();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

	
}}
