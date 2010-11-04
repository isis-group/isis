/*
 * MatrixHandler.cpp
 *
 *  Created on: Nov 1, 2010
 *      Author: tuerke
 */

#include "MatrixHandler.hpp"

namespace isis {

namespace viewer {

MatrixHandler::MatrixHandler( void )
{
	m_origMatrix = vtkMatrix4x4::New();
	m_origMatrix1 = vtkMatrix4x4::New();
	m_correctedMatrix = vtkMatrix4x4::New();
	m_correctedMatrix1 = vtkMatrix4x4::New();
	m_MatrixAxial = vtkMatrix4x4::New();
	m_MatrixAxial1 = vtkMatrix4x4::New();
	m_MatrixSagittal = vtkMatrix4x4::New();
	m_MatrixSagittal1= vtkMatrix4x4::New();
	m_MatrixCoronal = vtkMatrix4x4::New();
	m_MatrixCoronal1 = vtkMatrix4x4::New();
}

void MatrixHandler::setVectors( isis::util::fvector4 readVec, isis::util::fvector4 phaseVec, isis::util::fvector4 sliceVec )
{
	m_readVec = readVec;
	m_phaseVec = phaseVec;
	m_sliceVec = sliceVec;
	for ( size_t i = 0; i<3; i++ ) {
		m_origMatrix->SetElement(i,0, m_readVec[i] );
		m_origMatrix1->SetElement(i,0, m_readVec[i] < 0 ? ceil(m_readVec[i]-0.5) : floor(m_readVec[i]+0.5));
	}
	for ( size_t i = 0; i<3; i++ ) {
		m_origMatrix->SetElement(i,1, m_phaseVec[i] );
		m_origMatrix1->SetElement(i,1,m_phaseVec[i] < 0 ? ceil(m_phaseVec[i]-0.5) : floor(m_phaseVec[i]+0.5));
	}
	for ( size_t i = 0; i<3; i++ ) {
		m_origMatrix->SetElement(i,2, m_sliceVec[i] );
		m_origMatrix1->SetElement(i,2, m_sliceVec[i] < 0 ? ceil(m_sliceVec[i]-0.5) : floor(m_sliceVec[i]+0.5));
	}
	m_origMatrix->SetElement(3,3,1);
	m_origMatrix1->SetElement(3,3,1);
	m_correctedMatrix = m_origMatrix;
	m_correctedMatrix1 = m_origMatrix1;
	m_isRotationMatrix = correctMatrix();
	createMatricesForWidgets();
}

bool MatrixHandler::correctMatrix( void )
{
	if (m_origMatrix1->Determinant() != 1 ) {
		const isis::util::fvector4 crossVec1 = isis::util::fvector4( //we could use their cross-product as sliceVector
			m_origMatrix1->GetElement(1, 0)  * m_origMatrix1->GetElement(2, 1) - m_origMatrix1->GetElement(2, 0) * m_origMatrix1->GetElement(1,1),
			m_origMatrix1->GetElement(2, 0) * m_origMatrix1->GetElement(0, 1) - m_origMatrix1->GetElement(0, 0) * m_origMatrix1->GetElement(2,1),
			m_origMatrix1->GetElement(0, 0) * m_origMatrix1->GetElement(1, 1) - m_origMatrix1->GetElement(1, 0) * m_origMatrix1->GetElement(0, 1)
		);
		const isis::util::fvector4 crossVec = isis::util::fvector4( //we could use their cross-product as sliceVector
			m_origMatrix->GetElement(1, 0)  * m_origMatrix->GetElement(2, 1) - m_origMatrix->GetElement(2, 0) * m_origMatrix1->GetElement(1,1),
			m_origMatrix->GetElement(2, 0) * m_origMatrix->GetElement(0, 1) - m_origMatrix->GetElement(0, 0) * m_origMatrix1->GetElement(2,1),
			m_origMatrix->GetElement(0, 0) * m_origMatrix->GetElement(1, 1) - m_origMatrix->GetElement(1, 0) * m_origMatrix1->GetElement(0, 1)
		);
		m_correctedMatrix->SetElement(0,2,crossVec[0]);
		m_correctedMatrix->SetElement(1,2,crossVec[1]);
		m_correctedMatrix->SetElement(2,2,crossVec[2]);
		m_correctedMatrix1->SetElement(0,2,crossVec1[0]);
		m_correctedMatrix1->SetElement(1,2,crossVec1[1]);
		m_correctedMatrix1->SetElement(2,2,crossVec1[2]);
		return true;
	} else {
		return false;
	}
}

void MatrixHandler::createMatricesForWidgets( void )
{
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
	vtkMatrix4x4::Multiply4x4(axialMatrix, m_correctedMatrix1, m_MatrixAxial1);
	vtkMatrix4x4::Multiply4x4(sagittalMatrix, m_correctedMatrix1, m_MatrixSagittal1);
	vtkMatrix4x4::Multiply4x4(coronalMatrix, m_correctedMatrix1, m_MatrixCoronal1);
	vtkMatrix4x4::Multiply4x4(axialMatrix, m_correctedMatrix, m_MatrixAxial);
	vtkMatrix4x4::Multiply4x4(sagittalMatrix, m_correctedMatrix, m_MatrixSagittal);
	vtkMatrix4x4::Multiply4x4(coronalMatrix, m_correctedMatrix, m_MatrixCoronal);
}

}}
