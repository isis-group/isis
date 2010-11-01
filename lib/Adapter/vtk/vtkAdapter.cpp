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
	m_ImageISIS = src;
	const util::fvector4 dimensions( m_ImageISIS->sizeToVector() );
	const util::fvector4 indexOrigin( m_ImageISIS->getProperty<util::fvector4>( "indexOrigin" ) );
	const util::fvector4 spacing( m_ImageISIS->getProperty<util::fvector4>( "voxelSize" ) );

	std::vector<vtkSmartPointer<vtkImageData> >retVector;
	boost::shared_ptr<data::Image> imgPtr ( new data::TypedImage<u_int8_t>( *src ) );

	for (size_t t=0; t<m_ImageISIS->sizeToVector()[3]; t++ ) {
	vtkSmartPointer<vtkImageData> tmpImage = vtkImageData::New();
		for (size_t z=0; z<m_ImageISIS->sizeToVector()[2]; z++ ) {
			for (size_t y=0; y<m_ImageISIS->sizeToVector()[1]; y++ ) {
				for (size_t x=0; x<m_ImageISIS->sizeToVector()[0]; x++ ) {
					u_int8_t* voxel = static_cast<u_int8_t*>(tmpImage->GetScalarPointer(x,y,z));
					voxel[0] = m_ImageISIS->voxel<u_int8_t>(x,y,z,t);
				}
			}
		}
		tmpImage->SetOrigin( indexOrigin[0], indexOrigin[1], indexOrigin[2] );
		tmpImage->SetSpacing( spacing[0], spacing[1], spacing[2] );
		tmpImage->Update();
		retVector.push_back(tmpImage);

	}







	//set the datatype for the vtkImage object
	//TODO check datatypes

//	void *targePtr = malloc( m_ImageISIS->bytes_per_voxel() * myAdapter->m_ImageISIS->volume() );
//	uint8_t *refTarget = ( uint8_t * ) targePtr;
//	std::vector< boost::shared_ptr< data::Chunk> > chList = myAdapter->m_ImageISIS->getChunkList();
//	size_t chunkIndex = 0;
//	BOOST_FOREACH( boost::shared_ptr< data::Chunk> & ref, chList ) {
//		data::Chunk &chRef = *ref;
//		uint8_t *target = refTarget + chunkIndex++ * chRef.volume();
//		chRef.getTypePtr<uint8_t>().copyToMem( 0, ( chRef.volume() - 1 ), target );
//	}



	return retVector;
}

}
} //end namespace
