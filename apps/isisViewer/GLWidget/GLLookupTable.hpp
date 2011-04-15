#ifndef GLLOOKUPTABLE_HPP
#define GLLOOKUPTABLE_HPP

#include "Color.hpp"
#include <GL/gl.h>

namespace isis {
namespace viewer {
class GLLookUpTable
{
public:
	
	
	GLuint getLookUpTableAsTexture( const Color::LookUpTableType &lutType ) const;
	
	
};	

}}

#endif