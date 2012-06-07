#ifndef ISIS_FILTER_BASIC_COMMON_HPP
#define ISIS_FILTER_BASIC_COMMON_HPP

#include <cmath>

namespace isis
{
namespace filter
{
namespace _internal
{

double sigma2FWHM( const double &sigma )
{
	return 2 * sqrt ( 2 * log( 2 ) ) * sigma;
}

double FWHM2Sigma( const double &fwhm )
{
	return fwhm / ( 2 * sqrt( 2 * log( 2 ) ) );
}


}
}
}

#endif