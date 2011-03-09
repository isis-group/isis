#ifndef MAINWINDOWBASE_HPP
#define MAINWINDOWBASE_HPP

#include <QtGui>
#include "ui_isisViewerMain.h"

namespace isis {
namespace viewer {

class MainWindowBase : public QMainWindow
{
	Q_OBJECT
public:
	MainWindowBase(QWidget* parent = 0);
	
	
private Q_SLOTS:
	
Q_SIGNALS:
		
	
protected:
	Ui::isisViewerMain ui;
		
};
	
	
	
}} // end namespace



#endif
