#ifndef CONVERTTOPYTHON_HPP
#define CONVERTTOPYTHON_HPP


#include <map>
#include <list>
#include "CoreUtils/common.hpp"
#include "CoreUtils/property.hpp"
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/gregorian/gregorian_io.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <datetime.h>


using namespace boost::python;

namespace isis {

namespace python {

namespace _internal {
	
	
	
struct PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) = 0;
};

template<bool ISNUM, typename T>
struct PyObjectGenerator : PyObjectGeneratorBase {};


//conversion for all numeric values
template<typename T>
struct PyObjectGenerator<true, T> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		return  api::object( value.as<T>());
	}
};

template<typename T>
struct PyObjectGenerator<false, T> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		return api::object( value.as<T>());
	}
};

//dates
template<>
struct PyObjectGenerator<false, boost::gregorian::date> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		PyDateTime_IMPORT;
		boost::gregorian::date date = value.as<boost::gregorian::date>();
		return api::object(handle<>(borrowed( PyDate_FromDate( static_cast<int>( date.year() ), 
								static_cast<int>(date.month() ), 
								static_cast<int>(date.day() ) ) ) ) ); 
								
		
	}
};

//ptime
template<>
struct  PyObjectGenerator<false, boost::posix_time::ptime> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		PyDateTime_IMPORT;
		boost::posix_time::ptime time = value.as<boost::posix_time::ptime>();
		return api::object(handle<>(borrowed( PyDateTime_FromDateAndTime( static_cast<int>( time.date().year() ),
										  static_cast<int>( time.date().month() ),
										  static_cast<int>( time.date().day() ),
										  static_cast<int>( time.time_of_day().hours() ),
										  static_cast<int>( time.time_of_day().minutes() ),
										  static_cast<int>( time.time_of_day().seconds() ),
										  static_cast<int>( time.time_of_day().total_milliseconds() ) ))));
										  
	}
	
};

//vectors
template<>
struct PyObjectGenerator<false, util::ivector4> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		return api::object( value.as<util::ivector4>() );
	}
};

template<>
struct PyObjectGenerator<false, util::dvector4> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		return api::object( value.as<util::dvector4>() );
	}
};

template<>
struct PyObjectGenerator<false, util::fvector4> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		return api::object( value.as<util::fvector4>() );
	}
};

template<>
struct PyObjectGenerator<false, util::ilist> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		list retList;
		BOOST_FOREACH( util::ilist::const_reference ref, value.as<util::ilist>() )
		{
			retList.append<int32_t>(ref);
		}
		return retList;
	}
};

template<>
struct PyObjectGenerator<false, util::dlist> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		list retList;
		BOOST_FOREACH( util::dlist::const_reference ref, value.as<util::dlist>() )
		{
			retList.append<double>(ref);
		}
		return retList;
	}
};

template<>
struct PyObjectGenerator<false, util::slist> : PyObjectGeneratorBase
{
	virtual api::object convert( util::_internal::ValueBase &value ) {
		list retList;
		BOOST_FOREACH( util::slist::const_reference ref, value.as<util::slist>() )
		{
			retList.append<std::string>(ref);
		}
		return retList;
	}
};

struct Generator 
{
	Generator( std::map<unsigned short, boost::shared_ptr<PyObjectGeneratorBase> > &tMap) : typeMap(tMap) {}
	
	template<typename T>
	void operator() ( const T& ) { 
		typeMap[(unsigned short)util::Value<T>::staticID] = 
			boost::shared_ptr<PyObjectGeneratorBase>(
				new PyObjectGenerator<boost::is_arithmetic<T>::value, T>() ); 
		
	}
	std::map<unsigned short, boost::shared_ptr<PyObjectGeneratorBase> > &typeMap;
};


class TypesMap : public std::map<unsigned short, boost::shared_ptr<PyObjectGeneratorBase> >
{
public:
	void create() {
		boost::mpl::for_each<util::_internal::types>( Generator( *this ) );
	}
};

	
	
	
}}} // end namespace


#endif
