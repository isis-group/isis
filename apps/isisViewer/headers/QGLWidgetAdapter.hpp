#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#include <QtOpenGL/QGLWidget>
#include "WidgetAdapterBase.hpp"
#include "ViewerCoreBase.hpp"
#include <iostream>

namespace isis {
namespace viewer {
	

class QGLWidgetAdapter : public WidgetAdapterBase
{
	struct QGLWidgetImpl : public QGLWidget
	{
		QGLWidgetImpl( QWidget* p ) : QGLWidget( p ) {};
		virtual void initializeGL()
		{
			std::cout << "gna" << std::endl;
		}
		
	};
	
	
	
	
	
public:
	QGLWidgetAdapter( boost::shared_ptr<ViewerCoreBase> core);

	void setParent( QWidget* parent );
	
	void paint() const;
	
private:
	boost::shared_ptr<ViewerCoreBase> m_ViewerCore;
	boost::shared_ptr<QGLWidgetImpl> m_Target;
	bool m_Valid;
	
	
	
		
};




}}//end namespace

#endif