#ifndef VIEWERCORE_HPP
#define VIEWERCORE_HPP


#include "ViewerCoreBase.hpp"
#include "QGLWidgetImplementation.hpp"
#include <QtGui>
#include "Color.hpp"

namespace isis
{
namespace viewer
{

class QViewerCore : public QObject, public ViewerCoreBase
{
	Q_OBJECT
public:
	enum Actions {not_specified, timestep_changed, show_labels};
	typedef std::map<std::string, QWidget * > WidgetMap;
	QViewerCore( );

	virtual bool registerWidget( std::string key, QWidget *widget, Actions = not_specified );

	virtual void addImageList( const std::list<data::Image> imageList, const ImageHolder::ImageType &imageType, const util::slist &filenames = util::slist() );
	virtual void setImageList( const std::list<data::Image> imageList, const ImageHolder::ImageType &imageType, const util::slist &filenames = util::slist() );

	const WidgetMap &getWidgets() const { return m_WidgetMap; }

	std::vector< util::fvector4 > getRGBColorGradient() const { return m_RGBColorGradient; }

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


public Q_SLOTS:
	virtual void voxelCoordChanged( util::ivector4 );
	virtual void timestepChanged( int );
	virtual void setShowLabels( bool );
	virtual void updateScene();

Q_SIGNALS:
	void emitVoxelCoordChanged( util::ivector4 );
	void emitTimeStepChange( unsigned int );
	void emitImagesChanged( DataContainer );
	void emitShowLabels( bool );
	void emitUpdateScene();

private:
	//this map holds the widgets associated with a given name
	WidgetMap m_WidgetMap;
	std::vector< util::fvector4 > m_RGBColorGradient;

};


}
} // end namespace






#endif
