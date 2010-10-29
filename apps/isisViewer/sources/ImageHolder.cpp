/****************************************************************
 *
 *  Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
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

#include "ImageHolder.hpp"

using namespace isis::viewer;

ImageHolder::ImageHolder()
{
	m_Image = vtkImageData::New();
	m_OrientedImage = vtkImageData::New();
	m_ExtractAxial = vtkImageClip::New();
	m_ExtractSagittal = vtkImageClip::New();
	m_ExtractCoronal = vtkImageClip::New();
	m_MapperAxial = vtkDataSetMapper::New();
	m_MapperSagittal = vtkDataSetMapper::New();
	m_MapperCoronal = vtkDataSetMapper::New();
	m_ActorAxial = vtkActor::New();
	m_ActorSagittal = vtkActor::New();
	m_ActorCoronal = vtkActor::New();
	m_CorrectedMatrix = vtkMatrix4x4::New();
	m_OriginalMatrix = vtkMatrix4x4::New();
	m_MatrixAxial = vtkMatrix4x4::New();
	m_MatrixCoronal = vtkMatrix4x4::New();
	m_MatrixSagittal = vtkMatrix4x4::New();
}

bool ImageHolder::resetSliceCoordinates( void )
{
	return setSliceCoordinates(m_OrientedImage->GetDimensions()[0] / 2, m_OrientedImage->GetDimensions()[1] / 2, m_OrientedImage->GetDimensions()[2] / 2);
}

bool ImageHolder::setSliceCoordinates( const int& x, const int& y, const int& z )
{
	std::vector<vtkImageClip*> extractorVec;
	std::vector<unsigned int> currentSliceVec;
	extractorVec.push_back(m_ExtractSagittal);
	extractorVec.push_back(m_ExtractCoronal);
	extractorVec.push_back(m_ExtractAxial);

	extractorVec[getBiggestVecElem<float>(m_readVec)]->SetOutputWholeExtent( x, x, 0, m_OrientedImage->GetDimensions()[1] - 1, 0, m_OrientedImage->GetDimensions()[2] - 1  );
	extractorVec[getBiggestVecElem<float>(m_readVec)]->Update();
	extractorVec[getBiggestVecElem<float>(m_phaseVec)]->SetOutputWholeExtent( 0, m_OrientedImage->GetDimensions()[0] - 1, y, y, 0, m_OrientedImage->GetDimensions()[2] - 1 );
	extractorVec[getBiggestVecElem<float>(m_phaseVec)]->Update();
	extractorVec[getBiggestVecElem<float>(m_sliceVec)]->SetOutputWholeExtent( 0, m_OrientedImage->GetDimensions()[0] - 1, 0, m_OrientedImage->GetDimensions()[1] - 1, z, z );
	extractorVec[getBiggestVecElem<float>(m_sliceVec)]->Update();
	return true;
}

void ImageHolder::setUpPipe()
{
	//axial
	m_ExtractAxial->SetInput( m_OrientedImage );
	m_MapperAxial->SetInput( m_ExtractAxial->GetOutput() );
	m_ActorAxial->SetMapper( m_MapperAxial );
	m_ActorAxial->GetProperty()->SetInterpolationToFlat();
	m_ActorAxial->SetScale( m_OrientedImage->GetSpacing()[0], m_OrientedImage->GetSpacing()[1], m_OrientedImage->GetSpacing()[2] );
	m_ActorAxial->SetUserMatrix( m_MatrixAxial );

	//sagittal
	m_ExtractSagittal->SetInput( m_OrientedImage );
	m_MapperSagittal->SetInput( m_ExtractSagittal->GetOutput() );
	m_ActorSagittal->SetMapper( m_MapperSagittal );
	m_ActorSagittal->GetProperty()->SetInterpolationToFlat();
	m_ActorSagittal->SetScale( m_OrientedImage->GetSpacing()[0], m_OrientedImage->GetSpacing()[1], m_OrientedImage->GetSpacing()[2] );
	m_ActorSagittal->SetUserMatrix(m_MatrixSagittal);

	//coronal
	m_ExtractCoronal->SetInput( m_OrientedImage );
	m_MapperCoronal->SetInput( m_ExtractCoronal->GetOutput() );
	m_ActorCoronal->SetMapper( m_MapperCoronal );
	m_ActorCoronal->GetProperty()->SetInterpolationToFlat();
	m_ActorCoronal->SetScale( m_OrientedImage->GetSpacing()[0], m_OrientedImage->GetSpacing()[1], m_OrientedImage->GetSpacing()[2] );
	m_ActorCoronal->SetUserMatrix( m_MatrixCoronal );
}

void ImageHolder::setImages( boost::shared_ptr<isis::data::Image> isisImg,  vtkImageData* img )
{
	m_Image = img;
	m_ISISImage = isisImg;
	isis::util::TypeReference min, max;
	m_ISISImage->getMinMax( min, max );
	m_Min = min->as<double>();
	m_Max = max->as<double>();
	m_readVec = m_ISISImage->getProperty<isis::util::fvector4>("readVec");
	m_phaseVec = m_ISISImage->getProperty<isis::util::fvector4>("phaseVec");
	m_sliceVec = m_ISISImage->getProperty<isis::util::fvector4>("sliceVec");
	createOrientedImage();
	initMatrices();
	resetSliceCoordinates();
	setUpPipe();

}

bool ImageHolder::createOrientedImage( void )
{
	for ( size_t i = 0; i<3; i++ ) {
		m_OriginalMatrix->SetElement(i,0, m_readVec[i] < 0 ? ceil(m_readVec[i]-0.5) : floor(m_readVec[i]+0.5));
	}
	for ( size_t i = 0; i<3; i++ ) {
		m_OriginalMatrix->SetElement(i,1,m_phaseVec[i] < 0 ? ceil(m_phaseVec[i]-0.5) : floor(m_phaseVec[i]+0.5));
	}
	for ( size_t i = 0; i<3; i++ ) {
		m_OriginalMatrix->SetElement(i,2, m_sliceVec[i] < 0 ? ceil(m_sliceVec[i]-0.5) : floor(m_sliceVec[i]+0.5));
	}
	m_OriginalMatrix->SetElement(3,3,1);
	//TODO debug
	m_CorrectedMatrix = m_OriginalMatrix;
	m_OrientedImage = m_Image;

	if( correctMatrix(m_CorrectedMatrix) ) {
		vtkSmartPointer<vtkImageFlip> flipper = vtkImageFlip::New();
		flipper->SetFilteredAxis(2);
		flipper->SetInput(m_Image);
		flipper->Update();
		m_OrientedImage = flipper->GetOutput();
	}

	return true;
}

bool ImageHolder::correctMatrix( vtkSmartPointer<vtkMatrix4x4> matrix )
{
	matrix->Print(std::cout);
	if (matrix->Determinant() != 1 ) {
		std::cout << "correct" << std::endl;
		const isis::util::fvector4 crossVec = isis::util::fvector4( //we could use their cross-product as sliceVector
			matrix->GetElement(1, 0)  * matrix->GetElement(2, 1) - matrix->GetElement(2, 0) * matrix->GetElement(1,1),
			matrix->GetElement(2, 0) * matrix->GetElement(0, 1) - matrix->GetElement(0, 0) * matrix->GetElement(2,1),
			matrix->GetElement(0, 0) * matrix->GetElement(1, 1) - matrix->GetElement(1, 0) * matrix->GetElement(0, 1)
		);
		matrix->SetElement(0,2,crossVec[0]);
		matrix->SetElement(1,2,crossVec[1]);
		matrix->SetElement(2,2,crossVec[2]);
		return true;
	} else {
		return false;
	}
}

void ImageHolder::initMatrices( void ) {

	vtkSmartPointer<vtkMatrix4x4> axialMatrix = vtkMatrix4x4::New();
	vtkSmartPointer<vtkMatrix4x4> sagittalMatrix = vtkMatrix4x4::New();
	vtkSmartPointer<vtkMatrix4x4> coronalMatrix= vtkMatrix4x4::New();
	/*setup axial matrix
	*-1  0  0
	* 0 -1  0
	* 0  0  1
	*/
	axialMatrix->SetElement(0,0,-1);
	axialMatrix->SetElement(1,1,-1);
	/*setup sagittal matrix
	 * 0  1  0
	 * 0  0  1
	 * 1  0  0
	 */
	sagittalMatrix->SetElement(0,0,0);
	sagittalMatrix->SetElement(2,0,1);
	sagittalMatrix->SetElement(0,1,1);
	sagittalMatrix->SetElement(2,2,0);
	sagittalMatrix->SetElement(1,2,1);
	sagittalMatrix->SetElement(1,1,0);
	/*setup coronal matrix
	 * -1  0  0
	 *  0  0  1
	 *  0  1  0
	 */
	coronalMatrix->SetElement(0,0,-1);
	coronalMatrix->SetElement(1,1,0);
	coronalMatrix->SetElement(2,2,0);
	coronalMatrix->SetElement(2,1,1);
	coronalMatrix->SetElement(1,2,1);
	//now we have to multiply these view specific matrices with the corrected
	//orientation matrix to assure the determinant=1
	vtkMatrix4x4::Multiply4x4(axialMatrix, m_CorrectedMatrix, m_MatrixAxial);
	vtkMatrix4x4::Multiply4x4(sagittalMatrix, m_CorrectedMatrix, m_MatrixSagittal);
	vtkMatrix4x4::Multiply4x4(coronalMatrix, m_CorrectedMatrix, m_MatrixCoronal);

}


