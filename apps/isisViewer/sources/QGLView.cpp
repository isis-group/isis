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
	: QGLWidget( parent ),
	texname()
{
		QObject::connect( this, SIGNAL( mousePressEvent(QMouseEvent*) ), this, SLOT( paint() ) );
}


void QGLView::initializeGL()
{

	isis::data::Chunk ch = *m_Image.getChunkList().front();
	u_int32_t checkImage[ch.sizeToVector()[0]][ch.sizeToVector()[1]][ch.sizeToVector()[2]];
	int i,j,c;
	u_int32_t *ptr = ch.voxel<u_int32_t>(0);
	for (i=0;i<ch.sizeToVector()[0];i++) 
	{
		for (j=0;j<ch.sizeToVector()[1];j++)
		{
			
			for(unsigned short rgba=0;rgba<4;rgba++) 
			{
				std::cout << *ptr << std::endl;
				checkImage[i][j][rgba] = *ptr;
			}
			ptr++;
		}
	}
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_2D, texname);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
					GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
					GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ch.sizeToVector()[0], 
					ch.sizeToVector()[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, 
					checkImage);
}

void QGLView::paint( )
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_2D, texname);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 1.0, 0.0);
	glTexCoord2f(10.0, 1.0); glVertex3f(0.0, 1.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, -1.0, 0.0);

	glTexCoord2f(0.0, 0.0); glVertex3f(1.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(1.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(2.41421, 1.0, -1.41421);
	glTexCoord2f(1.0, 0.0); glVertex3f(2.41421, -1.0, -1.41421);
	glEnd();
	glFlush();
	glDisable(GL_TEXTURE_2D);
	
	updateGL();
}
	
}}