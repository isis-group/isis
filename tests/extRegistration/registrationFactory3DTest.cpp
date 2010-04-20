/*
 * registrationFactory3DTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: tuerke
 */

#define BOOST_TEST_MODULE RegistrationFactory3DTests;
#include "boost/test/included/unit_test.hpp"
#include "extRegistration/isisRegistrationFactory3D.h"

namespace isis
{
namespace test
{

typedef unsigned short PixelType;
const unsigned int ImageDimension = 3;
typedef itk::Image<PixelType, ImageDimension> ImageType;

BOOST_AUTO_TEST_CASE( registrationFactory_init_test )
{
	// itk smartpointer constructor
	isis::registration::RegistrationFactory3D<ImageType, ImageType>::Pointer regFactoryITK =
		isis::registration::RegistrationFactory3D<ImageType, ImageType>::New();
}

}
} // end namespace isis.test
