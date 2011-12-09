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
#include <DataStorage/chunk.hpp>
#include <DataStorage/io_factory.hpp>

namespace isis
{

namespace adapter
{

template<typename TImage> typename TImage::Pointer
itkAdapter::makeItkImageObject( const data::Image &src, const bool behaveAsItkReader )
{
	typedef TImage OutputImageType;
	m_ImageISIS = boost::shared_ptr< data::Image >( new data::Image( src ) );
	m_TypeID = m_ImageISIS->getChunkAt( 0 ).getTypeID();

	switch ( m_TypeID ) {
	case data::ValuePtr<int8_t>::staticID:
		return this->internCreateItk<int8_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::ValuePtr<u_int8_t>::staticID:
		return this->internCreateItk<u_int8_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::ValuePtr<int16_t>::staticID:
		return this->internCreateItk<int16_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::ValuePtr<u_int16_t>::staticID:
		return this->internCreateItk<u_int16_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::ValuePtr<int32_t>::staticID:
		return this->internCreateItk<int32_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::ValuePtr<u_int32_t>::staticID:
		return this->internCreateItk<u_int32_t, OutputImageType>( behaveAsItkReader );
		break;
	case data::ValuePtr<float>::staticID:
		return this->internCreateItk<float, OutputImageType>( behaveAsItkReader );
		break;
	case data::ValuePtr<double>::staticID:
		return this->internCreateItk<double, OutputImageType>( behaveAsItkReader );
		break;
	default:
		LOG( DataLog, error ) << "Unknown pixel data type";
		return 0;
	}
}
template<typename TImage> std::list<data::Image>
itkAdapter::makeIsisImageObject( const typename TImage::Pointer src, const bool behaveAsItkWriter )
{
	if( m_TypeID ) {
		switch ( m_TypeID ) {
		case data::ValuePtr<int8_t>::staticID:
			return this->internCreateISIS<TImage, int8_t>( src, behaveAsItkWriter );
			break;
		case data::ValuePtr<u_int8_t>::staticID:
			return this->internCreateISIS<TImage, u_int8_t>( src, behaveAsItkWriter );
			break;
		case data::ValuePtr<int16_t>::staticID:
			return this->internCreateISIS<TImage, int16_t>( src, behaveAsItkWriter );
			break;
		case data::ValuePtr<u_int16_t>::staticID:
			return this->internCreateISIS<TImage, u_int16_t>( src, behaveAsItkWriter );
			break;
		case data::ValuePtr<int32_t>::staticID:
			return this->internCreateISIS<TImage, int32_t>( src, behaveAsItkWriter );
			break;
		case data::ValuePtr<u_int32_t>::staticID:
			return this->internCreateISIS<TImage, u_int32_t>( src, behaveAsItkWriter );
			break;
		case data::ValuePtr<float>::staticID:
			return this->internCreateISIS<TImage, float>( src, behaveAsItkWriter );
			break;
		case data::ValuePtr<double>::staticID:
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
	typedef std::set<util::istring> PropKeyListType;
	typename MyImporterType::Pointer importer = MyImporterType::New();
	typename MyRescaleType::Pointer rescaler = MyRescaleType::New();
	typename OutputImageType::Pointer outputImage = OutputImageType::New();
	typename OutputImageType::SpacingType itkSpacing;
	typename OutputImageType::PointType itkOrigin;
	typename OutputImageType::DirectionType itkDirection;
	typename OutputImageType::SizeType itkSize;
	typename OutputImageType::RegionType itkRegion;
	// util::DefaultMsgPrint::stopBelow( warning );
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
	m_ImageISIS->transformCoords( T );
	//getting the required metadata from the isis image
	const util::fvector4 dimensions( m_ImageISIS->getSizeAsVector() );
	const util::fvector4 indexOrigin( m_ImageISIS->getPropertyAs<util::fvector4>( "indexOrigin" ) );
	util::fvector4 spacing( m_ImageISIS->getPropertyAs<util::fvector4>( "voxelSize" ) );

	if( spacing[3] == 0 ) { spacing[3] = 1; }

	const util::fvector4 readVec = m_ImageISIS->getPropertyAs<util::fvector4>( "rowVec" );

	const util::fvector4 phaseVec = m_ImageISIS->getPropertyAs<util::fvector4>( "columnVec" );

	const util::fvector4 sliceVec = m_ImageISIS->getPropertyAs<util::fvector4>( "sliceVec" );

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
	m_ImagePropertyMap = static_cast<util::PropertyMap>( *m_ImageISIS );
	m_RelevantDim = m_ImageISIS->getChunkAt( 0 ).getRelevantDims();
	//reorganisation of memory according to the chunk organisiation
	void *targePtr = malloc( m_ImageISIS->getBytesPerVoxel() * m_ImageISIS->getVolume() );
	typename InputImageType::PixelType *refTarget = ( typename InputImageType::PixelType * ) targePtr;
	std::vector< data::Chunk> chList = m_ImageISIS->copyChunksToVector();
	size_t chunkIndex = 0;
	BOOST_FOREACH(  std::vector<data::Chunk >::reference ref, chList ) {
		data::Chunk &chRef = ref;
		typename InputImageType::PixelType *target = refTarget + chunkIndex++ * chRef.getVolume();
		chRef.getValuePtr<typename InputImageType::PixelType>().copyToMem( target,  chRef.getVolume() );
		boost::shared_ptr<util::PropertyMap> tmpMap ( new util::PropertyMap ( static_cast<util::PropertyMap>( chRef ) ) );
		m_ChunkPropertyMapVector.push_back( tmpMap );
	}
	importer->SetImportPointer( refTarget, itkSize[0], false );
	rescaler->SetInput( importer->GetOutput() );
	std::pair<util::ValueReference, util::ValueReference> minMaxPair = m_ImageISIS->getMinMax();
	rescaler->SetOutputMinimum( minMaxPair.first->as<typename InputImageType::PixelType>() );
	rescaler->SetOutputMaximum( minMaxPair.second->as<typename InputImageType::PixelType>() );
	rescaler->Update();
	outputImage = rescaler->GetOutput();
	return outputImage;
}

template<typename TImageITK, typename TOutputISIS> std::list<data::Image> itkAdapter::internCreateISIS( const typename TImageITK::Pointer src, const bool behaveAsItkWriter )
{
	typename TImageITK::PointType indexOrigin = src->GetOrigin();
	typename TImageITK::SizeType imageSize = src->GetBufferedRegion().GetSize();
	typename TImageITK::SpacingType imageSpacing = src->GetSpacing();
	typename TImageITK::DirectionType imageDirection = src->GetDirection();
	typedef typename TImageITK::PixelType ITKRepn;
	typedef TOutputISIS ISISRepn;

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

	data::Chunk
	tmpChunk ( data::MemChunk< ITKRepn >( src->GetBufferPointer(), imageSize[0], imageSize[1], imageSize[2], ( TImageITK::ImageDimension == 4 ? imageSize[3] : 1 ) ) ) ;
	//we have to convert the datatype of retChunk to the desired TOutputISIS type to avoid autoscaling
	data::Chunk retChunk ( data::MemChunk<ISISRepn>( imageSize[0], imageSize[1], imageSize[2], ( TImageITK::ImageDimension == 4 ? imageSize[3] : 1 ) ) );
	const data::scaling_pair scale = tmpChunk.getScalingTo( data::ValuePtr<ISISRepn>::staticID, data::noscale );
	//
	data::numeric_convert<ITKRepn, ISISRepn>(
		tmpChunk.asValuePtr<ITKRepn>(),
		retChunk.asValuePtr<ISISRepn>(),
		scale.first->as<double>(),
		scale.second->as<double>() );
	//dummy join to allow creating this chunk
	retChunk.join( m_ImagePropertyMap );

	//since the acquisitionNumber is not stored in the PropertyMap of the image, we have
	//to create a dummy acquisitionNumber
	if ( !retChunk.hasProperty( "acqisitionNumber" ) )
		retChunk.setPropertyAs( "acquisitionNumber", static_cast<u_int32_t>( 1 ) );

	//do not try to grasp that in a sober state!!
	//workaround to create a TypedImage out of a MemChunk
	std::list<data::Chunk> tmpChList = std::list<data::Chunk>( 1, retChunk );
	std::list<data::Image> isisImageList = data::IOFactory::chunkListToImageList( tmpChList );
	data::TypedImage< TOutputISIS >  retImage ( data::TypedImage<TOutputISIS> ( isisImageList.front() ) );
	//these are properties eventually manipulated by itk. So we can not take the
	//parameters from the isis image which was handed over to the itkAdapter
	retImage.setPropertyAs( "indexOrigin", util::fvector4( indexOrigin[0], indexOrigin[1], indexOrigin[2], indexOrigin[3] ) );
	retImage.setPropertyAs( "rowVec", util::fvector4( imageDirection[0][0], imageDirection[1][0], imageDirection[2][0], 0 ) );
	retImage.setPropertyAs( "columnVec", util::fvector4( imageDirection[0][1], imageDirection[1][1], imageDirection[2][1], 0 ) );
	retImage.setPropertyAs( "sliceVec", util::fvector4( imageDirection[0][2], imageDirection[1][2], imageDirection[2][2], 0 ) );
	retImage.setPropertyAs( "voxelSize", util::fvector4( imageSpacing[0], imageSpacing[1], imageSpacing[2], imageSpacing[3] ) );
	//this will splice down the image the same way it was handed over to the itkAdapter
	retImage.spliceDownTo( static_cast<data::dimensions> ( m_RelevantDim ) );

	//add the residual parameters to the image
	retImage.join( m_ImagePropertyMap, false );
	std::vector< data::Chunk > chList = retImage.copyChunksToVector();
	LOG_IF( chList.size() != m_ChunkPropertyMapVector.size(), data::Debug, warning ) << "The image size has changed. The chunk-specific metadata will be interpolated.";
	//iterate through the spliced chunks of the image and set all the chunk specific parameters
	size_t chunkCounter = 0;
	BOOST_FOREACH( std::vector< data::Chunk >::reference chRef, chList ) {
		//TODO if the number of chunks gained by the splice method differs from
		//the size of the m_ChunkPropertyMapVector the size of the image was changed in itk.
		//Thus we have to interpolate the parameters (sliceTime so far)
		chRef.join( static_cast<util::PropertyMap &>( retImage ), false );

		if( chunkCounter < ( m_ChunkPropertyMapVector.size() - 1 ) ) {
			chunkCounter++;
		}

		chRef.join( *m_ChunkPropertyMapVector[chunkCounter], false );
	}
	std::list<data::Image> retList;
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
	BOOST_FOREACH( std::list<data::Image>::reference ref, retList ) {
		ref.transformCoords( T );
	}
	return retList;
}

}
}
