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
		m_parameterMap.setPropertyAs<TYPE>( key.c_str(), value );
	}
	template<typename TYPE>
	const TYPE &getResult( const std::string &key ) {
		if( !m_resultMap.hasProperty(key.c_str() ) ) {
			LOG( data::Runtime, error ) << "The filter " << getFilterName()
										<< " has no result with the key " << key;
			return TYPE();
		} else {
			return m_resultMap.getPropertyAs<TYPE>(key.c_str() );
		}
	}
	const util::PropertyMap& getResultMap() const { return m_resultMap; }

protected:
	FilterBase();

	//has to be implemented by the author of the filter
	virtual bool process() = 0;

	bool m_inputIsSet;

	boost::shared_ptr< util::ProgressFeedback > m_progressfeedback;

	util::PropertyMap m_parameterMap;
	util::PropertyMap m_resultMap;
};



template<typename TYPE>
class Filter11Base : public _internal::FilterBase
{
public:
	void setInput( const TYPE& input ) {
		m_input = boost::shared_ptr<TYPE>( new TYPE( input ) );
		m_inputIsSet = true;
	};

	const TYPE& getOutput() const { return *m_output.get(); }
	TYPE getOutput() { return *m_output.get(); }

protected:
	boost::shared_ptr< TYPE > m_input;
	boost::shared_ptr< TYPE > m_output;

};

} // end _internal namespace

class ImageFilter11 : public _internal::Filter11Base<data::Image>
{};

class ChunkFilter11 : public _internal::Filter11Base<data::Chunk>
{};


}
}

#endif
