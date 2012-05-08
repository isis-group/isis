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

	void setInputParameter( const util::PropertyMap &inputParameter ) { m_inputParameterMap = inputParameter; }
	const util::PropertyMap& getResultMap() const { return m_resultMap; }

protected:
	FilterBase(){};

	//has to be implemented by the author of the filter
	virtual bool process() const = 0;

	bool m_inputIsSet;

	boost::shared_ptr< util::ProgressFeedback > m_progressfeedback;

	util::PropertyMap m_inputParameterMap;
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
