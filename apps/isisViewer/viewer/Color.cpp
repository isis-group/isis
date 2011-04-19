#include "Color.hpp"



namespace isis
{
namespace viewer
{

std::vector< util::fvector4 > Color::getColorGradientRGB( const Color::LookUpTableType &lutType, const size_t &numberOfEntries )
{
	float angleStep = 360.0 / numberOfEntries;
	std::vector< util::fvector4 > retRGBGradient;
	QColor rgbColor;

	switch( lutType ) {
	case Color::hsvLUT:

		for ( float angle = 0; angle < 360; angle += angleStep ) {
			rgbColor = QColor::fromHsv( angle, 255, 255 );
			retRGBGradient.push_back( util::fvector4( rgbColor.red(), rgbColor.green(), rgbColor.blue() ) );
		}

		return retRGBGradient;
		break;
	case Color::hsvLUT_reverse:

		for ( float angle = 359; angle >= 0; angle -= angleStep ) {
			rgbColor = QColor::fromHsv( angle, 255, 255 );
			retRGBGradient.push_back( util::fvector4( rgbColor.red(), rgbColor.green(), rgbColor.blue() ) );
		}

		return retRGBGradient;
		break;
	}

}


}
} // end namespace