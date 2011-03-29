#ifndef GLCROSSHAIR_HPP
#define GLCROSSHAIR_HPP

#include "common.hpp"

namespace isis
{
namespace viewer
{

class GLCrossHair
{
public:
	enum CrosshairType { dotted, dashed, dash_dot_dash };
	enum CenterType { lonelyCenterPoint, noCenterPoint, noCenterSpec };
	void draw( float x, float y );

private:
	CrosshairType m_CrossHairType;
	CenterType m_CenterType;
	util::fvector4 m_Color;
	float m_LineWidth;

};


}
}

#endif