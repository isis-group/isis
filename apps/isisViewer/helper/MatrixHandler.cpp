/*
 * MatrixHandler.cpp
 *
 *  Created on: Nov 1, 2010
 *      Author: tuerke
 */

#include "MatrixHandler.hpp"

namespace isis
{
namespace viewer
{
namespace _internal
{

MatrixHandler::MatrixHandler( void )
	: m_Valid( false )
{}

void MatrixHandler::setVectors( isis::util::fvector4 readVec, isis::util::fvector4 phaseVec, isis::util::fvector4 sliceVec )
{
	m_Valid = true;
	m_readVec = readVec;
	m_phaseVec = phaseVec;
	m_sliceVec = sliceVec;

	for ( size_t i = 0; i < 3; i++ ) {
		m_origMatrix( i, 0 ) = m_readVec[i];
		m_origMatrix1( i, 0 ) = m_readVec[i] < 0 ? ceil( m_readVec[i] - 0.5 ) : floor( m_readVec[i] + 0.5 );
	}

	for ( size_t i = 0; i < 3; i++ ) {
		m_origMatrix( i, 1 ) = m_phaseVec[i];
		m_origMatrix1( i, 1 ) = m_phaseVec[i] < 0 ? ceil( m_phaseVec[i] - 0.5 ) : floor( m_phaseVec[i] + 0.5 );
	}

	for ( size_t i = 0; i < 3; i++ ) {
		m_origMatrix( i, 2 ) = m_sliceVec[i];
		m_origMatrix1( i, 2 ) = m_sliceVec[i] < 0 ? ceil( m_sliceVec[i] - 0.5 ) : floor( m_sliceVec[i] + 0.5 );
	}

	m_correctedMatrix = m_origMatrix;
	m_correctedMatrix1 = m_origMatrix1;
	m_isRotationMatrix = correctMatrix();
	createMatricesForWidgets();
}

bool MatrixHandler::correctMatrix( void )
{
	if ( determinant( m_origMatrix1 ) == 1.0 ) {
		const isis::util::fvector4 crossVec1 = isis::util::fvector4( //we could use their cross-product as sliceVector
				m_origMatrix1( 1, 0 )  * m_origMatrix1( 2, 1 ) - m_origMatrix1( 2, 0 ) * m_origMatrix1( 1, 1 ),
				m_origMatrix1( 2, 0 ) * m_origMatrix1( 0, 1 ) - m_origMatrix1( 0, 0 ) * m_origMatrix1( 2, 1 ),
				m_origMatrix1( 0, 0 ) * m_origMatrix1( 1, 1 ) - m_origMatrix1( 1, 0 ) * m_origMatrix1( 0, 1 )
											   );
		const isis::util::fvector4 crossVec = isis::util::fvector4( //we could use their cross-product as sliceVector
				m_origMatrix( 1, 0 )  * m_origMatrix( 2, 1 ) - m_origMatrix( 2, 0 ) * m_origMatrix1( 1, 1 ),
				m_origMatrix( 2, 0 ) * m_origMatrix( 0, 1 ) - m_origMatrix( 0, 0 ) * m_origMatrix1( 2, 1 ),
				m_origMatrix( 0, 0 ) * m_origMatrix( 1, 1 ) - m_origMatrix( 1, 0 ) * m_origMatrix1( 0, 1 )
											  );
		m_correctedMatrix( 0, 2 ) = crossVec[0];
		m_correctedMatrix( 1, 2 ) = crossVec[1];
		m_correctedMatrix( 2, 2 ) = crossVec[2];
		m_correctedMatrix1( 0, 2 ) = crossVec1[0];
		m_correctedMatrix1( 1, 2 ) = crossVec1[1];
		m_correctedMatrix1( 2, 2 ) = crossVec1[2];
		return true;
	} else {
		return false;
	}
}

void MatrixHandler::createMatricesForWidgets( void )
{
	MatrixType axialMatrix ( 3, 3 );
	MatrixType sagittalMatrix ( 3, 3 );
	MatrixType coronalMatrix ( 3, 3 );
	/*setup axial matrix
	*-1  0  0
	* 0 -1  0
	* 0  0  1
	*/
	axialMatrix( 0, 0 ) = -1;
	axialMatrix( 1, 1 ) = -1;
	/*setup sagittal matrix
	 * 0  1  0
	 * 0  0  1
	 * 1  0  0
	 */
	sagittalMatrix( 0, 0 ) = 0;
	sagittalMatrix( 2, 0 ) = 1;
	sagittalMatrix( 0, 1 ) = 1;
	sagittalMatrix( 2, 2 ) = 0;
	sagittalMatrix( 1, 2 ) = 1;
	sagittalMatrix( 1, 1 ) = 0;
	/*setup coronal matrix
	 * -1  0  0
	 *  0  0  1
	 *  0  1  0
	 */
	coronalMatrix( 0, 0 ) = -1;
	coronalMatrix( 1, 1 ) = 0;
	coronalMatrix( 2, 2 ) = 0;
	coronalMatrix( 2, 1 ) = 1;
	coronalMatrix( 1, 2 ) = 1;
	//now we have to multiply these view specific matrices with the corrected
	//orientation matrix to assure the determinant=1
	m_MatrixAxial1 = boost::numeric::ublas::prod( axialMatrix, m_correctedMatrix1 );
	m_MatrixSagittal1 = boost::numeric::ublas::prod( sagittalMatrix, m_correctedMatrix1 );
	m_MatrixCoronal1 = boost::numeric::ublas::prod( coronalMatrix, m_correctedMatrix1 );
	m_MatrixAxial = boost::numeric::ublas::prod( axialMatrix, m_correctedMatrix );
	m_MatrixSagittal = boost::numeric::ublas::prod( sagittalMatrix, m_correctedMatrix );
	m_MatrixCoronal = boost::numeric::ublas::prod( coronalMatrix, m_correctedMatrix );
}

util::fvector4 MatrixHandler::createPseudoOrigin( const util::fvector4 &size, const util::fvector4 &voxelSize ) const
{
	if ( !m_Valid ) {
		LOG( Runtime, error ) << "Cannot create pseudo origin. First call setVectors.";
		return util::fvector4( 0, 0, 0, 0 );
	} else {
		return util::fvector4( -size[0] * voxelSize[0] / 2,
							   -size[1] * voxelSize[1] / 2,
							   -size[2] * voxelSize[2] / 2,
							   0 );
	}
}

util::fvector4 MatrixHandler::transformOrigin( const util::fvector4 &origin, const util::fvector4 &voxelSize  ) const
{
	MatrixType matrix = m_correctedMatrix;

	if( !m_Valid ) {
		LOG( Runtime, error ) << "Cannot create transformed origin. First call setVectors.";
		return origin;
	} else {
		return util::fvector4(  ( origin[0] * voxelSize[0] * matrix( 0, 0 ) + origin[1] * voxelSize[1] * matrix( 0, 1 ) + origin[2] * voxelSize[2] * matrix( 0, 2 ) ),
								( origin[0] * voxelSize[0] * matrix( 1, 0 ) + origin[1] * voxelSize[1] * matrix( 1, 1 ) + origin[2] * voxelSize[2] * matrix( 1, 2 ) ),
								( origin[0] * voxelSize[0] * matrix( 2, 0 ) + origin[1] * voxelSize[1] * matrix( 2, 1 ) + origin[2] * voxelSize[2] * matrix( 2, 2 ) ),
								0 );
	}
}
int MatrixHandler::determinant_sign( const boost::numeric::ublas::permutation_matrix<size_t>& pm )
{
	int pm_sign = 1;
	size_t size = pm.size();

	for ( size_t i = 0; i < size; ++i )
		if ( i != pm( i ) )
			pm_sign *= -1.0; // swap_rows would swap a pair of rows here, so we change sign

	return pm_sign;
}

double MatrixHandler::determinant( MatrixType &mat_r )
{
	boost::numeric::ublas::permutation_matrix<size_t> pm( mat_r.size1() );
	double det = 1.0;

	if( boost::numeric::ublas::lu_factorize( mat_r, pm ) ) {
		det = 0.0;
	} else {
		for( unsigned int i = 0; i < mat_r.size1(); i++ )
			det *= mat_r( i, i );

		det = det * determinant_sign( pm );

	}

	return det;
}

}
}
} // end namespace
