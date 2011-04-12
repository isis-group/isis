#ifndef VIEWERCORE_HPP
#define VIEWERCORE_HPP

#include "ViewerCoreBase.hpp"
#include "QGLWidgetImplementation.hpp"
#include <QtGui>

namespace isis
{
namespace viewer
{

class QViewerCore : public QObject, public ViewerCoreBase
{
	Q_OBJECT
public:
	enum Actions {not_specified, timestep_changed};
	typedef std::map<std::string, QWidget * > WidgetMap;
	QViewerCore( );

	virtual bool registerWidget( std::string key, QWidget *widget, Actions = not_specified );

	const WidgetMap &getWidgets() const { return m_WidgetMap; }

	template<typename T>
	T *getWidgetAs( std::string key ) {
		if( m_WidgetMap.find( key ) == m_WidgetMap.end() ) {
			LOG( Runtime, error ) << "A widget with the name " << key << " is not registered!";
			return 0;
		}

		if( dynamic_cast<T *>( m_WidgetMap[key] ) != 0 ) {
			return dynamic_cast<T *>( m_WidgetMap[key] );
		} else {
			LOG( Runtime, error ) << "Error while converting widget " << key << " !";
		}

	};


protected Q_SLOTS:
	virtual void voxelCoordChanged( util::ivector4 );
	virtual void timestepChanged( int );

Q_SIGNALS:
	void emitVoxelCoordChanged( util::ivector4 );
	void emitTimeStepChange( unsigned int );
private:
	//this map holds the widgets associated with a given name
	WidgetMap m_WidgetMap;

};


}
} // end namespace






#endif
