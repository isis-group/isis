#ifndef ISIS_FILTER_HPP
#define ISIS_FILTER_HPP

#include "chunk.hpp"
#include "image.hpp"
#include "../CoreUtils/progressfeedback.hpp"

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

	template<typename TYPE>
	void setParameter( const std::string &key, const TYPE &value ) {
		parameterMap.setPropertyAs<TYPE>( key.c_str(), value );
	}
	void setParameterMap( const util::PropertyMap &pMap ) { parameterMap = pMap; }

	template<typename TYPE>
	TYPE getResult( const std::string &key ) {
		if( !resultMap.hasProperty( key.c_str() ) ) {
			LOG( data::Runtime, error ) << "The filter " << getFilterName()
										<< " has no result with the key " << key;
			return TYPE();
		} else {
			return resultMap.getPropertyAs<TYPE>( key.c_str() );
		}
	}
	const util::PropertyMap &getResultMap() const { return resultMap; }

	void setInput( const std::string &label, const data::Image & );
	void setInput( const std::string &label, const data::Chunk & );

protected:
	FilterBase();

	bool m_inputIsSet;

	boost::shared_ptr< util::ProgressFeedback > m_progressfeedback;

	util::PropertyMap parameterMap;
	util::PropertyMap resultMap;

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

} // end _internal namespace

class ImageFilterInPlace : public _internal::InPlace<data::Image>
{};

class ChunkFilterInPlace : public _internal::InPlace<data::Chunk>
{};

class ImageOutputFilter : public _internal::OutputFilterBase<data::Image>
{};

class ChunkOutputFilter : public _internal::OutputFilterBase<data::Chunk>
{};

}
}

#endif
