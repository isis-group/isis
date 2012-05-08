#ifndef ISIS_FILTER_HPP
#define ISIS_FILTER_HPP

#include "chunk.hpp"
#include "image.hpp"
#include "../CoreUtils/progressfeedback.hpp"

#include <list>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

namespace isis
{
namespace filter
{
namespace _internal
{

class FilterBase
{
public:
	bool run();

	void setProgressFeedback( boost::shared_ptr< util::ProgressFeedback > pfb ) { m_progressfeedback = pfb; }
	boost::shared_ptr< util::ProgressFeedback > getProgressFeedback() const  { return m_progressfeedback; }

	//pure virtual
	virtual bool isValid() const = 0;
	virtual std::string getFilterName() const = 0;
	virtual ~FilterBase() {}

	//signals
	boost::signals2::signal<void (const std::string &)> filterStartedSignal;
	boost::signals2::signal<void (const std::string &, bool)> filterFinishedSignal;

	template<typename TYPE>
	void setParameter( const std::string &key, const TYPE &value ) {
		parameterMap.setPropertyAs<TYPE>( key.c_str(), value );
	}
	template<typename TYPE>
	TYPE getResult( const std::string &key ) {
		if( !resultMap.hasProperty(key.c_str() ) ) {
			LOG( data::Runtime, error ) << "The filter " << getFilterName()
										<< " has no result with the key " << key;
			return TYPE();
		} else {
			return resultMap.getPropertyAs<TYPE>(key.c_str() );
		}
	}
	const util::PropertyMap& getResultMap() const { return resultMap; }

protected:
	FilterBase();

	//has to be implemented by the author of the filter
	virtual bool process() = 0;

	bool m_inputIsSet;

	boost::shared_ptr< util::ProgressFeedback > m_progressfeedback;

	util::PropertyMap parameterMap;
	util::PropertyMap resultMap;
};


template<typename TYPE>
class FilterInPlaceBase : public FilterBase
{
public:
	void setInput( TYPE& input ) {
		workingImage = &input;
	}
protected:
    FilterInPlaceBase()
	: workingImage(NULL)
	{};
	TYPE* workingImage;
};


} // end _internal namespace

class ImageFilterInPlace : public _internal::FilterInPlaceBase<data::Image>
{};

class ChunkFilterInPlace : public _internal::FilterInPlaceBase<data::Chunk>
{};


}
}

#endif
