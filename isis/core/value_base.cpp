//
// C++ Implementation: type_base
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "value_base.hpp"
#include "value.hpp"
#include "singletons.hpp"
#include <regex.h>
#include <iomanip>

namespace isis
{
namespace util
{
/// @cond _internal
namespace _internal
{
bool GenericValue::isSameType ( const GenericValue &second ) const
{
	return getTypeID() == second.getTypeID();
}
}
/// @endcond _internal

ValueBase::~ValueBase() {}


const _internal::ValueConverterMap &ValueBase::converters()
{
	static _internal::ValueConverterMap ret; //@todo not using class Singleton because ValueArrayConverterMap is hidden
	return ret;
}

const ValueBase::Converter &ValueBase::getConverterTo( unsigned short ID )const
{
	const _internal::ValueConverterMap::const_iterator f1 = converters().find( getTypeID() );
	assert( f1 != converters().end() );
	const _internal::ValueConverterMap::mapped_type::const_iterator f2 = f1->second.find( ID );
	assert( f2 != f1->second.end() );
	return f2->second;
}
bool ValueBase::convert( const ValueBase &from, ValueBase &to )
{
	const Converter &conv = from.getConverterTo( to.getTypeID() );

	if ( conv ) {
		switch ( conv->convert( from, to ) ) {
		case boost::numeric::cPosOverflow:
			LOG( Runtime, error ) << "Positive overflow when converting " << from.toString( true ) << " to " << to.getTypeName() << ".";
			break;
		case boost::numeric::cNegOverflow:
			LOG( Runtime, error ) << "Negative overflow when converting " << from.toString( true ) << " to " << to.getTypeName() << ".";
			break;
		case boost::numeric::cInRange:
			return true;
			break;
		}
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from "
				<< MSubject( from.toString( true ) ) << " to " << MSubject( to.getTypeName() );
	}

	return false;
}

bool ValueBase::fitsInto( short unsigned int ID ) const //@todo find a better way to do this
{
	std::unique_ptr<ValueBase> to;
	const Converter &conv = getConverterTo( ID );

	if ( conv ) {
		return ( conv->generate( *this, to ) ==  boost::numeric::cInRange );
	} else {
		LOG( Runtime, info )
				<< "I dont know any conversion from "
				<< MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[ID] );
		return false; // return an empty Reference
	}
}

ValueBase::Reference ValueBase::copyByID( short unsigned int ID ) const
{
	std::unique_ptr<ValueBase> to;
	const Converter &conv = getConverterTo( ID );

	if ( conv ) {
		switch ( conv->generate( *this, to ) ) {
		case boost::numeric::cPosOverflow:
			LOG( Runtime, error ) << "Positive overflow when converting " << MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[ID] ) << ".";
			break;
		case boost::numeric::cNegOverflow:
			LOG( Runtime, error ) << "Negative overflow when converting " << MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[ID] ) << ".";
			break;
		case boost::numeric::cInRange:
			break;
		}

		return *to; // return the generated Value-Object - wrapping it into Reference
	} else {
		LOG( Runtime, error ) << "I dont know any conversion from " << MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[ID] );
		return Reference(); // return an empty Reference
	}
}

bool ValueBase::apply(const ValueBase& val)
{
	return convert(val,*this);
}

std::string ValueBase::toString(bool labeled, std::string formatting ) const
{
	std::string ret;
	if(!formatting.empty()){
		if(isFloat() || isInteger()) {
			using namespace std::regex_constants;
			static const std::regex regFormatInt  ( "%[-+#+]*[\\d]*[di]$", ECMAScript|optimize );
			static const std::regex regFormatUInt ( "%[-+#+]*[\\d]*[u]$",  ECMAScript|optimize );
			static const std::regex regFormatFloat( "%[-+#+]*[\\d]*(.\\d+)?[efag]$", ECMAScript|optimize| icase); 
			enum formatting_type {none,integer,uinteger,floating};
			std::smatch fwhat;
			formatting_type type =
				std::regex_search( formatting, fwhat, regFormatInt ) ? integer:
				std::regex_search( formatting, fwhat, regFormatUInt ) ? uinteger:
				std::regex_search( formatting, fwhat, regFormatFloat ) ? floating:
				none;
			char buffer[1024];
			switch(type){
			case integer:
				std::snprintf(buffer,1024,formatting.c_str(),as<int64_t>());
				break;
			case uinteger:
				std::snprintf(buffer,1024,formatting.c_str(),as<uint64_t>());
				break;
			case  floating:
				std::snprintf(buffer,1024,formatting.c_str(),as<double>());
				break;
			case none:;//should never happen / just to stop compiler from whining about missing 'none'
			}
			ret=buffer;
		} else if(is<date>() || is<timestamp>()){
			const std::chrono::seconds sec=std::chrono::duration_cast<std::chrono::seconds>(as<timestamp>().time_since_epoch());
			const time_t tme(sec.count());
			std::stringstream o;
			o<<std::put_time(std::localtime(&tme), formatting.c_str()); // write time and date
			ret=o.str();
		} else {
			LOG(Runtime,error) << "Got formatting string, but value is neither a number nor a timepoint/date, ignoring the formatting";
		}
	} 
	if(ret.empty()){
		Reference ref = copyByID( Value<std::string>::staticID() );

		if ( ref.isEmpty() ) {
			LOG( Debug, error ) << "Automatic conversion of " << getTypeName() << " to string failed. Returning empty string";
		} else {
			ret = ref->castTo<std::string>();
		}
	}
	if ( labeled )ret += "(" + getTypeName() + ")";

	return ret;
}

ValueBase* ValueBase::heap_clone_allocator::allocate_clone( const ValueBase& r )
{
	return r.clone();
}
void ValueBase::heap_clone_allocator::deallocate_clone( const ValueBase* r )
{
	delete r;
}

ValueReference ValueBase::plus( const ValueBase &ref )const{ValueReference ret(*this);return ret->add(ref);}
ValueReference ValueBase::minus( const ValueBase &ref )const{ValueReference ret(*this);return ret->substract(ref);}
ValueReference ValueBase::multiply( const ValueBase &ref )const{ValueReference ret(*this);return ret->multiply_me(ref);}
ValueReference ValueBase::divide( const ValueBase &ref )const{ValueReference ret(*this);return ret->divide_me(ref);}

bool ValueBase::operator!=( const ValueBase &second )const{return !this->operator==(second);}
}
}
