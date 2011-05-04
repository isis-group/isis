#ifndef GLLOOKUPTABLE_HPP
#define GLLOOKUPTABLE_HPP

#include "Color.hpp"

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
class GLLookUpTable
{
public:


	GLuint getLookUpTableAsTexture( const Color::LookUpTableType &lutType ) const;


};

}
}

#endif