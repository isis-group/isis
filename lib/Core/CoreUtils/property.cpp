//
// C++ Implementation: property
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "property.hpp"

namespace isis
{
namespace util
{

bool &PropertyValue::needed() { return m_needed;}
bool PropertyValue::isNeeded()const { return m_needed;}


bool PropertyValue::operator== ( const util::PropertyValue &second )const
{
	return !second.isEmpty() && container==second.container;
}
bool PropertyValue::operator!= ( const util::PropertyValue &second )const
{
	return container!=second.container;
}

bool PropertyValue::operator== ( const ValueBase &second )const
{
	return size()==1 && front()==second;
}
bool PropertyValue::operator!=( const ValueBase& second ) const
{
	return size()==1 && front()!=second;
}


PropertyValue::PropertyValue ( ) : m_needed( false ) {}

PropertyValue PropertyValue::copyByID( short unsigned int ID ) const
{
	PropertyValue ret(size());
	for(const_iterator i=begin();i!=end();i++){
		ValueReference buff=i->copyByID(ID);//@todo use std::move
		ret.transfer(buff);
	}
}

std::string PropertyValue::toString( bool labeled )const
{
	if(size()==1)
		return front().toString(labeled);
	else{
		PropertyValue buff=copyByID(Value<std::string>::staticID);
		std::string ret=listToString(buff.begin(),buff.end(),",","[","]");
		if(labeled && !isEmpty())
			ret+="("+getTypeName()+")";
		return ret;
	}
}
bool PropertyValue::isEmpty() const{return container.empty();}

ValueReference PropertyValue::operator()() const{return front();}

void PropertyValue::push_back( const ValueBase& ref ){
	container.push_back(ValueBase::heap_clone_allocator::allocate_clone( ref ));
}
void PropertyValue::insert( size_t at, const isis::util::ValueBase& ref ){
	container.insert(container.begin()+at,ValueBase::heap_clone_allocator::allocate_clone( ref ));
}
void PropertyValue::transfer( ValueReference& ref ){
	container.push_back(ref.release());
}

ValueBase& PropertyValue::at( size_t n ){return container.at(n);}
const ValueBase& PropertyValue::at( size_t n ) const{return container.at(n);}

ValueBase& PropertyValue::operator[]( size_t n ){return at(n);}
const ValueBase& PropertyValue::operator[]( size_t n ) const{return at(n);}

PropertyValue::iterator PropertyValue::begin(){return container.begin();}
PropertyValue::const_iterator PropertyValue::begin() const{return container.begin();}
PropertyValue::iterator PropertyValue::end(){return container.end();}
PropertyValue::const_iterator PropertyValue::end() const{return container.end();}

PropertyValue::iterator PropertyValue::erase( size_t at ){return container.erase(begin()+at);}
PropertyValue::iterator PropertyValue::erase( iterator first, iterator last ){return container.erase(first,last);}

ValueBase& PropertyValue::front(){
	LOG_IF(size()>1,Debug,warning) << "Doing single value operation on a multi value Property";
	LOG_IF(isEmpty(),Debug,error) << "Doing single value operation on an empy Property, exception ahead ..";
	return container.front();
	
}
const ValueBase& PropertyValue::front() const{return container.front();}

void PropertyValue::reserve( size_t size ){container.reserve(size);}
size_t PropertyValue::size() const{return container.size();}

std::vector< PropertyValue > PropertyValue::splice( size_t len)
{
	std::vector<PropertyValue> ret(size()/len);
	for(size_t i=0;i<ret.size();i++){
		PropertyValue dst=ret[i];
		dst.container.transfer(dst.end(),container.begin(),container.begin()+len,container);
	}
}


// ValueBase hooks
bool PropertyValue::fitsInto( short unsigned int ID ) const{return front().fitsInto(ID);}
std::string PropertyValue::getTypeName() const{return front().getTypeName();}
short unsigned int PropertyValue::getTypeID() const{return front().getTypeID();}

PropertyValue& PropertyValue::add( const PropertyValue& ref ){
	for(size_t i=0;i<ref.size();i++)
		at(i).add(ref[i]);
	return *this;
}
PropertyValue& PropertyValue::substract( const PropertyValue& ref ){
	for(size_t i=0;i<ref.size();i++)
		at(i).substract(ref[i]);
	return *this;
}
PropertyValue& PropertyValue::multiply_me( const PropertyValue& ref ){
	for(size_t i=0;i<ref.size();i++)
		at(i).multiply_me(ref[i]);
	return *this;
}
PropertyValue& PropertyValue::divide_me( const PropertyValue& ref ){
	for(size_t i=0;i<ref.size();i++)
		at(i).divide_me(ref[i]);
	return *this;
}

PropertyValue PropertyValue::plus( const PropertyValue& ref ) const{
	PropertyValue ret(*this);
	ret.add(ref);
	return ret;
}
PropertyValue PropertyValue::minus( const PropertyValue& ref ) const{
	PropertyValue ret(*this);
	ret.substract(ref);
	return ret;
}
PropertyValue PropertyValue::multiply( const PropertyValue& ref ) const{
	PropertyValue ret(*this);
	ret.multiply_me(*this);
	return ret;
}
PropertyValue PropertyValue::divide( const PropertyValue& ref ) const{
	PropertyValue ret(*this);
	ret.divide_me(ref);
	return ret;
}

bool PropertyValue::eq( const PropertyValue& ref ) const{
	bool ret=!ref.isEmpty();
	for(size_t i=0;i<ref.size();i++)
		ret&=at(i).eq(ref[i]);
	return ret;
}
bool PropertyValue::gt( const PropertyValue& ref ) const{
	bool ret=!ref.isEmpty();
	for(size_t i=0;i<ref.size();i++)
		ret&=at(i).gt(ref[i]);
	return ret;
}
bool PropertyValue::lt( const PropertyValue& ref ) const{
	bool ret=!ref.isEmpty();
	for(size_t i=0;i<ref.size();i++)
		ret&=at(i).lt(ref[i]);
	return ret;
}


}
}
