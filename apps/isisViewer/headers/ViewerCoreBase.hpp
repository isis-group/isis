
#ifndef VIEWERCOREBASE_HPP
#define VIEWERCOREBASE_HPP

#include "DataContainer.hpp"
#include <QWidget>
#include <map>

namespace isis {
namespace viewer {

class ViewerCoreBase
{

public:
	typedef std::map<std::string, boost::shared_ptr<QWidget> > WidgetMap;
	
	void setImageList( const std::list<data::Image> );
	bool registerWidget( std::string key, boost::shared_ptr<QWidget> widget);
	
	WidgetMap getWidgets() const { return m_WidgetMap; }
	
	template<typename T>
	boost::shared_ptr<T> getWidgetAs( std::string key )
	{
		if( boost::dynamic_pointer_cast<T>( m_WidgetMap[key] ) != 0 )
		{
			return boost::dynamic_pointer_cast<T>( m_WidgetMap[key] );
		} else {
			LOG( Runtime, error ) << "Error while converting widget " << key << " !";
		}
		
	};
	
	DataContainer getDataContainer() const { return m_DataContainer; }
	

private:
	//this is the container which actually holds all the images 
	DataContainer m_DataContainer;
	
	//this map holds the widgets associated with a given name
	WidgetMap m_WidgetMap;
	
		
};
	
	
	
	
}} // end namespace



#endif