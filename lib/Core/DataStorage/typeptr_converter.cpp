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

#include "DataStorage/typeptr_converter.hpp"

// #include "type_converter.hpp"
#include "DataStorage/typeptr_base.hpp"
#include "DataStorage/numeric_convert.hpp"
#include "CoreUtils/types.hpp"
#include <boost/mpl/for_each.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/mpl/and.hpp>

#ifdef ISIS_USE_LIBOIL
#include <liboil/liboil.h>
#endif

// @todo we need to know this for lexical_cast (toString)
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

/// @cond _internal
namespace isis
{
namespace data
{
namespace _internal
{

//Define generator - this can be global because its using convert internally
template<typename SRC, typename DST> class TypePtrGenerator: public TypePtrConverterBase
{
public:
	void generate( const TypePtrBase &src, boost::scoped_ptr<TypePtrBase>& dst, const scaling_pair &scaling )const {
		LOG_IF( dst.get(), Debug, warning ) << "Generating into existing value " << dst->toString( true );
		//Create new "stuff" in memory
		TypePtr<DST> *newDat = new TypePtr<DST>( ( DST * )malloc( sizeof( DST )*src.length() ), src.length() );
		dst.reset( newDat );
		convert( src, *dst, scaling );//and convert into that
	}
};
void TypePtrConverterBase::convert( const TypePtrBase &src, TypePtrBase &dst, const scaling_pair &scaling ) const
{
	LOG( Debug, error ) << "Empty conversion was called as conversion from " << src.typeName() << " to " << dst.typeName() << " this is most likely an error.";
}


/////////////////////////////////////////////////////////////////////////////
// general converter version -- does nothing
/////////////////////////////////////////////////////////////////////////////
template<bool NUMERIC, bool SAME, typename SRC, typename DST> class TypePtrConverter : public TypePtrGenerator<SRC, DST>
{
public:
	virtual ~TypePtrConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// trivial version -- for conversion of the same type
/////////////////////////////////////////////////////////////////////////////
template<bool NUMERIC, typename SRC, typename DST> class TypePtrConverter<NUMERIC, true, SRC, DST> : public TypePtrGenerator<SRC, DST>
{
	TypePtrConverter() {
		LOG( Debug, verbose_info )
				<< "Creating trivial copy converter for " << TypePtr<SRC>::staticName();
	};
public:
	static boost::shared_ptr<const TypePtrConverterBase> create() {
		TypePtrConverter<NUMERIC, true, SRC, DST> *ret = new TypePtrConverter<NUMERIC, true, SRC, DST>;
		return boost::shared_ptr<const TypePtrConverterBase>( ret );
	}
	void convert( const TypePtrBase &src, TypePtrBase &dst, const scaling_pair &scaling )const {
		TypePtr<SRC> &dstVal = dst.castToTypePtr<SRC>();
		const SRC *srcPtr = &src.castToTypePtr<SRC>()[0];
		LOG_IF( src.length() < dst.length(), Debug, info ) << "The target is longer than the the source (" << dst.length() << ">" << src.length() << "). Will only copy/convert " << src.length() << " elements";
		LOG_IF( src.length() > dst.length(), Debug, error ) << "The target is shorter than the the source (" << dst.length() << "<" << src.length() << "). Will only copy/convert " << dst.length() << " elements";
		dstVal.copyFromMem( srcPtr, std::min( src.length(), dstVal.length() ) );
	}
	virtual scaling_pair getScaling(const util::_internal::TypeBase &min, const util::_internal::TypeBase &max, autoscaleOption scaleopt = autoscale)const{
		//as we're just copying - its 1/0
		return std::make_pair(
			util::TypeReference(util::Type<uint8_t>(1)),
			util::TypeReference(util::Type<uint8_t>(0))
		);
	}
	virtual ~TypePtrConverter() {}
};


/////////////////////////////////////////////////////////////////////////////
// VC90 thinks bool is numeric - so tell him it isn't
/////////////////////////////////////////////////////////////////////////////
template<typename SRC> class TypePtrConverter<true, false, SRC, bool> : public TypePtrGenerator<SRC, bool> {virtual ~TypePtrConverter() {}};
template<typename DST> class TypePtrConverter<true, false, bool, DST> : public TypePtrGenerator<bool, DST> {virtual ~TypePtrConverter() {}};

/////////////////////////////////////////////////////////////////////////////
// Numeric version -- uses numeric_convert
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class TypePtrConverter<true, false, SRC, DST> : public TypePtrGenerator<SRC, DST>
{
	TypePtrConverter() {
		LOG( Debug, verbose_info )
				<< "Creating numeric converter from "
				<< TypePtr<SRC>::staticName() << " to " << TypePtr<DST>::staticName();
	};
public:
	static boost::shared_ptr<const TypePtrConverterBase> create() {
		TypePtrConverter<true, false, SRC, DST> *ret = new TypePtrConverter<true, false, SRC, DST>;
		return boost::shared_ptr<const TypePtrConverterBase>( ret );
	}
	void convert( const TypePtrBase &src, TypePtrBase &dst, const scaling_pair &scaling )const {
		LOG_IF(scaling.first.empty() || scaling.first.empty(), Debug,error) << "Running conversion with invalid scaling (" << scaling << ") this won't work";
		numeric_convert( src.castToTypePtr<SRC>(), dst.castToTypePtr<DST>(), scaling.first->as<double>(), scaling.second->as<double>() );
	}
	scaling_pair getScaling(const util::_internal::TypeBase &min, const util::_internal::TypeBase &max, autoscaleOption scaleopt = autoscale)const{
		const std::pair<double,double> scale=getNumericScaling<SRC,DST>(min,max,scaleopt);
		return std::make_pair(
			util::TypeReference(util::Type<double>(scale.first)),
			util::TypeReference(util::Type<double>(scale.second))
		);
	}
	virtual ~TypePtrConverter() {}
};


////////////////////////////////////////////////////////////////////////
//OK, thats about the foreplay. Now we get to the dirty stuff.
////////////////////////////////////////////////////////////////////////

///generate a TypePtrConverter for conversions from SRC to any type from the "types" list
template<typename SRC> struct inner_TypePtrConverter {
	std::map<int, boost::shared_ptr<const TypePtrConverterBase> > &m_subMap;
	inner_TypePtrConverter( std::map<int, boost::shared_ptr<const TypePtrConverterBase> > &subMap ): m_subMap( subMap ) {}
	template<typename DST> void operator()( DST ) { //will be called by the mpl::for_each in outer_TypePtrConverter for any DST out of "types"
		//create a converter based on the type traits and the types of SRC and DST
		typedef boost::mpl::and_<boost::is_arithmetic<SRC>, boost::is_arithmetic<DST> > is_num;
		typedef boost::is_same<SRC, DST> is_same;
		boost::shared_ptr<const TypePtrConverterBase> conv =
			TypePtrConverter<is_num::value, is_same::value, SRC, DST>::create();
		//and insert it into the to-conversion-map of SRC
		m_subMap.insert( m_subMap.end(), std::make_pair( TypePtr<DST>::staticID, conv ) );
	}
};

///generate a TypePtrConverter for conversions from any SRC from the "types" list
struct outer_TypePtrConverter {
	std::map< int , std::map<int, boost::shared_ptr<const TypePtrConverterBase> > > &m_map;
	outer_TypePtrConverter( std::map< int , std::map<int, boost::shared_ptr<const TypePtrConverterBase> > > &map ): m_map( map ) {}
	template<typename SRC> void operator()( SRC ) {//will be called by the mpl::for_each in TypePtrConverterMap() for any SRC out of "types"
		boost::mpl::for_each<util::_internal::types>( // create a functor for from-SRC-conversion and call its ()-operator for any DST out of "types"
			inner_TypePtrConverter<SRC>( m_map[TypePtr<SRC>::staticID] )
		);
	}
};
/// @endcond

TypePtrConverterMap::TypePtrConverterMap()
{
	#ifdef ISIS_USE_LIBOIL
	LOG(Debug,info) << "Initializing liboil";
	oil_init();
	#endif // ISIS_USE_LIBOIL
	boost::mpl::for_each<util::_internal::types>( outer_TypePtrConverter( *this ) );
	LOG( Debug, info )
			<< "conversion map for " << size() << " array-types created";
}

}
}
}

