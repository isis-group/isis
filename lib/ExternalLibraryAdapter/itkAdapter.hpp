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
#include "CoreUtils/common.hpp"

//external includes
#include <boost/shared_ptr.hpp>

//itk includes
#include <itkImage.h>
#include <itkImportImageFilter.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkNumericTraits.h>
#include <itkMetaDataObject.h>

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
	/**
	  * Convert a isis image object in an itk image.
	  * \param behaveAsItkReader if set to true, the orientation matrix will be transformed like <br>
	  * -1 -1 -1 -1 <br>
	  * -1 -1 -1 -1 <br>
	  *  1  1  1  1 <br>
	  *  to meet the itk intern data representation requirements.<br>
	  *  If set to false, orientation matrix will not be changed.
	  *  \returns an itk smartpointer on the itkImage object
	  */
	template<typename TImage> static typename TImage::Pointer
	makeItkImageObject( const boost::shared_ptr<data::Image> src, const bool behaveAsItkReader = true ) {
		typedef TImage OutputImageType;
		itkAdapter* myAdapter = new itkAdapter( src );

		switch ( src->chunksBegin()->typeID() ) {
		case util::TypePtr<int8_t>::staticID:
			return myAdapter->internCreateItk<int8_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<u_int8_t>::staticID:
			return myAdapter->internCreateItk<u_int8_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<int16_t>::staticID:
			return myAdapter->internCreateItk<int16_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<u_int16_t>::staticID:
			return myAdapter->internCreateItk<u_int16_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<int32_t>::staticID:
			return myAdapter->internCreateItk<int32_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<u_int32_t>::staticID:
			return myAdapter->internCreateItk<u_int32_t, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<float>::staticID:
			return myAdapter->internCreateItk<float, OutputImageType>( behaveAsItkReader );
			break;
		case util::TypePtr<double>::staticID:
			return myAdapter->internCreateItk<double, OutputImageType>( behaveAsItkReader );
			break;
		}
	}

	template<typename TImage> static data::ImageList
	makeIsisImageObject( const typename TImage::Pointer src, const bool behaveAsItkWriter = true ) {
		typename TImage::PointType indexOrigin = src->GetOrigin();
		typename TImage::SizeType imageSize = src->GetBufferedRegion().GetSize();
		typename TImage::SpacingType imageSpacing = src->GetSpacing();
		typename TImage::DirectionType imageDirection = src->GetDirection();

		if ( TImage::ImageDimension < 4 ) {
			imageSize[3] = 1;
		}

		if ( behaveAsItkWriter ) {
			imageDirection[0][0] = -imageDirection[0][0];
			imageDirection[0][1] = -imageDirection[0][1];
			imageDirection[0][2] = -imageDirection[0][2];
			imageDirection[1][0] = -imageDirection[1][0];
			imageDirection[1][1] = -imageDirection[1][1];
			imageDirection[1][2] = -imageDirection[1][2];
			indexOrigin[0] = -indexOrigin[0];
			indexOrigin[1] = -indexOrigin[1];
		}

		boost::shared_ptr<data::MemChunk<typename TImage::PixelType > >
		retChunk( new data::MemChunk<typename TImage::PixelType >( src->GetBufferPointer(), imageSize[0], imageSize[1], imageSize[2], imageSize[3] ) );
		retChunk->setProperty( "indexOrigin", util::fvector4( indexOrigin[0], indexOrigin[1], indexOrigin[2], indexOrigin[3] ) );
		retChunk->setProperty( "readVec", util::fvector4( imageDirection[0][0], imageDirection[0][1], imageDirection[0][2], 0 ) );
		retChunk->setProperty( "phaseVec", util::fvector4( imageDirection[1][0], imageDirection[1][1], imageDirection[1][2], 0 ) );
		retChunk->setProperty( "sliceVec", util::fvector4( imageDirection[2][0], imageDirection[2][1], imageDirection[2][2], 0 ) );
		retChunk->setProperty( "voxelSize", util::fvector4( imageSpacing[0], imageSpacing[1], imageSpacing[2], imageSpacing[3] ) );
		retChunk->setProperty( "centerVec", util::fvector4( indexOrigin[0], indexOrigin[1], indexOrigin[2], indexOrigin[3] ) );
		itk::MetaDataDictionary myItkDict = src->GetMetaDataDictionary();

		if ( myItkDict.HasKey( "sequenceNumber" ) ) {
			std::string sequenceNumber;
			itk::ExposeMetaData<std::string>( myItkDict, "sequenceNumber", sequenceNumber );
			retChunk->setProperty( "sequenceNumber", atoi( sequenceNumber.c_str() ) );
		} else {
			retChunk->setProperty( "sequenceNumber", 1 );
		}

		if ( myItkDict.HasKey( "acquisitionNumber" ) ) {
			std::string acquisitionNumber;
			itk::ExposeMetaData<std::string>( myItkDict, "acquisitionNumber", acquisitionNumber );
			retChunk->setProperty( "acquisitionNumber", atoi( acquisitionNumber.c_str() ) );
		} else {
			for ( unsigned int t = 0; t < imageSize[3]; t++ ) {
				retChunk->setProperty( "acquisitionNumber", t );
			}
		}

		data::ChunkList chunkList;
		chunkList.push_back( *retChunk );
		data::ImageList isisImageList( chunkList );
		return isisImageList;
	}




