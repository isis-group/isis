#include "Color.hpp"



namespace isis {
namespace viewer {
	
std::vector< util::fvector4 > Color::getColorGradientRGB(  )
{
	
	std::vector< util::fvector4 > retRGBGradient;
	QColor rgbColor;
	for (float angle = 0; angle<360; angle++)
	{
		rgbColor = QColor::fromHsv( angle, 255, 255 );
		retRGBGradient.push_back( util::fvector4( rgbColor.red(), rgbColor.green(), rgbColor.blue() ) );
	}
	return retRGBGradient;
}
	
	
}} // end namespace