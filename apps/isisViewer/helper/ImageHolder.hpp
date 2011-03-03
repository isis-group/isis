#ifndef IMAGEHOLDER_HPP
#define IMAGEHOLDER_HPP

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <CoreUtils/propmap.hpp>
#include <DataStorage/image.hpp>
#include "common.hpp"

namespace isis {
namespace viewer {
	
/**
 * Class that holds one image in a vector of boost::shared_ptr.
 * It ensures the data is hold in continuous memory and one type.
 */

template <class TYPE>
class ImageHolder
{
	
public:
	ImageHolder();
	bool setImage( data::Image );
	
	
private:
	size_t m_NumberOfTimeSteps;
	bool m_IsValid;
	unsigned short m_TypeID;
	util::fvector4 m_ImageSize;
	
	// do not try this at home!!!
	std::vector< data::_internal::ValuePtrBase::Reference > m_ImageVector;
};

}} //end namespace

#include "ImageHolder_impl.hpp"
#endif