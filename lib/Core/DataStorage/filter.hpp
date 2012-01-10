#ifndef ISIS_FILTER_HPP
#define ISIS_FILTER_HPP

#include "chunk.hpp"
#include "image.hpp"
#include "../CoreUtils/progressfeedback.hpp"

#include <list>

namespace isis
{
namespace filter
{
namespace _internal
{

class FilterBase
{
public:
	bool run( boost::shared_ptr<util::ProgressFeedback> = boost::shared_ptr<util::ProgressFeedback> () ) ;

	bool isValid() const { return valid; }
	virtual std::string getFilterName() const = 0;
protected:
	virtual bool process() = 0;
	bool valid;
	boost::shared_ptr< util::ProgressFeedback > progressfeedback;
};

template<typename TYPE, int NInput>
class HasNInput : public FilterBase
{
public:
	void setInput( std::list<TYPE> inputList ) {
		if( inputList.size() != NInput ) {
			LOG( data::Runtime, warning ) << "The size of the input list (" << inputList.size()
										  << ") passed to the filter \"" << getFilterName()
										  << "\" does not coincide with the expected size ("
										  << NInput << "). Filter object is not valid !";
			valid = false;
			return;
		}

		unsigned short index = 0;
		BOOST_FOREACH( typename std::list<TYPE>::const_reference elem, inputList ) {
			m_input[index++] = boost::shared_ptr<TYPE> ( new TYPE( elem ) );
		}
		valid = true;
	}

protected:
	HasNInput() {
		valid = !NInput;
	};
	util::FixedVector<boost::shared_ptr<TYPE>, NInput> m_input;
};

template<typename TYPE, int NOutput>
class HasNOutput
{
public:
	std::list<TYPE> getOutput() const {
		std::list<TYPE> retList;

		for( unsigned short i = 0; i < NOutput; i++ ) {
			retList.push_back( *( m_output[i] ) );
		}

		return retList;
	}

protected:
	util::FixedVector<boost::shared_ptr<TYPE>, NOutput> m_output;
};


template<typename InType, typename OutType, int NInput, int NOutput >
class Filter : public HasNInput<InType, NInput>, public HasNOutput<OutType, NOutput>
{};


//specialization for having fixed input size but variable outputsize
template< typename InType, typename OutType, int NInput>
class Filter < InType, OutType, NInput, -1 > : public HasNInput<InType, NInput>
{
public:
	std::list<OutType> getOutput() const {
		std::list<OutType> retList;
		BOOST_FOREACH( typename std::vector<boost::shared_ptr<OutType> >::const_reference elem, m_output ) {
			retList.push_back( *elem );
		}
		return retList;
	}

protected:
	std::vector<boost::shared_ptr<OutType> > m_output;
};

//specialization for having variable input size but fixed outputsize
template< typename InType, typename OutType, int NOutput>
class Filter < InType, OutType, -1, NOutput > : public FilterBase, public HasNOutput<OutType, NOutput>
{
	void setInput( std::list<InType> inputList ) {
		if( inputList.size() == 0 ) {
			LOG( data::Runtime, warning ) << "The list passed to the filter \""
										  << getFilterName() << "\" is empty. Filter object is not valid!";
		}

		BOOST_FOREACH( typename std::list<InType>::const_reference elem, inputList ) {
			m_input.push_back( boost::shared_ptr<InType> ( new InType( elem ) ) );
		}
		valid = true;
	}
protected:
	std::vector<boost::shared_ptr<InType> > m_input;
};


//specialization for having 0 inputs and fixed outputsize


}

template<unsigned short NInput, unsigned short NOutput>
class ChunkFilter : public _internal::Filter<data::Chunk, data::Chunk, NInput, NOutput>
{};

template<unsigned short NInput, unsigned short NOutput>
class ImageFilter : public _internal::Filter<data::Image, data::Image, NInput, NOutput>
{};



}
}



#endif