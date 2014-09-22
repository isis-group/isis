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
PropertyValue &PropertyValue::operator=(const PropertyValue& other){container=other.container;return *this;}


PropertyValue PropertyValue::copyByID( short unsigned int ID ) const
{
	PropertyValue ret;ret.container.reserve(size());
	for(const_iterator i=begin();i!=end();i++){
		ret.push_back(*(i->copyByID(ID)));
	}
	return ret;
}

std::string PropertyValue::toString( bool labeled )const
{
	if(container.empty()){
		return std::string("\u2205");//utf8 for empty set
	} else if(size()==1)
		return front().toString(labeled);
	else{
		const PropertyValue buff=copyByID(Value<std::string>::staticID);
		std::string ret=listToString(buff.begin(),buff.end(),",","[","]");
		if(labeled && !isEmpty())
			ret+="("+getTypeName()+"["+boost::lexical_cast<std::string>(size())+"])";
		return ret;
	}
}
bool PropertyValue::isEmpty() const{return container.empty();}

ValueReference PropertyValue::operator()() const{return front();}

void PropertyValue::push_back( const PropertyValue& ref ){insert(end(),ref);}
void PropertyValue::push_back( const ValueBase& ref ){insert(end(),ref);}

void PropertyValue::insert( iterator at, const PropertyValue& ref ){
	if(ref.isEmpty()){
		LOG(Debug,warning) << "Not inserting empty Property";
	} else {
		LOG_IF(!isEmpty() && getTypeID()!=ref.getTypeID(),Debug,error) << "Inserting inconsistent type " << MSubject(ref.toString(true)) << " in " << MSubject(*this);
		container.insert(at,ref.container );
	}
}
PropertyValue::iterator PropertyValue::insert( iterator at, const ValueBase& ref ){
	LOG_IF(!isEmpty() && getTypeID()!=ref.getTypeID(),Debug,error) << "Inserting inconsistent type " << MSubject(ref.toString(true)) << " in " << MSubject(*this);
	return container.insert(at,ValueBase::heap_clone_allocator::allocate_clone( ref ));
}

void PropertyValue::transfer(isis::util::PropertyValue::iterator at, PropertyValue& ref)
{
	if(ref.isEmpty()){
		LOG(Debug,error) << "Not transfering empty Property";
	} else {
		LOG_IF(!isEmpty() && getTypeID()!=ref.getTypeID(),Debug,error) << "Inserting inconsistent type " << MSubject(ref.toString(true)) << " in " << MSubject(*this);
		container.transfer(at,ref.container );
	}
}

void PropertyValue::transfer(PropertyValue& ref)
{
	if(ref.isEmpty()){
		LOG(Debug,error) << "Not transfering empty Property";
	} else {
		LOG_IF(!isEmpty(),Debug,warning) << "Transfering " << MSubject(ref.toString(true)) <<  " into non empty " << MSubject(*this);
		container.clear();
		swap(ref);
	}
}
void PropertyValue::swap(PropertyValue& ref)
{
	container.swap(ref.container);
}

bool PropertyValue::transform(uint16_t dstID)
{
	PropertyValue ret,err;
	BOOST_FOREACH(const ValueBase& ref,container){
		const ValueBase::Reference erg = ref.copyByID( dstID );
		if(erg.isEmpty()){
			err=ref;
			break;
		} else
			ret.push_back(*erg);
	}

	if(!err.isEmpty()){
		LOG( Debug, error ) << "Interpretation of " << err << " as " << util::getTypeMap(true,false)[dstID] << " failed. Keeping old type.";
		return false;
	} else {
		container.swap(ret.container);
		return true;
	}
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
const ValueBase& PropertyValue::front() const{
	LOG_IF(size()>1,Debug,warning) << "Doing single value operation on a multi value Property";
	LOG_IF(isEmpty(),Debug,error) << "Doing single value operation on an empy Property, exception ahead ..";
	return container.front();
}

void PropertyValue::reserve( size_t size ){container.reserve(size);}
size_t PropertyValue::size() const{return container.size();}
void PropertyValue::resize( size_t size, const ValueBase &clone ){ // the builtin resize wants non-const clone, so we do our own
	size_t old_size = container.size();
	if( old_size > size ){
		erase( begin()+size, end() );
	} else if( size > old_size ) {
		for( ; old_size != size; ++old_size )
			push_back( clone );
	}
	assert( container.size() == size );
}

std::vector< PropertyValue > PropertyValue::splice( const size_t len )
{
	assert(len);
	std::vector<PropertyValue> ret(ceil(double(size())/len));
	
	for(std::vector<PropertyValue>::iterator dst=ret.begin();container.size()>=len;dst++){ //as long as there is enough transfer given amount
		dst->container.transfer(dst->end(),begin(),begin()+len,container);
		assert(dst->size()==len);
	}
	if(!container.empty()){ // store the remainder in last PropertyValue
		LOG(Runtime,warning) << "Last splice will be " << size() << " entries only, as thats all what is left";
		ret.back().container.transfer(ret.back().end(),container);
	}
	assert(isEmpty());
	return ret;
}


// ValueBase hooks
bool PropertyValue::fitsInto( short unsigned int ID ) const{return front().fitsInto(ID);}
std::string PropertyValue::getTypeName() const{
	LOG_IF(isEmpty(),Debug,error) << "Doing getTypeName on an empty PropertyValue will raise an exception.";
	return at(0).getTypeName();
}
short unsigned int PropertyValue::getTypeID() const{
	LOG_IF(isEmpty(),Debug,error) << "Doing getTypeID on an empty PropertyValue will raise an exception.";
	return at(0).getTypeID();
}

PropertyValue& PropertyValue::add( const PropertyValue& ref ){
	LOG_IF(ref.isEmpty(),Debug,error) << "Adding an empty property, won't do anything";
	for(size_t i=0;i<ref.size();i++)
		at(i).add(ref[i]);
	return *this;
}
PropertyValue& PropertyValue::substract( const PropertyValue& ref ){
	LOG_IF(ref.isEmpty(),Debug,error) << "Substracting an empty property, won't do anything";
	for(size_t i=0;i<ref.size();i++)
		at(i).substract(ref[i]);
	return *this;
}
PropertyValue& PropertyValue::multiply_me( const PropertyValue& ref ){
	LOG_IF(ref.isEmpty(),Debug,error) << "Multiplying with an empty property, won't do anything";
	for(size_t i=0;i<ref.size();i++)
		at(i).multiply_me(ref[i]);
	return *this;
}
PropertyValue& PropertyValue::divide_me( const PropertyValue& ref ){
	LOG_IF(ref.isEmpty(),Debug,error) << "Dividing by an empty property, won't do anything";
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
	ret.multiply_me(ref);
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

namespace std{
template<> void swap< isis::util::PropertyValue >(isis::util::PropertyValue& a, isis::util::PropertyValue& b){a.swap(b);}
}
