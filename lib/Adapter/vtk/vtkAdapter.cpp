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

//return a list of vtkImageData type pointer
std::vector<vtkSmartPointer<vtkImageData> >vtkAdapter::makeVtkImageObject( const boost::shared_ptr<data::Image> src )
{
	const util::fvector4 indexOrigin( src->getProperty<util::fvector4>( "indexOrigin" ) );
	const util::fvector4 spacing( src->getProperty<util::fvector4>( "voxelSize" ) );
	std::vector<vtkSmartPointer<vtkImageData> > retVector;


	boost::shared_ptr<data::Image> typedPtr ( new data::TypedImage<u_int8_t>( *src ) );
	void *targePtr = malloc( typedPtr->bytes_per_voxel() * typedPtr->volume() );
	uint8_t *refTarget = ( uint8_t * ) targePtr;
	std::vector< boost::shared_ptr< data::Chunk> > chList = typedPtr->getChunkList();
	size_t chunkIndex = 0;

	BOOST_FOREACH( boost::shared_ptr< data::Chunk> & ref, chList ) {
		data::Chunk &chRef = *ref;
		u_int8_t *target = refTarget + chunkIndex++ * chRef.volume();
		chRef.getTypePtr<uint8_t>().copyToMem( 0, ( chRef.volume() - 1 ), target );
	}
	size_t imageVolume3D = typedPtr->sizeToVector()[0] * typedPtr->sizeToVector()[1] * typedPtr->sizeToVector()[2];
	for ( size_t t = 0; t<typedPtr->sizeToVector()[3]; t++ ) {
		vtkSmartPointer<vtkImageImport> importer = vtkImageImport::New();
		vtkSmartPointer<vtkImageData> vtkImage = vtkImageData::New();
		importer->SetDataScalarTypeToUnsignedChar();
		vtkImage->SetScalarTypeToUnsignedChar();
		importer->SetImportVoidPointer( refTarget + t * imageVolume3D );
		importer->SetWholeExtent( 0, typedPtr->sizeToVector()[0] - 1, 0, typedPtr->sizeToVector()[1] - 1, 0, typedPtr->sizeToVector()[2] - 1);
		importer->SetDataExtentToWholeExtent();
		importer->Update();
		vtkImage = importer->GetOutput();
		vtkImage->SetSpacing( spacing[0], spacing[1], spacing[2] );
		vtkImage->SetOrigin( indexOrigin[0], indexOrigin[1], indexOrigin[2] );
		vtkImage->Update();
		retVector.push_back(vtkImage);
	}
	return retVector;
}

}
} //end namespace
