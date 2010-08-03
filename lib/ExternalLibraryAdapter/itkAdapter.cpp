/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
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
 *****************************************************************/

#include "itkAdapter.hpp"
namespace isis
{

namespace adapter
{

template<typename TImage> typename TImage::Pointer
itkAdapter::makeItkImageObject( const boost::shared_ptr<data::Image> src, const bool behaveAsItkReader )
{
	typedef TImage OutputImageType;
	m_ImageISIS = *src;
	m_TypeID = m_ImageISIS.getChunkAt(0).typeID();

	switch ( m_TypeID ) {
	case data::TypePtr<int8_t>::staticID:
		return this->internCreateItk<int8_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::TypePtr<u_int8_t>::staticID:
		return this->internCreateItk<u_int8_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::TypePtr<int16_t>::staticID:
		return this->internCreateItk<int16_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::TypePtr<u_int16_t>::staticID:
		return this->internCreateItk<u_int16_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::TypePtr<int32_t>::staticID:
		return this->internCreateItk<int32_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::TypePtr<u_int32_t>::staticID:
		return this->internCreateItk<u_int32_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::TypePtr<float>::staticID:
		return this->internCreateItk<float, OutputImageType>( behaveAsItkReader );
		break;
	case data::TypePtr<double>::staticID:
		return this->internCreateItk<double, OutputImageType>( behaveAsItkReader );
		break;
	default:
		LOG( DataLog, error ) << "Unknown pixel data type";
		return 0;
	}
}
template<typename TImage> data::ImageList
itkAdapter::makeIsisImageObject( const typename TImage::Pointer src, const bool behaveAsItkWriter )
{
	if( m_TypeID ) {
		switch ( m_TypeID ) {
		case data::TypePtr<int8_t>::staticID:
			return this->internCreateISIS<TImage, int8_t>( src, behaveAsItkWriter );
			break;
		case data::TypePtr<u_int8_t>::staticID:
			return this->internCreateISIS<TImage, u_int8_t>( src, behaveAsItkWriter );
			break;
		case data::TypePtr<int16_t>::staticID:
			return this->internCreateISIS<TImage, int16_t>( src, behaveAsItkWriter );
			break;
		case data::TypePtr<u_int16_t>::staticID:
			return this->internCreateISIS<TImage, u_int16_t>( src, behaveAsItkWriter );
			break;
		case data::TypePtr<int32_t>::staticID:
			return this->internCreateISIS<TImage, int32_t>( src, behaveAsItkWriter );
			break;
		case data::TypePtr<u_int32_t>::staticID:
			return this->internCreateISIS<TImage, u_int32_t>( src, behaveAsItkWriter );
			break;
		case data::TypePtr<float>::staticID:
			return this->internCreateISIS<TImage, float>( src, behaveAsItkWriter );
			break;
		case data::TypePtr<double>::staticID:
			return this->internCreateISIS<TImage, double>( src, behaveAsItkWriter );
			break;
		default:
			return this->internCreateISIS<TImage, typename TImage::PixelType>( src, behaveAsItkWriter );
			break;
		}
	} else {
		return this->internCreateISIS<TImage, typename TImage::PixelType>( src, behaveAsItkWriter );
	}
}

template<typename TInput, typename TOutput>
typename TOutput::Pointer itkAdapter::internCreateItk( const bool behaveAsItkReader )
{
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
	//itk::MetaDataDictionary myItkDict;
	// since ITK uses a dialect of the Nifti image space, we need to transform
	// the image metadata into a nifti coordinate system
	//declare transformation matrix T (NIFTI -> DICOM)
	// -1  1  0
	//  0 -1  0
	//  0  0  1
	boost::numeric::ublas::matrix<float> T( 3, 3 );
	T( 0, 0 ) = -1;
	T( 0, 1 ) = 0;
	T( 0, 2 ) = 0;
	T( 1, 0 ) = 0;
	T( 1, 1 ) = -1;
	T( 1, 2 ) = 0;
	T( 2, 0 ) = 0;
	T( 2, 1 ) = 0;
	T( 2, 2 ) = 1;
	// apply transformation to local isis image copy
	m_ImageISIS.transformCoords( T );

	//getting the required metadata from the isis image
	const util::fvector4 dimensions( m_ImageISIS.sizeToVector() );
	const util::fvector4 indexOrigin( m_ImageISIS.getProperty<util::fvector4>( "indexOrigin" ) );
	util::fvector4 spacing( m_ImageISIS.getProperty<util::fvector4>( "voxelSize" ) );
	if(spacing[3] == 0) { spacing[3] = 1; }
	const util::fvector4 readVec = m_ImageISIS.getProperty<util::fvector4>( "readVec" );
	const util::fvector4 phaseVec = m_ImageISIS.getProperty<util::fvector4>( "phaseVec" );
	const util::fvector4 sliceVec = m_ImageISIS.getProperty<util::fvector4>( "sliceVec" );
	//  std::cout << "indexOrigin: " << indexOrigin << std::endl;
	//  std::cout << "readVec: " << readVec << std::endl;
	//  std::cout << "phaseVec: " << phaseVec << std::endl;
	//  std::cout << "sliceVec: " << sliceVec << std::endl;
	//  std::cout << "spacing: " << spacing << std::endl;

	for ( unsigned short i = 0; i < 3; i++ ) {
		itkOrigin[i] = indexOrigin[i];
		itkSize[i] = dimensions[i];
		itkSpacing[i] = spacing[i];
		itkDirection[i][0] = readVec[i];
		itkDirection[i][1] = phaseVec[i];
		itkDirection[i][2] = sliceVec[i];
	}

	// To mimic the behavior of the itk nifti image io plugin the
	// orientation matrix will be transformed this way:

	/*
	-1 -1 -1 -1
	-1 -1 -1 -1
	 1  1  1  1
	 */
	if ( behaveAsItkReader ) {
		itkOrigin[0] = -indexOrigin[0];
		itkOrigin[1] = -indexOrigin[1];
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

	//temporary reorganisation of memory according to the chunk organisiation
	void* targePtr = malloc(m_ImageISIS.bytes_per_voxel() * m_ImageISIS.volume());
	typename InputImageType::PixelType* refTarget = ( typename InputImageType::PixelType* ) targePtr;
	std::vector< boost::shared_ptr< data::Chunk> > chList = m_ImageISIS.getChunkList();
	size_t chunkIndex = 0;

	BOOST_FOREACH( boost::shared_ptr< data::Chunk> & ref, chList)
	{
		data::Chunk &chRef=*ref;
		typename InputImageType::PixelType* target = refTarget + chunkIndex++ * chRef.volume();
		chRef.getTypePtr<typename InputImageType::PixelType>().copyToMem(0, (chRef.volume() - 1), target );
	}
	importer->SetImportPointer( refTarget, itkSize[0], false );
	rescaler->SetInput( importer->GetOutput() );
	typename InputImageType::PixelType minIn, maxIn;
	m_ImageISIS.getMinMax( minIn, maxIn );
	rescaler->SetOutputMinimum( minIn );
	rescaler->SetOutputMaximum( maxIn );
	rescaler->Update();
	outputImage = rescaler->GetOutput();
	//since itk properties do not match the isis properties we need to define metaproperties to prevent data loss
	propKeyList  = m_ImageISIS.getKeys();
	BOOST_FOREACH( PropKeyListType::const_reference ref, propKeyList ) {
		if ( m_ImageISIS.propertyValue( ref )->is<util::fvector4>() ) {
			itk::EncapsulateMetaData<util::fvector4>( m_ITKDict, ref, m_ImageISIS.getProperty<util::fvector4>( ref ) );
		} else {
			itk::EncapsulateMetaData<std::string>( m_ITKDict, ref, m_ImageISIS.getProperty<std::string>( ref ) );
		}
	}
	outputImage->SetMetaDataDictionary( m_ITKDict );
	return outputImage;
}

template<typename TImageITK, typename TOutputISIS> data::ImageList itkAdapter::internCreateISIS( const typename TImageITK::Pointer src, const bool behaveAsItkWriter )
{
	typename TImageITK::PointType indexOrigin = src->GetOrigin();
	typename TImageITK::SizeType imageSize = src->GetBufferedRegion().GetSize();
	typename TImageITK::SpacingType imageSpacing = src->GetSpacing();
	typename TImageITK::DirectionType imageDirection = src->GetDirection();

	if ( TImageITK::ImageDimension < 4 ) {
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

	//  std::cout << "GNA: " << indexOrigin << std::endl;
	//  std::cout << "indexOrigin: " << indexOrigin << std::endl;
	//  std::cout << "itkDirection: " << imageDirection << std::endl;
	// TODO use MemImage instead of MemChunk.
	boost::shared_ptr<data::MemChunk< typename TImageITK::PixelType > >
	retChunk( new data::MemChunk< typename TImageITK::PixelType  >( src->GetBufferPointer(), imageSize[0], imageSize[1], imageSize[2], imageSize[3] ) ) ;

	if ( m_ITKDict.HasKey( "sequenceNumber" ) ) {
		std::string sequenceNumber;
		itk::ExposeMetaData<std::string>( m_ITKDict, "sequenceNumber", sequenceNumber );
		retChunk->setProperty( "sequenceNumber", atoi( sequenceNumber.c_str() ) );
	} else {
		retChunk->setProperty( "sequenceNumber", 1 );
	}

	if ( m_ITKDict.HasKey( "acquisitionNumber" ) ) {
		std::string acquisitionNumber;
		itk::ExposeMetaData<std::string>( m_ITKDict, "acquisitionNumber", acquisitionNumber );
		retChunk->setProperty( "acquisitionNumber", atoi( acquisitionNumber.c_str() ) );
	} else {
		retChunk->setProperty( "acquisitionNumber",  imageSize[TImageITK::ImageDimension] );
	}

	if ( m_ITKDict.HasKey( "voxelGap" ) ) {
		util::fvector4 voxelGap;
		itk::ExposeMetaData<util::fvector4>( m_ITKDict, "voxelGap", voxelGap );
		retChunk->setProperty( "voxelGap", voxelGap );
	}

	retChunk->setProperty( "indexOrigin", util::fvector4( indexOrigin[0], indexOrigin[1], indexOrigin[2], indexOrigin[3] ) );
	retChunk->setProperty( "readVec", util::fvector4( imageDirection[0][0], imageDirection[1][0], imageDirection[2][0], 0 ) );
	retChunk->setProperty( "phaseVec", util::fvector4( imageDirection[0][1], imageDirection[1][1], imageDirection[2][1], 0 ) );
	retChunk->setProperty( "sliceVec", util::fvector4( imageDirection[0][2], imageDirection[1][2], imageDirection[2][2], 0 ) );
	retChunk->setProperty( "voxelSize", util::fvector4( imageSpacing[0], imageSpacing[1], imageSpacing[2], imageSpacing[3] ) );
	//do not try to grasp that in a sober state!!
	data::ChunkList chunkList;
	chunkList.push_back( *retChunk );
	//  std::cout << "prior: " << retChunk->propertyValue("indexOrigin");
	data::ImageList isisImageList( chunkList );
	boost::shared_ptr< data::TypedImage< TOutputISIS > > retImage (
		new data::TypedImage<TOutputISIS>  ( *isisImageList.front().get() ) );
	data::ImageList retList;
	//  std::cout << "prior: " << retImage->propertyValue("indexOrigin");
	retList.push_back( retImage );
	//declare transformation matrix T (NIFTI -> DICOM)
	// -1  1  0
	//  0 -1  0
	//  0  0  1
	boost::numeric::ublas::matrix<float> T( 3, 3 );
	T( 0, 0 ) = -1;
	T( 0, 1 ) = 0;
	T( 0, 2 ) = 0;
	T( 1, 0 ) = 0;
	T( 1, 1 ) = -1;
	T( 1, 2 ) = 0;
	T( 2, 0 ) = 0;
	T( 2, 1 ) = 0;
	T( 2, 2 ) = 1;
	// apply transformation to local isis image copy
	BOOST_FOREACH( data::ImageList::const_reference ref, retList ) {
		ref->transformCoords( T );
	}
	return retList;
}

}
}
