
#ifndef VIEWERCOREBASE_HPP
#define VIEWERCOREBASE_HPP

#include "DataContainer.hpp"
#include "GLOrientationHandler.hpp"
#include <QWidget>
#include <map>

namespace isis
{
namespace viewer
{

class ViewerCoreBase
{

public:
	ViewerCoreBase( data::Image );

	typedef std::map<std::string, QWidget * > WidgetMap;

	void setImageList( const std::list<data::Image> );
	bool registerWidget( std::string key, QWidget *widget );

	const ImageHolder &getCurrentImage() const { return m_CurrentImage; }
	size_t getCurrentTimestep() const { return m_CurrentTimestep; }
	bool setCurrentTimestep( size_t timestep ) {
		if(  m_CurrentImage.getImageSize()[3] > m_CurrentTimestep ) {
			m_CurrentTimestep = timestep;
			return true;
		} else { return false; }
	}

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

	const DataContainer &getDataContainer() const { return m_DataContainer; }

private:
	//this is the container which actually holds all the images
	DataContainer m_DataContainer;

	//this map holds the widgets associated with a given name
	WidgetMap m_WidgetMap;

	ImageHolder m_CurrentImage;
	size_t m_CurrentTimestep;


};




}
} // end namespace



#endif