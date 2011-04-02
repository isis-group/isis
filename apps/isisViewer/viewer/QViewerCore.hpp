#ifndef VIEWERCORE_HPP
#define VIEWERCORE_HPP

#include "ViewerCoreBase.hpp"
#include "QGLWidgetImplementation.hpp"

namespace isis
{
namespace viewer
{
	
class QViewerCore : public QObject, public ViewerCoreBase
{
	Q_OBJECT
public:
	typedef std::map<std::string, QWidget * > WidgetMap;
	QViewerCore( data::Image );
	
	virtual bool registerWidget( std::string key, QWidget *widget );
	
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
	void voxelCoordChanged( util::ivector4 );
private:
	//this map holds the widgets associated with a given name
	WidgetMap m_WidgetMap;

};


}
} // end namespace






#endif
