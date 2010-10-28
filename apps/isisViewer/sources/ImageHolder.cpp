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

//rotation values for visualization in image space
const double ImageHolder::orientSagittal[] = {-0,-90,270};
const double ImageHolder::orientAxial[] = {0,0,180};
const double ImageHolder::orientCoronal[] = {90,0,180};

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
	m_Matrix = vtkMatrix4x4::New();
	m_OriginalMatrix = vtkMatrix4x4::New();

}

bool ImageHolder::resetSliceCoordinates( void )
{
	return setSliceCoordinates(m_OrientedImage->GetDimensions()[0] / 2, m_OrientedImage->GetDimensions()[1] / 2, m_OrientedImage->GetDimensions()[2] / 2);
}

bool ImageHolder::setSliceCoordinates( const int& x, const int& y, const int& z )
{
	std::vector<vtkImageClip*> extractorVec;
	std::vector<unsigned int> currentSliceVec;
	extractorVec.push_back(m_ExtractAxial);
	extractorVec.push_back(m_ExtractCoronal);
	extractorVec.push_back(m_ExtractSagittal);

	extractorVec[getBiggestVecElem<float>(m_transposedReadVec)]->SetOutputWholeExtent( 0, m_OrientedImage->GetDimensions()[0] - 1, 0, m_OrientedImage->GetDimensions()[1] - 1, z, z );

	extractorVec[getBiggestVecElem<float>(m_transposedPhaseVec)]->SetOutputWholeExtent( 0, m_OrientedImage->GetDimensions()[0] - 1, y, y, 0, m_OrientedImage->GetDimensions()[2] - 1 );

	extractorVec[getBiggestVecElem<float>(m_transposedSliceVec)]->SetOutputWholeExtent( x, x, 0, m_OrientedImage->GetDimensions()[1] - 1, 0, m_OrientedImage->GetDimensions()[2] - 1  );

	m_ExtractAxial->Update();
	m_ExtractSagittal->Update();
	m_ExtractCoronal->Update();

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
	m_ActorAxial->SetOrientation( orientAxial[0] + m_RotationVector[0][0], orientAxial[1] + m_RotationVector[0][1], orientAxial[2] + m_RotationVector[0][2] );

	//sagittal
	m_ExtractSagittal->SetInput( m_OrientedImage );
	m_MapperSagittal->SetInput( m_ExtractSagittal->GetOutput() );
	m_ActorSagittal->SetMapper( m_MapperSagittal );
	m_ActorSagittal->GetProperty()->SetInterpolationToFlat();
	m_ActorSagittal->SetScale( m_OrientedImage->GetSpacing()[0], m_OrientedImage->GetSpacing()[1], m_OrientedImage->GetSpacing()[2] );
	m_ActorSagittal->SetOrientation( orientSagittal[0] + m_RotationVector[1][0], orientSagittal[1] + m_RotationVector[1][1], orientSagittal[2]+ m_RotationVector[1][2] );


	//coronal
	m_ExtractCoronal->SetInput( m_OrientedImage );
	m_MapperCoronal->SetInput( m_ExtractCoronal->GetOutput() );
	m_ActorCoronal->SetMapper( m_MapperCoronal );
	m_ActorCoronal->GetProperty()->SetInterpolationToFlat();
	m_ActorCoronal->SetScale( m_OrientedImage->GetSpacing()[0], m_OrientedImage->GetSpacing()[1], m_OrientedImage->GetSpacing()[2] );
	m_ActorCoronal->SetOrientation( orientCoronal[0] + m_RotationVector[2][0], orientCoronal[1] + m_RotationVector[2][1], orientCoronal[2] + m_RotationVector[2][2] );

//	m_ActorAxial->AddOrientation(m_RotX, m_RotY, m_RotZ);
//	m_ActorSagittal->AddOrientation(m_RotX, m_RotY, m_RotZ);
//	m_ActorCoronal->AddOrientation(m_RotX, m_RotY, m_RotZ);

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
	m_transposedReadVec = isis::util::fvector4( m_readVec[0], m_phaseVec[0], m_sliceVec[0], 0);
	m_transposedPhaseVec = isis::util::fvector4( m_readVec[1], m_phaseVec[1], m_sliceVec[1], 0);
	m_transposedSliceVec = isis::util::fvector4( m_readVec[2], m_phaseVec[2], m_sliceVec[2], 0);
	createOrientedImage();
	resetSliceCoordinates();
	setUpPipe();

}

