#include "Color.hpp"



namespace isis
{
namespace viewer
{

std::vector< util::fvector4 > Color::getColorGradientRGB( const Color::LookUpTableType &lutType, const size_t &numberOfEntries )
{
	
	std::vector< util::fvector4 > retRGBGradient;
	QColor rgbColor;

	switch( lutType ) {
	case Color::hsvLUT:
	{
		float angleStep = 360.0 / numberOfEntries;
		for ( float angle = 0; angle < 360.0; angle += angleStep ) {
			rgbColor = QColor::fromHsv( util::Value<float>( angle ).as<int>(), 255, 255 );
			retRGBGradient.push_back( util::fvector4( rgbColor.red(), rgbColor.green(), rgbColor.blue() ) );
		}

		return retRGBGradient;
		break;
	}
	case Color::hsvLUT_reverse:
	{
		float angleStep = 360.0 / numberOfEntries;
		for ( float angle = 359.0; angle >= 0; angle -= angleStep ) {
			rgbColor = QColor::fromHsv( util::Value<float>( angle ).as<int>(), 255, 255 );
			retRGBGradient.push_back( util::fvector4( rgbColor.red(), rgbColor.green(), rgbColor.blue() ) );
		}

		return retRGBGradient;
		break;
	}
	//white to blue to red to white
	case Color::wbryw:
	{
		float stepLength = 512.0 / numberOfEntries;
		for ( float step = 255; step >= 0; step -= stepLength ) {
			int rg = 255 - util::Value<float>( step ).as<int>();
			retRGBGradient.push_back( util::fvector4( rg, rg, 255 ) );
		}
		for ( float step = 0; step <= 255; step += stepLength ) {
			int gb = util::Value<float>( step ).as<int>();
			if( step < 127 ) {
				retRGBGradient.push_back( util::fvector4( 255, gb * 2, 0 ) );
			} else {
				retRGBGradient.push_back( util::fvector4( 255, 255, (gb-127)*2 ) );
			}
		}
		return retRGBGradient;
		break;
	}
	}
}


}
} // end namespace