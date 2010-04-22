/****************************************************************
 *
 * <Copyright information>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2010
 *
 * vtkAdapter.cpp
 *
 * Description:
 *
 *  Created on: Mar,30 2010
 *      Author: tuerke
 ******************************************************************/
#ifndef ITKADAPTER_HPP_
#define ITKADAPTER_HPP_

#include "DataStorage/image.hpp"
#include "CoreUtils/log.hpp"

//external includes
#include <boost/shared_ptr.hpp>

//itk includes
#include <itkImage.h>
#include <itkImportImageFilter.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkNumericTraits.h>

namespace isis
{
namespace adapter
{

/**
  * ITKAdapter is capable of taking an isis image object and return an itkImage object.
  */

class itkAdapter
{
public:
	template<typename TImage> static typename TImage::Pointer
	makeItkImageObject( const boost::shared_ptr<data::Image> src, const bool behaveAsItkReader = true ) {
		typedef TImage OutputImageType;
		itkAdapter* myAdapter = new itkAdapter( src );

		switch ( myAdapter->m_ImageISIS->chunksBegin()->typeID() ) {
		case util::TypePtr<int8_t>::staticID:
			return myAdapter->internCreate<int8_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<u_int8_t>::staticID:
			return myAdapter->internCreate<u_int8_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<int16_t>::staticID:
			return myAdapter->internCreate<int16_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<u_int16_t>::staticID:
			return myAdapter->internCreate<u_int16_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<int32_t>::staticID:
			return myAdapter->internCreate<int32_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<u_int32_t>::staticID:
			return myAdapter->internCreate<u_int32_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<float>::staticID:
			return myAdapter->internCreate<float, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<double>::staticID:
			return myAdapter->internCreate<double, OutputImageType>( behaveAsItkReader );
			break;
		}
	}
protected:
	//should not be loaded directly
	itkAdapter( const boost::shared_ptr<data::Image> );
	itkAdapter( const itkAdapter& ) {};
private:

	boost::shared_ptr<data::Image> m_ImageISIS;

	template<typename TInput, typename TOutput> typename TOutput::Pointer internCreate( const bool behaveAsItkReader ) {
		typedef itk::Image<TInput, TOutput::ImageDimension> InputImageType;
		typedef TOutput OutputImageType;
		typedef itk::ImportImageFilter<typename InputImageType::PixelType, OutputImageType::ImageDimension> MyImporterType;
		typedef itk::RescaleIntensityImageFilter<InputImageType, OutputImageType> MyRescaleType;
		typename MyImporterType::Pointer importer = MyImporterType::New();
		typename MyRescaleType::Pointer rescaler = MyRescaleType::New();
		typename OutputImageType::Pointer outputImage = OutputImageType::New();
		typename InputImageType::Pointer inputImage = InputImageType::New();
		typename OutputImageType::SpacingType itkSpacing;
		typename OutputImageType::PointType itkOrigin;
		typename OutputImageType::DirectionType itkDirection;
		typename OutputImageType::SizeType itkSize;
		typename OutputImageType::RegionType itkRegion;
		//getting the required metadata from the isis image
		const util::fvector4 dimensions( this->m_ImageISIS->sizeToVector() );
		const util::fvector4 indexOrigin( this->m_ImageISIS->getProperty<util::fvector4>( "indexOrigin" ) );
		const util::fvector4 spacing( this->m_ImageISIS->getProperty<util::fvector4>( "voxelSize" ) );
		const util::fvector4 readVec = this->m_ImageISIS->getProperty<util::fvector4>( "readVec" );
		const util::fvector4 phaseVec = this->m_ImageISIS->getProperty<util::fvector4>( "phaseVec" );
		const util::fvector4 sliceVec = this->m_ImageISIS->getProperty<util::fvector4>( "sliceVec" );

		for ( unsigned short i = 0; i < 3; i++ ) {
			itkOrigin[i] = indexOrigin[i];
			itkSize[i] = dimensions[i];
			itkSpacing[i] = spacing[i];
			itkDirection[i][0] = readVec[i];
			itkDirection[i][1] = phaseVec[i];
			itkDirection[i][2] = sliceVec[i];
		}

		//TODO why the hell negates the itkNiftio some seemingly arbitrary elements of the orientation?????
		//matrix will be transformed this way:
		/*
		-1 -1 -1 -1
		-1 -1 -1 -1
		 1  1  1  1
		 */
		if ( behaveAsItkReader ) {
			itkOrigin[0] = -indexOrigin[0];
			itkOrigin[1] = -indexOrigin[1];
			itkOrigin[2] = indexOrigin[2];
			itkDirection[0][0] = -readVec[0];
			itkDirection[0][1] = -phaseVec[0];
			itkDirection[0][2] = -sliceVec[0];
			itkDirection[1][0] = -readVec[1];
			itkDirection[1][1] = -phaseVec[1];
			itkDirection[1][2] = -sliceVec[1];
		}

		//if the user requests a 4d image we need to set these parameters
		if ( OutputImageType::ImageDimension == 4 ) {
			itkSpacing[3] = spacing[3];
			itkSize[3] = dimensions[3];
			itkDirection[3][3] = 1; //ensures determinant is unequal 0
		}

		itkRegion.SetSize( itkSize );
		importer->SetRegion( itkRegion );
		importer->SetSpacing( itkSpacing );
		importer->SetOrigin( itkOrigin );
		importer->SetDirection( itkDirection );
		importer->SetImportPointer( &this->m_ImageISIS->voxel<typename InputImageType::PixelType>( 0, 0, 0, 0 ), itkSize[0], false );
		rescaler->SetInput( importer->GetOutput() );
		typename InputImageType::PixelType minIn, maxIn;
		this->m_ImageISIS->getMinMax<typename InputImageType::PixelType>( minIn, maxIn );
		rescaler->SetOutputMinimum( minIn );
		rescaler->SetOutputMaximum( maxIn );
		rescaler->Update();
		return rescaler->GetOutput();
	}
};

}
}// end namespace


#endif
