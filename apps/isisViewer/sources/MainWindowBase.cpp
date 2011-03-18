#include "MainWindowBase.hpp"
#include <iostream>

namespace isis
{
namespace viewer
{

MainWindowBase::MainWindowBase( QWidget *parent )
	: QMainWindow( parent )
{
	ui.setupUi( this );


}

}
} // end namespace