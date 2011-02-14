/*
 * MatrixHandler.hpp
 *
 *  Created on: Nov 1, 2010
 *      Author: tuerke
 */

#ifndef MATRIXHANDLER_HPP_
#define MATRIXHANDLER_HPP_

#include "CoreUtils/vector.hpp"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "common.hpp"

namespace isis
{

namespace viewer
{

class MatrixHandler
{
public:
	typedef boost::numeric::ublas::matrix<float> MatrixType;
	MatrixHandler();
	void setVectors ( isis::util::fvector4 readVec, isis::util::fvector4 phaseVec, isis::util::fvector4 sliceVec );

	MatrixType getAxialMatrix( void ) const { return m_MatrixAxial; }
	MatrixType getSagittalMatrix( void ) const { return m_MatrixSagittal; }
	MatrixType getCoronalMatrix( void ) const { return m_MatrixCoronal; }

	MatrixType getAxialMatrix1( void ) const { return m_MatrixAxial1; }
	MatrixType getSagittalMatrix1( void ) const { return m_MatrixSagittal1; }
	MatrixType getCoronalMatrix1( void ) const { return m_MatrixCoronal1; }

	bool isRotationMatrix( void ) const { return m_isRotationMatrix; }
	util::fvector4 createPseudoOrigin( const util::fvector4 &size, const util::fvector4 &voxelSize ) const;
	util::fvector4 transformOrigin( const util::fvector4 &origin, const util::fvector4 &voxelSize ) const;

private:
	bool m_isRotationMatrix;
	bool m_Valid;
	isis::util::fvector4 m_readVec;
	isis::util::fvector4 m_phaseVec;
	isis::util::fvector4 m_sliceVec;
	MatrixType m_origMatrix;
	MatrixType m_origMatrix1;
	MatrixType m_correctedMatrix;
	MatrixType m_correctedMatrix1;
	// matrices associated with the widgets
	MatrixType m_MatrixAxial;
	MatrixType m_MatrixSagittal;
	MatrixType m_MatrixCoronal;
	MatrixType m_MatrixAxial1;
	MatrixType m_MatrixSagittal1;
	MatrixType m_MatrixCoronal1;
	bool correctMatrix( void );
	void createMatricesForWidgets( void );
	double determinant( MatrixType& );
	int determinant_sign(const boost::numeric::ublas::permutation_matrix<size_t>& );
};

}
}
#endif /* MATRIXHANDLER_HPP_ */
