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
vtkImageData* vtkAdapter::makeVtkImageObject( const boost::shared_ptr<data::Image> src, unsigned int dim4 )
{
	vtkAdapter* myAdapter = new vtkAdapter( src );
	vtkImageData* vtkImage = vtkImageData::New();
	vtkImageImport* importer = vtkImageImport::New();
	const util::fvector4 dimensions( myAdapter->m_ImageISIS->sizeToVector() );
	const util::fvector4 indexOrigin( myAdapter->m_ImageISIS->getProperty<util::fvector4>( "indexOrigin" ) );
	const util::fvector4 spacing( myAdapter->m_ImageISIS->getProperty<util::fvector4>( "voxelSize" ) );

	if ( dim4 > dimensions[3] - 1 ) dim4 = dimensions[3] - 1;

	//set the datatype for the vtkImage object
	//TODO check datatypes
	
	vtkImage->SetOrigin( indexOrigin[0], indexOrigin[1], indexOrigin[2] );
	vtkImage->SetSpacing( spacing[0], spacing[1], spacing[2] );
	//go through every timestep (dimensions[3] and add the chunk to the image list

	switch ( myAdapter->m_ImageISIS->chunksBegin()->typeID() ) {
	case util::TypePtr<int8_t>::staticID:
		importer->SetDataScalarTypeToUnsignedChar();
		importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<int8_t>( 0, 0, 0, dim4 ) );
		vtkImage->SetScalarTypeToChar();
		break;
	case util::TypePtr<u_int8_t>::staticID:
		importer->SetDataScalarTypeToUnsignedChar();
		vtkImage->SetScalarTypeToUnsignedChar();
		importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<u_int8_t>( 0, 0, 0, dim4 ) );
		break;
	case util::TypePtr<int16_t>::staticID:
		importer->SetDataScalarTypeToShort();
		vtkImage->SetScalarTypeToShort();
		importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<int16_t>( 0, 0, 0, dim4 ) );
		break;
	case util::TypePtr<u_int16_t>::staticID:
		importer->SetDataScalarTypeToUnsignedShort();
		vtkImage->SetScalarTypeToUnsignedShort();
		importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<u_int16_t>( 0, 0, 0, dim4 ) );
		break;
	case util::TypePtr<int32_t>::staticID:
		importer->SetDataScalarTypeToInt();
		vtkImage->SetScalarTypeToInt();
		importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<int32_t>( 0, 0, 0, dim4 ) );
		break;
	case util::TypePtr<u_int32_t>::staticID:
		importer->SetDataScalarTypeToInt();
		vtkImage->SetScalarTypeToUnsignedInt();
		importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<u_int32_t>( 0, 0, 0, dim4 ) );
		break;
	case util::TypePtr<float>::staticID:
		importer->SetDataScalarTypeToFloat();
		vtkImage->SetScalarTypeToFloat();
		importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<float>( 0, 0, 0, dim4 ) );
		break;
	case util::TypePtr<double>::staticID:
		importer->SetDataScalarTypeToDouble();
		vtkImage->SetScalarTypeToDouble();
		importer->SetImportVoidPointer( &myAdapter->m_ImageISIS->voxel<double>( 0, 0, 0, dim4 ) );
		break;
	}

	importer->Update();
	vtkImage = importer->GetOutput();
	importer->SetWholeExtent( 0, dimensions[0] - 1, 0, dimensions[1] - 1, 0, dimensions[2] - 1 ); //TODO what is exactly defined by the whole extend????????????
	importer->SetDataExtentToWholeExtent();
	return vtkImage;
}

}
} //end namespace
