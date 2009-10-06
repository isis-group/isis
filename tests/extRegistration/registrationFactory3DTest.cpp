/*
 * registrationFactory3DTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: tuerke
 */

#include "boost/test/included/unit_test.hpp"
#include "extRegistration/isisRegistrationFactory3D.h"

typedef BOOST_TEST_MODULE RegistrationFactory3DTests;

BOOST_AUTO_TEST_CASE(registrationFactory_init_test)
{
	// default constructor
	isis::registration::RegistrationFactory3D regFactoryStd;

	// itk smartpointer constructor
	isis::registration::RegistrationFactory3D::Pointer regFactoryITK = isis::registration::RegistrationFactory3D::New();


}
