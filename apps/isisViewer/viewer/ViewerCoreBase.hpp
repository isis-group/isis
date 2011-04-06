
#ifndef VIEWERCOREBASE_HPP
#define VIEWERCOREBASE_HPP

#include "DataContainer.hpp"
#include <map>

namespace isis
{
namespace viewer
{

class ViewerCoreBase
{

public:
	ViewerCoreBase( data::Image image );

	virtual void addImageList( const std::list<data::Image> imageList );
	virtual void setImageList( const std::list<data::Image> imageList );

	void setCurrentImage( const ImageHolder &image ) { m_CurrentImage = image; }
	const ImageHolder &getCurrentImage() const { return m_CurrentImage; }
	size_t getCurrentTimestep() const { return m_CurrentTimestep; }
	bool setCurrentTimestep( size_t timestep ) {
		if(  m_CurrentImage.getImageSize()[3] > timestep ) {
			m_CurrentTimestep = timestep;
			return true;
		} else { return false; }
	}



	const DataContainer &getDataContainer() const { return m_DataContainer; }

private:
	//this is the container which actually holds all the images
	DataContainer m_DataContainer;
	ImageHolder m_CurrentImage;
	size_t m_CurrentTimestep;


};




}
} // end namespace



#endif