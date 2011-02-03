/*
 * ImageFormatNii.cpp
 *
 *  Created on: Dec 07, 2009
 *      Author: hellrung
 */

#include <boost/foreach.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "CoreUtils/log.hpp"
#include "DataStorage/io_factory.hpp"
#include "DataStorage/image.hpp"
#include "ImageIO/common.hpp"

int main( int argc, char *argv[] )
{
	ENABLE_LOG( isis::DataDebug, isis::util::DefaultMsgPrint, isis::info );
	ENABLE_LOG( isis::DataLog, isis::util::DefaultMsgPrint, isis::info );
	ENABLE_LOG( isis::CoreDebug, isis::util::DefaultMsgPrint, isis::info );
	ENABLE_LOG( isis::CoreLog, isis::util::DefaultMsgPrint, isis::info );
	ENABLE_LOG( isis::ImageIoDebug, isis::util::DefaultMsgPrint, isis::info );
	ENABLE_LOG( isis::ImageIoLog, isis::util::DefaultMsgPrint, isis::info );
	std::list<isis::data::Image> img = isis::data::IOFactory::load( "/scr/feige1/tmp/data.nii", "" );
	/*/SCR/Programming2/hellrung/isis/Debug/tests/ImageIO/*/
	isis::data::IOFactory::write( img, "/tmp/delme.nii", "" );
	return EXIT_SUCCESS;
}
