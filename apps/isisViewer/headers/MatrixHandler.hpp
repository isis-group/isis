/*
 * MatrixHandler.hpp
 *
 *  Created on: Nov 1, 2010
 *      Author: tuerke
 */

#ifndef MATRIXHANDLER_HPP_
#define MATRIXHANDLER_HPP_

#include "CoreUtils/vector.hpp"
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
#include "common.hpp"

namespace isis
{

namespace viewer
{

class MatrixHandler
{
public:
	MatrixHandler();
	void setVectors ( isis::util::fvector4 readVec, isis::util::fvector4 phaseVec, isis::util::fvector4 sliceVec );

	vtkSmartPointer<vtkMatrix4x4> getAxialMatrix( void ) const { return m_MatrixAxial; }
	vtkSmartPointer<vtkMatrix4x4> getSagittalMatrix( void ) const { return m_MatrixSagittal; }
	vtkSmartPointer<vtkMatrix4x4> getCoronalMatrix( void ) const { return m_MatrixCoronal; }

	vtkSmartPointer<vtkMatrix4x4> getAxialMatrix1( void ) const { return m_MatrixAxial1; }
	vtkSmartPointer<vtkMatrix4x4> getSagittalMatrix1( void ) const { return m_MatrixSagittal1; }
	vtkSmartPointer<vtkMatrix4x4> getCoronalMatrix1( void ) const { return m_MatrixCoronal1; }

	bool isRotationMatrix( void ) const { return m_isRotationMatrix; }
	util::fvector4 createPseudoOrigin( const util::fvector4 &size, const util::fvector4 &voxelSize ) const;
	util::fvector4 transformOrigin( const util::fvector4 &origin, const util::fvector4 &voxelSize ) const;

private:
	bool m_isRotationMatrix;
	bool m_Valid;
	isis::util::fvector4 m_readVec;
	isis::util::fvector4 m_phaseVec;
	isis::util::fvector4 m_sliceVec;
	vtkSmartPointer<vtkMatrix4x4> m_origMatrix;
	vtkSmartPointer<vtkMatrix4x4> m_origMatrix1;
	vtkSmartPointer<vtkMatrix4x4> m_correctedMatrix;
	vtkSmartPointer<vtkMatrix4x4> m_correctedMatrix1;
	// matrices associated with the widgets
	vtkSmartPointer<vtkMatrix4x4> m_MatrixAxial;
	vtkSmartPointer<vtkMatrix4x4> m_MatrixSagittal;
	vtkSmartPointer<vtkMatrix4x4> m_MatrixCoronal;
	vtkSmartPointer<vtkMatrix4x4> m_MatrixAxial1;
	vtkSmartPointer<vtkMatrix4x4> m_MatrixSagittal1;
	vtkSmartPointer<vtkMatrix4x4> m_MatrixCoronal1;
	bool correctMatrix( void );
	void createMatricesForWidgets( void );
};

}
}
#endif /* MATRIXHANDLER_HPP_ */
