#ifndef GLMEMORYMANAGER_HPP
#define GLMEMORYMANAGER_HPP

#define GL_GLEXT_PROTOTYPES

#include "DataContainer.hpp"
#include "ViewerCoreBase.hpp"
#include "GL/gl.h"
#include <DataStorage/typeptr.hpp>
#include <DataStorage/image.hpp>

#include <limits>

namespace isis
{
namespace viewer
{

/**
 * This class is responsible for copying the image data to a GL_TEXTURE_3D.
 * It gets the data as a boost::weak_ptr and copies it to the texture.
 */

class GLTextureHandler
{
public:
	enum ScalingType { no_scaling, automatic_scaling, manual_scaling };
	enum InterpolationType { neares_neighbor, linear };
	
	GLTextureHandler() { m_CutAway = std::make_pair<float, float>(0.05,0.05); }
	
	///The image map is a mapping of the imageID and timestep to the texture of the GL_TEXTURE_3D.
	typedef std::map<ImageHolder, std::map<size_t, GLuint > > ImageMapType;

	///Convinient function to copy all in DataContainer available volumes to a GL_TEXTURE_3D.
	std::map<ImageHolder, GLuint> copyAllImagesToTextures( const DataContainer &data, ScalingType scaling = automatic_scaling, InterpolationType interpolation = neares_neighbor );

	///Copies the given timestep of an image with the given imageID to a GL_TEXTURE_3D. Return the texture id.
	GLuint copyImageToTexture( const DataContainer &data, const ImageHolder &image, size_t timestep, ScalingType scaling = automatic_scaling, InterpolationType interpolation = neares_neighbor );

	///The image map is a mapping of the imageID and timestep to the texture of the GL_TEXTURE_3D.
	ImageMapType getImageMap() const { return m_ImageMap; }
	
	void setMinMax( const std::pair<double, double> minMax ) { m_MinMax = minMax; }
	
	void setCutAway( std::pair<float, float> cutAway ) const { m_CutAway = cutAway; }

private:

	ImageMapType m_ImageMap;
	//this is only needed if one specifies the manual scaling
	std::pair<double, double> m_MinMax;
	std::pair<float, float> m_CutAway;
	
	template<typename TYPE>
	GLuint internCopyImageToTexture( const DataContainer &data, GLenum format, const ImageHolder &image, size_t timestep, ScalingType scalingType = automatic_scaling, InterpolationType interpolation = neares_neighbor  ) {
		LOG( Debug, info ) << "Copy image " << image.getID() << " with timestep " << timestep << " to texture";
		GLuint texture;
		util::FixedVector<size_t, 4> size = image.getImageSize();
		TYPE *dataPtr = static_cast<TYPE *>( data.getImageWeakPointer( image, timestep ).lock().get() );
		assert( dataPtr != 0 );
		float pixelBias = 0;
		float pixelScaling = 1;
		switch (scalingType) 
		{
			case no_scaling:
				LOG( Debug, info ) << "No scaling.";
				break;
			case automatic_scaling:
			{
				TYPE maxTypeValue = std::numeric_limits<TYPE>::max();
				TYPE minTypeValue = std::numeric_limits<TYPE>::min();
				TYPE minImage = image.getInternMinMax().first->as<TYPE>();
				TYPE maxImage = image.getInternMinMax().second->as<TYPE>();
				TYPE extent = maxImage - minImage;
				double histogram[extent];
				size_t stepSize = 2;
				size_t volume = size[0] * size[1] * size[2];
				size_t numberOfVoxels = volume / stepSize;
				//initialize the histogram with 0
				for( TYPE i = 0; i< extent; i++){
					histogram[i] = 0;
				}
				//create the histogram
				for( size_t i = 0;i< volume; i+=stepSize)
				{
					histogram[dataPtr[i]]++;
				}
				//normalize histogram
				for( TYPE i = 0;i< extent; i++)
				{
					histogram[i] /= numberOfVoxels;
				}
				TYPE upperBorder = extent-1;
				TYPE lowerBorder = 0;
				double sum = 0;
				//cut away 5% from top
				while(sum < m_CutAway.second) 
				{	
					sum += histogram[upperBorder--];
					
				}
				sum = 0;
				//cut away 5% from below
				while (sum < m_CutAway.first)
				{
					sum += histogram[lowerBorder++];
				}
				pixelBias = (1.0 / extent) * lowerBorder;
				pixelScaling = (float)maxTypeValue / float(upperBorder - lowerBorder);
				LOG( Debug, info ) << "Automatic scaling -> scaling: " << pixelScaling << " -> bias: " << pixelBias;
				break;
			}
			case manual_scaling:
			{
				double minImage = image.getMinMax().first->as<double>();
				double maxImage = image.getMinMax().second->as<double>();;
				double extent = maxImage - minImage;
				pixelBias = (1.0 / extent) * (m_MinMax.first - minImage );
				pixelScaling = extent / (m_MinMax.second - (m_MinMax.first - minImage));
				break;
			}
		}
		glPixelTransferf( GL_RED_SCALE, pixelBias );
		glPixelTransferf( GL_GREEN_SCALE, pixelBias );
		glPixelTransferf( GL_BLUE_SCALE, pixelBias );
		glPixelTransferf( GL_RED_SCALE, pixelScaling);
		glPixelTransferf( GL_BLUE_SCALE, pixelScaling);
		glPixelTransferf( GL_GREEN_SCALE, pixelScaling);
		GLint interpolationType = 0;
		switch (interpolation)
		{
			case neares_neighbor:
				interpolationType = GL_NEAREST;
				break;
			case linear:
				interpolationType = GL_LINEAR;
				break;
		}
		glShadeModel( GL_FLAT );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_3D, texture );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, interpolationType );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, interpolationType );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
		glTexImage3D( GL_TEXTURE_3D, 0, 3,
					  size[0],
					  size[1],
					  size[2], 0, GL_LUMINANCE, format,
					  dataPtr );
		m_ImageMap[image].insert( std::make_pair<size_t, GLuint >( timestep, texture ) );
		return texture;
	}

};

}
}

#endif