protected:
	//should not be loaded directly
	itkAdapter( const boost::shared_ptr<data::Image> src ) : m_ImageISIS( src ) {};
	itkAdapter( const itkAdapter& ) {};
	itkAdapter() {};
private:

	boost::shared_ptr<data::Image> m_ImageISIS;

	template<typename TInput, typename TOutput> typename TOutput::Pointer internCreateItk( const bool behaveAsItkReader ) {
		typedef itk::Image<TInput, TOutput::ImageDimension> InputImageType;
		typedef TOutput OutputImageType;
		typedef itk::ImportImageFilter<typename InputImageType::PixelType, OutputImageType::ImageDimension> MyImporterType;
		typedef itk::RescaleIntensityImageFilter<InputImageType, OutputImageType> MyRescaleType;
		typedef std::set<std::string, isis::util::_internal::caselessStringLess> PropKeyListType;
		typename MyImporterType::Pointer importer = MyImporterType::New();
		typename MyRescaleType::Pointer rescaler = MyRescaleType::New();
		typename OutputImageType::Pointer outputImage = OutputImageType::New();
		typename OutputImageType::SpacingType itkSpacing;
		typename OutputImageType::PointType itkOrigin;
		typename OutputImageType::DirectionType itkDirection;
		typename OutputImageType::SizeType itkSize;
		typename OutputImageType::RegionType itkRegion;
		PropKeyListType propKeyList;
		itk::MetaDataDictionary myItkDict;
		//getting the required metadata from the isis image
		const util::fvector4 dimensions( m_ImageISIS->sizeToVector() );
		const util::fvector4 indexOrigin( m_ImageISIS->getProperty<util::fvector4>( "indexOrigin" ) );
		const util::fvector4 spacing( m_ImageISIS->getProperty<util::fvector4>( "voxelSize" ) );
		const util::fvector4 readVec = m_ImageISIS->getProperty<util::fvector4>( "readVec" );
		const util::fvector4 phaseVec = m_ImageISIS->getProperty<util::fvector4>( "phaseVec" );
		const util::fvector4 sliceVec = m_ImageISIS->getProperty<util::fvector4>( "sliceVec" );

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
		outputImage = rescaler->GetOutput();
		//since itk properties do not match the isis properties we need to define metaproperties to prevent data loss
		propKeyList  = m_ImageISIS->keys();
		PropKeyListType::const_iterator propIter;

		for ( propIter = propKeyList.begin(); propIter != propKeyList.end(); propIter++ ) {
			itk::EncapsulateMetaData<std::string>( myItkDict, *propIter, m_ImageISIS->getPropertyValue( *propIter ).toString() );
		}

		outputImage->SetMetaDataDictionary( myItkDict );
		return outputImage;
	}
};

}
}// end namespace


#endif
