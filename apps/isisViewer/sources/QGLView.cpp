/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "QGLView.h"

namespace isis {
namespace viewer {
	
QGLView::QGLView( QWidget *parent )
	: QGLWidget( parent )
{
		QObject::connect( this, SIGNAL( mousePressEvent(QMouseEvent*) ), this, SLOT( paint() ) );
}


void QGLView::initializeGL()
{
	glClearColor(1,1,1,1);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void QGLView::paint( )
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glColor3f(1.0,0.0,0.0);
	glRotated(45 / 16.0, 1.0, 0.0, 0.0);
	glRotated(45 / 16.0, 0.0, 1.0, 0.0);
	glRotated(90 / 16.0, 0.0, 0.0, 1.0);
	// A basic square to impress all your friends
	glBegin(GL_LINE_LOOP);
		glVertex3d(0,0,0);
		glVertex3d(1,0,0);
		glVertex3d(1,1,0);
		glVertex3d(0,1,0);
	glEnd();
	updateGL();
}
	
}}