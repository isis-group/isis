#ifndef ISIS_FILTER_HPP
#define ISIS_FILTER_HPP

#include "chunk.hpp"
#include "image.hpp"
#include "../CoreUtils/progressfeedback.hpp"
#include "../CoreUtils/progparameter.hpp"

#include <list>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#ifdef _OPENMP
#include <omp.h>
#endif /*_OPENMP*/

namespace isis
{
namespace filter
{

#ifdef _OPENMP
struct omp {
	static void setNumberOfThreads( const uint16_t &threads ) {
		omp_set_num_threads( threads );
	}

	static void useAllAvailableThreads() {
		omp_set_num_threads( omp_get_num_procs() );
	}
	static uint16_t getNumberOfAvailableThreads() {
		return omp_get_num_procs();
	}
};
#endif

namespace _internal
{

class FilterBase
{
public:

	void setProgressFeedback( boost::shared_ptr< util::ProgressFeedback > pfb ) { m_progressfeedback = pfb; }
	boost::shared_ptr< util::ProgressFeedback > getProgressFeedback() const  { return m_progressfeedback; }

	//pure virtual
	virtual bool isValid() const = 0;
	virtual std::string getFilterName() const = 0;
	virtual std::string getDescription() const { return std::string( "not_set" );};
	virtual ~FilterBase() {}

	//signals
	boost::signals2::signal<void ( const std::string & )> filterStartedSignal;
	boost::signals2::signal<void ( const std::string &, bool )> filterFinishedSignal;

	void setInput( const std::string &label, const data::Image & );
	void setInput( const std::string &label, const data::Chunk & );

	util::ParameterMap parameters;
	util::ParameterMap results;

protected:
	FilterBase();

	bool m_inputIsSet;

	boost::shared_ptr< util::ProgressFeedback > m_progressfeedback;

	std::map< std::string, boost::shared_ptr<data::Image> > m_additionalImages;
	std::map< std::string, boost::shared_ptr<data::Chunk> > m_additionalChunks;

};

template<typename TYPE>
class NotInPlace : public FilterBase
{
public:
	bool run() {
		if( isValid() ) {
			filterStartedSignal( getFilterName() );
			const bool success = process();
			filterFinishedSignal( getFilterName(), success );
			return success;
		} else {
			LOG( data::Runtime, warning ) << "The filter \"" << getFilterName()
										  << "\" is not valid. Will not run it!";
			return false;
		}
	}
protected:
	virtual bool process() = 0;
};

template<typename TYPE>
class InPlace : public FilterBase
{
public:
	bool run( TYPE &input ) {
		if( isValid() ) {
			filterStartedSignal( getFilterName() );
			const bool success = process( input );
			filterFinishedSignal( getFilterName(), success );
			return success;
		} else {
			LOG( data::Runtime, warning ) << "The filter \"" << getFilterName()
										  << "\" is not valid. Will not run it!";
			return false;
		}
	}
protected:
	virtual bool process( TYPE &input ) = 0;
};


template<typename TYPE>
class OutputFilterBase : public NotInPlace<TYPE>
{
public:
	TYPE getOutput() const { return *output; }
protected:
	OutputFilterBase() {};
	boost::shared_ptr<TYPE> output;

};



class ImageFilterInPlace : public _internal::InPlace<data::Image>
{};

class ChunkFilterInPlace : public _internal::InPlace<data::Chunk>
{};

class ImageOutputFilter : public _internal::OutputFilterBase<data::Image>
{};

class ChunkOutputFilter : public _internal::OutputFilterBase<data::Chunk>
{};

} // end _internal namespace
}
}

#endif
