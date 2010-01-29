/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "converter.hpp"
#include "type_base.hpp"
#include "propmap.hpp" // we must have all types here and PropMap was only forward-declared in types.hpp
#include <boost/mpl/for_each.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/mpl/and.hpp>

namespace isis{ namespace util{ namespace _internal{

/////////////////////////////////////////////////////////////////////////////
// general converter version -- does nothing
/////////////////////////////////////////////////////////////////////////////
template<bool NUMERIC,bool SAME,typename SRC, typename DST> class TypeConverter : public TypeConverterBase{
public:
	virtual ~TypeConverter(){}
};

/////////////////////////////////////////////////////////////////////////////
// trivial version -- for conversion of the same type
/////////////////////////////////////////////////////////////////////////////
// @todo template<bool NUMERIC,typename SRC> class TypeConverter<NUMERIC,SRC,SRC> wont work because its ambigous
template<typename SRC> class TypeConverter<true,true,SRC,SRC> : public TypeConverterBase{
	TypeConverter(){
		LOG(CoreDebug,verbose_info)
		<< "Creating trivial numeric converter for " << Type<SRC>::staticName();
	};
public:
	static boost::shared_ptr<TypeConverterBase> create(){
		TypeConverter<true,true,SRC,SRC> *ret=new TypeConverter<true,true,SRC,SRC>;
		return boost::shared_ptr<TypeConverterBase>(ret);
	}
	void convert(const TypeBase& src, TypeBase& dst){
		SRC &dstVal=dst.cast_to_Type<SRC>();
		const SRC &srcVal=src.cast_to_Type<SRC>();
		dstVal = srcVal;
	}
	virtual ~TypeConverter(){}
};
template<typename SRC> class TypeConverter<false,true,SRC,SRC> : public TypeConverterBase{
	TypeConverter(){
		LOG(CoreDebug,verbose_info)
		<< "Creating trivial converter for " << Type<SRC>::staticName();
	};
	public:
		static boost::shared_ptr<TypeConverterBase> create(){
			TypeConverter<false,true,SRC,SRC> *ret=new TypeConverter<false,true,SRC,SRC>;
			return boost::shared_ptr<TypeConverterBase>(ret);
		}
		void convert(const TypeBase& src, TypeBase& dst){
			SRC &dstVal=dst.cast_to_Type<SRC>();
			const SRC &srcVal=src.cast_to_Type<SRC>();
			dstVal = srcVal;
		}
	virtual ~TypeConverter(){}
};


/////////////////////////////////////////////////////////////////////////////
// Numeric version -- uses boost::numeric_cast
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class TypeConverter<true,false,SRC,DST> : public TypeConverterBase{
	TypeConverter(){
		LOG(CoreDebug,verbose_info)
			<< "Creating numeric converter from "
			<< Type<SRC>::staticName() << " to " << Type<DST>::staticName();
	};
public:
	static boost::shared_ptr<TypeConverterBase> create(){
		TypeConverter<true,false,SRC,DST> *ret=new TypeConverter<true,false,SRC,DST>;
		return boost::shared_ptr<TypeConverterBase>(ret);
	}
	void convert(const TypeBase& src, TypeBase& dst){
		try
		{
			typedef boost::numeric::converter<
				DST,SRC,
				boost::numeric::conversion_traits<DST,SRC>,
				boost::numeric::def_overflow_handler,
				boost::numeric::RoundEven<SRC>
			> converter;
			DST &dstVal=dst.cast_to_Type<DST>();
			const SRC &srcVal=src.cast_to_Type<SRC>();
			dstVal = converter::convert(srcVal);
		}
		catch ( boost::numeric::bad_numeric_cast const& e)
		{
			LOG(CoreLog,error)
			<< "Automatic numeric conversion of " << MSubject(src.toString(true)) << " to " << dst.typeName() << " failed: " << e.what();
		}
	}
	virtual ~TypeConverter(){}
};

/////////////////////////////////////////////////////////////////////////////
// string version -- uses lexical_cast to convert from/to string
/////////////////////////////////////////////////////////////////////////////
template<typename DST> class TypeConverter<false,false,std::string,DST> : public TypeConverterBase{
	TypeConverter(){
		LOG(CoreDebug,verbose_info)
		<< "Creating from-string converter for " << Type<DST>::staticName();
	};
public:
	static boost::shared_ptr<TypeConverterBase> create(){
		TypeConverter<false,false,std::string,DST> *ret=new TypeConverter<false,false,std::string,DST>;
		return boost::shared_ptr<TypeConverterBase>(ret);
	}
	void convert(const TypeBase& src, TypeBase& dst){
		DST &dstVal=dst.cast_to_Type<DST>();
		const std::string &srcVal=src.cast_to_Type<std::string>();
		dstVal = boost::lexical_cast<DST>(srcVal);
	}
	virtual ~TypeConverter(){}
};
template<typename SRC> class TypeConverter<false,false,SRC,std::string> : public TypeConverterBase{
	TypeConverter(){
		LOG(CoreDebug,verbose_info)
		<< "Creating to-string converter for " << Type<SRC>::staticName();
	};
public:
	static boost::shared_ptr<TypeConverterBase> create(){
		TypeConverter<false,false,SRC,std::string> *ret=new TypeConverter<false,false,SRC,std::string>;
		return boost::shared_ptr<TypeConverterBase>(ret);
	}
	void convert(const TypeBase& src, TypeBase& dst){
		std::string &dstVal=dst.cast_to_Type<std::string>();
		const SRC &srcVal=src.cast_to_Type<SRC>();
		dstVal = boost::lexical_cast<std::string>(srcVal);
	}
	virtual ~TypeConverter(){}
};
// @todo we cannot parse this stuff yet
template<> class TypeConverter<false,false,std::string,PropMap> : public TypeConverterBase{
public:
	virtual ~TypeConverter(){}
};
template<typename TYPE> class TypeConverter<false,false,std::string,vector4<TYPE> > :public TypeConverterBase{
public:
	virtual ~TypeConverter(){}
};

template<typename SRC> struct inner_add {
	std::map<int, boost::shared_ptr<TypeConverterBase> > &m_subMap;
	inner_add(std::map<int, boost::shared_ptr<TypeConverterBase> > &subMap):m_subMap(subMap){}
	template<typename DST> void operator()(DST){
		typedef boost::mpl::and_<boost::is_arithmetic<SRC>,boost::is_arithmetic<DST> > is_num;
		typedef boost::is_same<SRC,DST> is_same;
		boost::shared_ptr<TypeConverterBase> conv=
			TypeConverter<is_num::value,is_same::value,SRC,DST>::create();
		m_subMap.insert(m_subMap.end(),std::make_pair(Type<DST>::staticId(),conv));
	}
};

struct outer_add {
	std::map< int ,std::map<int, boost::shared_ptr<TypeConverterBase> > > &m_map;
	outer_add(std::map< int ,std::map<int, boost::shared_ptr<TypeConverterBase> > > &map):m_map(map){}
	template<typename SRC> void operator()(SRC){
		boost::mpl::for_each<types>(
			inner_add<SRC>(m_map[Type<SRC>::staticId()])
		);
	}
};
	
TypeConverterMap::TypeConverterMap()
{
	boost::mpl::for_each<types>(outer_add(*this));
	LOG(CoreDebug,info)
	<< "conversion map for " << size() << " types created";
}

}}}
