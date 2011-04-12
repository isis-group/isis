#ifndef IMAGEHOLDER_HPP
#define IMAGEHOLDER_HPP

#include <boost/foreach.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <vector>
#include <CoreUtils/propmap.hpp>
#include <DataStorage/image.hpp>
#include "common.hpp"

namespace isis
{
namespace viewer
{

/**
 * Class that holds one image in a vector of data::ValuePtr's
 * It ensures the data is hold in continuous memory and only consists of one type.
 * Furthermore this class handles the meta information of the image
 */

class ImageHolder
{

public:
	typedef data::_internal::ValuePtrBase::Reference ImagePointerType;

	ImageHolder( );

	bool setImage( const data::Image &image, const std::string &filename = "" );

	unsigned short getID() const { return m_ID; }
	void setID( const unsigned short id ) { m_ID = id; }

	std::vector< ImagePointerType > getImageVector() const { return m_ImageVector; }
	std::vector< util::PropertyMap > getTimeStepProperties() const { return m_TimeStepProperties; }
	util::PropertyMap getPropMap() const { return m_PropMap; }
	util::FixedVector<size_t, 4> getImageSize() const { return m_ImageSize; }
	boost::shared_ptr< data::Image >getImage() const { return m_Image; }
	boost::numeric::ublas::matrix<float> getNormalizedImageOrientation( bool transposed = false ) const;
	boost::numeric::ublas::matrix<float> getImageOrientation( bool transposed = false ) const;
	std::pair<util::ValueReference, util::ValueReference> getMinMax() const { return m_MinMax; }
	util::slist getFileNames() const { return m_Filenames; }

	bool operator<( const ImageHolder &ref ) const { return m_ID < ref.getID(); }

private:
	size_t m_NumberOfTimeSteps;
	util::FixedVector<size_t, 4> m_ImageSize;
	util::PropertyMap m_PropMap;
	std::vector< util::PropertyMap > m_TimeStepProperties;
	std::pair<util::ValueReference, util::ValueReference> m_MinMax;
	boost::shared_ptr<data::Image> m_Image;
	util::slist m_Filenames;
	unsigned short m_ID;

	std::vector< ImagePointerType > m_ImageVector;
	bool filterRelevantMetaInformation();
};

}
} //end namespace

#endif