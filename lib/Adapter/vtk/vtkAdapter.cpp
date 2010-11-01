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

#include "vtkAdapter.hpp"

namespace isis
{
namespace adapter
{

vtkAdapter::vtkAdapter( const boost::shared_ptr<isis::data::Image> src )
	: m_ImageISIS( src )
{}

//return a list of vtkImageData type pointer
vtkImageData *vtkAdapter::makeVtkImageObject( const boost::shared_ptr<data::Image> src, bool keepType, size_t dim4 )
{
	vtkAdapter *myAdapter;
	vtkImageData *vtkImage = vtkImageData::New();
	vtkImageImport *importer = vtkImageImport::New();
	if( !keepType ) {
		boost::shared_ptr<data::Image> imgPtr ( new data::TypedImage<u_int8_t>( *src ) );
		myAdapter = new vtkAdapter( imgPtr );
		importer->SetDataScalarTypeToUnsignedChar();
		vtkImage->SetScalarTypeToUnsignedChar();
		importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<u_int8_t>( 0,0,0,dim4 ) );
	} else {
		myAdapter = new vtkAdapter( src );
	}
	const util::fvector4 dimensions( myAdapter->m_ImageISIS->sizeToVector() );
	const util::fvector4 indexOrigin( myAdapter->m_ImageISIS->getProperty<util::fvector4>( "indexOrigin" ) );
	const util::fvector4 spacing( myAdapter->m_ImageISIS->getProperty<util::fvector4>( "voxelSize" ) );

	if ( dim4 > dimensions[3] - 1 ) dim4 = dimensions[3] - 1;

	//set the datatype for the vtkImage object
	//TODO check datatypes

	void *targePtr = malloc( myAdapter->m_ImageISIS->bytes_per_voxel() * myAdapter->m_ImageISIS->volume() );
	uint8_t *refTarget = ( uint8_t * ) targePtr;
	std::vector< boost::shared_ptr< data::Chunk> > chList = myAdapter->m_ImageISIS->getChunkList();
	size_t chunkIndex = 0;
	BOOST_FOREACH( boost::shared_ptr< data::Chunk> & ref, chList ) {
		data::Chunk &chRef = *ref;
		uint8_t *target = refTarget + chunkIndex++ * chRef.volume();
		chRef.getTypePtr<uint8_t>().copyToMem( 0, ( chRef.volume() - 1 ), target );
	}


	if ( keepType ) {
		switch ( myAdapter->m_ImageISIS->typeID() ) {
		case data::TypePtr<int8_t>::staticID:
			importer->SetDataScalarTypeToUnsignedChar();
			importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<int8_t>( 0, 0, 0, dim4 ) );
			vtkImage->SetScalarTypeToChar();
			break;
		case data::TypePtr<u_int8_t>::staticID:
			importer->SetDataScalarTypeToUnsignedChar();
			vtkImage->SetScalarTypeToUnsignedChar();
			importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<u_int8_t>( 0, 0, 0, dim4 ) );
			break;
		case data::TypePtr<int16_t>::staticID:
			importer->SetDataScalarTypeToShort();
			vtkImage->SetScalarTypeToShort();
			importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<int16_t>( 0, 0, 0, dim4 ) );
			break;
		case data::TypePtr<u_int16_t>::staticID:
			importer->SetDataScalarTypeToUnsignedShort();
			vtkImage->SetScalarTypeToUnsignedShort();
			importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<u_int16_t>( 0, 0, 0, dim4 ) );
			break;
		case data::TypePtr<int32_t>::staticID:
			importer->SetDataScalarTypeToInt();
			vtkImage->SetScalarTypeToInt();
			importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<int32_t>( 0, 0, 0, dim4 ) );
			break;
		case data::TypePtr<u_int32_t>::staticID:
			importer->SetDataScalarTypeToInt();
			vtkImage->SetScalarTypeToUnsignedInt();
			importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<u_int32_t>( 0, 0, 0, dim4 ) );
			break;
		case data::TypePtr<float>::staticID:
			importer->SetDataScalarTypeToFloat();
			vtkImage->SetScalarTypeToFloat();
			importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<float>( 0, 0, 0, dim4 ) );
			break;
		case data::TypePtr<double>::staticID:
			importer->SetDataScalarTypeToDouble();
			vtkImage->SetScalarTypeToDouble();
			importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<double>( 0, 0, 0, dim4 ) );
			break;
		}
	}

	importer->SetWholeExtent( 0, dimensions[0] - 1, 0, dimensions[1] - 1, 0, dimensions[2] - 1 );
	importer->SetDataExtentToWholeExtent();
	importer->Update();
	vtkImage = importer->GetOutput();
	vtkImage->SetOrigin( indexOrigin[0], indexOrigin[1], indexOrigin[2] );
	vtkImage->SetSpacing( spacing[0], spacing[1], spacing[2] );
	vtkImage->Update();
	return vtkImage;
}

}
} //end namespace
