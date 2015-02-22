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


#ifndef ITKADAPTER_HPP_
#define ITKADAPTER_HPP_

#include "DataStorage/image.hpp"
#include "CoreUtils/log.hpp"
#include "CoreUtils/common.hpp"
#include "CoreUtils/vector.hpp"
#include "DataStorage/numeric_convert.hpp"

//external includes
#include <memory>
#include <boost/numeric/ublas/matrix.hpp>

//itk includes
#include <itkImage.h>
#include <itkImportImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>

#include <vector>

namespace isis
{
namespace adapter
{
/**
  * ITKAdapter is capable of taking an isis image object and return an itkImage object.
  */

class itkAdapter
{
public:
	itkAdapter() : m_TypeID( 0 ), m_ChunkPropertyMapVector( NULL ), m_ImagePropertyMap( util::PropertyMap() ), m_RelevantDim( 0 ) {};
	/**
	  * Converts an isis image object in an itk image.
	  * \param src boost sharedpointer of the isisImage
	  * \param behaveAsItkReader if set to true, the orientation matrix will be transformed like <br>
	  * -1 -1 -1 -1 <br>
	  * -1 -1 -1 -1 <br>
	  *  1  1  1  1 <br>
	  *  to meet the itk intern data representation requirements.<br>
	  *  If set to false, orientation matrix will not be changed.
	  *  \returns an itk smartpointer on the itkImage object
	  */
	template<typename TImage> typename TImage::Pointer
	makeItkImageObject( const data::Image &src, const bool behaveAsItkReader = true );

	/**
	  * Converts an itk image object in an isis image object.
	  * \param src itk smartpointer of the itkImage
	  * \param behaveAsItkWriter if set to true, the orientation matrix will be transformed like <br>
	  * -1 -1 -1 -1 <br>
	  * -1 -1 -1 -1 <br>
	  *  1  1  1  1 <br>
	  *  to meet the itk intern data representation requirements.<br>
	  *  If you used the makeItkImageObject with
	  *  behaveAsItkReader=true, behaveAsItkWriter should also be true here to preserve the right orientation. <br>
	  *  If set to false, orientation matrix will not be changed.
	  *  \returns an isis::data::ImageList.
	  */
	template<typename TImage> std::list<data::Image>
	makeIsisImageObject( const typename TImage::Pointer src, const bool behaveAsItkWriter = true );

protected:
	//should not be loaded directly
	itkAdapter( const std::shared_ptr<data::Image> src ) : m_ImageISIS( src ) {};
	itkAdapter( const itkAdapter & ) {};

private:

	std::shared_ptr<data::Image> m_ImageISIS;
	//  data::Image m_ImageISIS;
	unsigned short m_TypeID;
	std::vector< std::shared_ptr<util::PropertyMap> > m_ChunkPropertyMapVector;
	util::PropertyMap m_ImagePropertyMap;
	size_t m_RelevantDim;

	template<typename TInput, typename TOutput> typename TOutput::Pointer internCreateItk( const bool behaveAsItkReader );

	template<typename TImageITK, typename TOutputISIS> std::list<data::Image> internCreateISIS( const typename TImageITK::Pointer src, const bool behaveAsItkWriter );
};

}
}// end namespace

#include "itkAdapter_impl.hpp"

#endif