bool ImageHolder::createOrientedImage( void )
{
	for ( size_t i = 0; i<3; i++ ) {
		m_Matrix->SetElement(i,0, floor(fabs(m_readVec[i])+0.5));
		m_OriginalMatrix->SetElement(i,0, m_readVec[i] < 0 ? ceil(m_readVec[i]-0.5) : floor(m_readVec[i]+0.5));
	}
	for ( size_t i = 0; i<3; i++ ) {
		m_Matrix->SetElement(i,1, floor(fabs(m_phaseVec[i])+0.5));
		m_OriginalMatrix->SetElement(i,1,m_phaseVec[i] < 0 ? ceil(m_phaseVec[i]-0.5) : floor(m_phaseVec[i]+0.5));
	}
	for ( size_t i = 0; i<3; i++ ) {
		m_Matrix->SetElement(i,2, floor(fabs(m_sliceVec[i])+0.5));
		m_OriginalMatrix->SetElement(i,2, m_sliceVec[i] < 0 ? ceil(m_sliceVec[i]-0.5) : floor(m_sliceVec[i]+0.5));
	}

	m_Matrix->SetElement(3,3,1);
	m_OriginalMatrix->SetElement(3,3,1);
	//TODO debug
	m_Matrix->Print(std::cout);
	m_OriginalMatrix->Print(std::cout);

	createOrientationFromMatrix(m_Matrix);

	isis::util::fvector4 readVec = m_ISISImage->getProperty<isis::util::fvector4>("readVec");
	isis::util::fvector4 phaseVec = m_ISISImage->getProperty<isis::util::fvector4>("phaseVec");
	isis::util::fvector4 sliceVec = m_ISISImage->getProperty<isis::util::fvector4>("sliceVec");
	vtkImageData* phaseImage = vtkImageData::New();
	vtkImageData* sliceImage = vtkImageData::New();

	if ( readVec[getBiggestVecElem(readVec)] < 0 ) {
		vtkImageFlip* flipper = vtkImageFlip::New();
		flipper->SetFilteredAxis(0);
		flipper->SetInput(m_Image);
		flipper->Update();
		phaseImage = flipper->GetOutput();
	} else {
		phaseImage = m_Image;
	}
	if ( phaseVec[getBiggestVecElem(phaseVec)] < 0 ) {
		vtkImageFlip* flipper = vtkImageFlip::New();
		flipper->SetFilteredAxis(1);
		flipper->SetInput(phaseImage);
		flipper->Update();
		sliceImage = flipper->GetOutput();
	} else {
		sliceImage = phaseImage;
	}
		if ( sliceVec[getBiggestVecElem(sliceVec)] < 0 ) {
		vtkImageFlip* flipper = vtkImageFlip::New();
		flipper->SetFilteredAxis(2);
		flipper->SetInput(sliceImage);
		flipper->Update();
		m_OrientedImage = flipper->GetOutput();
	} else {
		m_OrientedImage = sliceImage;
	}

	return true;
}

void ImageHolder::createOrientationFromMatrix( vtkMatrix4x4* matrix )
{
	std::vector<std::vector<double> > retVec;

	std::vector<double> axial;
	std::vector<double> coronal;
	std::vector<double> sagittal;

	double rotY = atan2(matrix->GetElement(2,0), sqrt((matrix->GetElement(0,0)*matrix->GetElement(0,0))+(matrix->GetElement(1,0)*matrix->GetElement(1,0)))) * (180 / M_PI);
	double rotZ = atan2((matrix->GetElement(1,0) / cos(rotY)), matrix->GetElement(0,0) / cos(rotY)) * (180 / M_PI);
	double rotX = atan2((matrix->GetElement(2,1) / cos(rotY)), matrix->GetElement(2,2) / cos(rotY)) * (180 / M_PI);

	axial.push_back(rotX);
	axial.push_back(rotY);
	axial.push_back(rotZ);

	sagittal.push_back(rotY);
	sagittal.push_back(rotZ);
	sagittal.push_back(rotX);

	coronal.push_back(rotZ);
	coronal.push_back(rotX);
	coronal.push_back(rotY);

	std::cout << "x: " << rotX << std::endl;
	std::cout << "y: " << rotY << std::endl;
	std::cout << "z: " << rotZ << std::endl;

	retVec.push_back(axial);
	retVec.push_back(sagittal);
	retVec.push_back(coronal);

	m_RotationVector = retVec;


}

