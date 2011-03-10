/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#include "MainWindow.hpp"
#include <common.hpp>

namespace isis {
namespace viewer {

MainWindow::MainWindow( boost::shared_ptr<ViewerCoreBase> core )
	  : m_ViewerCore( core )
{
	ui.setupUi( this );
	
	
	
}

}} //end namespace
