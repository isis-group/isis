
#ifndef WIDGETADAPTERBASE_HPP
#define WIDGETADAPTERBASE_HPP

#include <QWidget>
#include <boost/shared_ptr.hpp>


namespace isis {
namespace viewer {

class WidgetAdapterBase
{
public:
	virtual void setParent( QWidget* parent ) = 0;
	
// 	WidgetAdapterBase() {};

	
};


}} // end namespace

#endif