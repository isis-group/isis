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

#ifdef _MSC_VER
#pragma warning(disable:4800 4996)
#endif

#include "typeptr_converter.hpp"

#include "typeptr_base.hpp"
#include "numeric_convert.hpp"
#include "../CoreUtils/types.hpp"
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

size_t getConvertSize( const ValuePtrBase &src, const ValuePtrBase &dst )
{

	LOG_IF( src.getLength() > dst.getLength(), Runtime, error ) << "The " << src.getLength() << " elements of src wont fit into the destination. Will only convert " << dst.getLength() << " elements.";
	LOG_IF( src.getLength() < dst.getLength(), Runtime, warning ) << "Source is shorter than destination. Will only convert " << src.getLength() << " values";
	return std::min( src.getLength(), dst.getLength() );
}
static bool checkScale(const scaling_pair &scaling){
	assert(!scaling.first.isEmpty() && !scaling.first.isEmpty());
	const double scale =scaling.first->as<double>(),offset =scaling.second->as<double>();
	LOG_IF(scale < 1, Runtime, warning ) << "Downscaling your values by Factor " << scaling.first->as<double>() << " you might lose information.";
	return (scale != 1 || offset );
}

template<typename SRC,typename DST>
scaling_pair getScalingToColor( const util::_internal::ValueBase &min, const util::_internal::ValueBase &max, autoscaleOption scaleopt = autoscale ) {
	double scalMin,scalMax;
	if(min.isFloat() || min.isInteger())scalMin=min.as<double>(); // if min is allready a scalar
		else{ // of not, determine the scalar min from the elements
			const util::color48 minCol=min.as<util::color48>(); //use the "biggest" known color type
			scalMin =*std::min_element(&minCol.r,&minCol.b); // take the lowest value
		}
		if(max.isFloat() || max.isInteger())scalMax=max.as<double>(); // if max is allready a scalar
		else { // of not, determine the scalar min from the elements
			const util::color48 maxCol=max.as<util::color48>(); //use the "biggest" known color type
			scalMax =*std::max_element(&maxCol.r,&maxCol.b); // take the lowest value
		}
		
		const std::pair<double, double> scale = getNumericScaling<SRC, DST>( util::Value<double>(scalMin), util::Value<double>(scalMax), scaleopt );
		return std::make_pair(
			util::ValueReference( util::Value<double>( scale.first ) ),
			util::ValueReference( util::Value<double>( scale.second ) )
		);
}
template<typename SRC,typename DST>
scaling_pair getScalingToComplex( const util::_internal::ValueBase &min, const util::_internal::ValueBase &max, autoscaleOption scaleopt = autoscale ) {
	double scalMin,scalMax;
	if(min.isFloat() || min.isInteger())scalMin=min.as<double>(); // if min is allready a scalar
		else{ // of not, determine the scalar min from the elements
			const std::complex<double> minCpl=min.as<std::complex<double> >(); //use the "biggest" known color type
			scalMin =*std::min_element(&minCpl.real(),&minCpl.imag()); // take the lowest value
		}
		if(max.isFloat() || max.isInteger())scalMax=max.as<double>(); // if max is allready a scalar
		else { // of not, determine the scalar min from the elements
			const std::complex<double> maxCpl=max.as<std::complex<double> >(); //use the "biggest" known color type
			scalMax =*std::max_element(&maxCpl.real(),&maxCpl.imag()); // take the lowest value
		}
		
		const std::pair<double, double> scale = getNumericScaling<SRC, DST>( util::Value<double>(scalMin), util::Value<double>(scalMax), scaleopt );
		return std::make_pair(
			util::ValueReference( util::Value<double>( scale.first ) ),
			util::ValueReference( util::Value<double>( scale.second ) )
		);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// basic numeric conversion class
////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct NumConvImplBase{
	static scaling_pair getScaling( const util::_internal::ValueBase &/*min*/, const util::_internal::ValueBase &/*max*/, autoscaleOption /*scaleopt*/ ) {
		return scaling_pair( util::ValueReference( util::Value<uint8_t>( 1 ) ), util::ValueReference( util::Value<uint8_t>( 0 ) ) );
	}
};
// default generic conversion between numeric types
template<typename SRC,typename DST,bool SAME> struct NumConvImpl:NumConvImplBase{
	static void convert( const SRC *src, DST *dst, const scaling_pair &scaling, size_t size ) {
		checkScale(scaling);
		const double scale =scaling.first->as<double>(),offset =scaling.second->as<double>();
		numeric_convert( src, dst,size,scale , offset );
	}
	static scaling_pair getScaling( const util::_internal::ValueBase &min, const util::_internal::ValueBase &max, autoscaleOption scaleopt ) {
		const std::pair<double, double> scale = getNumericScaling<SRC, DST>( min, max, scaleopt );
		return std::make_pair(
			util::ValueReference( util::Value<double>( scale.first ) ),
			util::ValueReference( util::Value<double>( scale.second ) )
		);
	}
};
// special generic conversion between equal numeric types (maybe we can copy / scaling will be 1/0)
template<typename T> struct NumConvImpl<T,T,true>:NumConvImplBase{
	static void convert( const T *src, T *dst, const scaling_pair &scaling, size_t size ) {
		if(checkScale(scaling)){ 
			const double scale =scaling.first->as<double>(),offset =scaling.second->as<double>();
			numeric_convert( src, dst, size,scale , offset );
		} else { // if there is no scaling - we can copy
			numeric_copy( src, dst, size );
		}
	}
};

// specialisation for bool
template<typename SRC> struct NumConvImpl<SRC,bool,false>:NumConvImplBase{
	static void convert( const SRC *src, bool *dst, const scaling_pair &/*scaling*/,size_t size ) {
		while( size-- )
			*( dst++ ) = ( *( src++ ) != 0 );
	}
};
template<typename DST> struct NumConvImpl<bool,DST,false>:NumConvImplBase{
	static void convert( const bool *src, DST *dst, const scaling_pair &/*scaling*/,size_t size ) {
		while( size-- )
			*( dst++ ) = *( src++ ) ? 1 : 0 ;
	}
};

//default implementation of ValuePtrConverterBase::getScaling - allways returns scaling of 1/0 - should be overridden by real converters if they do use a scaling
scaling_pair ValuePtrConverterBase::getScaling( const isis::util::_internal::ValueBase & min, const isis::util::_internal::ValueBase & max, autoscaleOption scaleopt ) const
{
	return NumConvImplBase::getScaling(min,max,scaleopt);
}

//Define generator - this can be global because its using convert internally
template<typename SRC, typename DST> class ValuePtrGenerator: public ValuePtrConverterBase
{
public:
	void create( boost::scoped_ptr<ValuePtrBase>& dst, const size_t len )const {
		LOG_IF( dst.get(), Debug, warning ) << "Creating into existing value " << dst->toString( true );
		ValuePtr<DST> *newDat = new ValuePtr<DST>( ( DST * )malloc( sizeof( DST )*len ), len );
		dst.reset( newDat );
	}
	void generate( const ValuePtrBase &src, boost::scoped_ptr<ValuePtrBase>& dst, const scaling_pair &scaling )const {
		//Create new "stuff" in memory
		create( dst, src.getLength() );
		assert( dst );
		convert( src, *dst, scaling );//and convert into that
	}
};
void ValuePtrConverterBase::convert( const ValuePtrBase &src, ValuePtrBase &dst, const scaling_pair &/*scaling*/ ) const
{
	LOG( Debug, error ) << "Empty conversion was called as conversion from " << src.getTypeName() << " to " << dst.getTypeName() << " this is most likely an error.";
}


/////////////////////////////////////////////////////////////////////////////
// general converter version -- does nothing and returns 1/0 as scaling
/////////////////////////////////////////////////////////////////////////////
template<bool SRC_NUM,bool DST_NUM, typename SRC, typename DST> class ValuePtrConverter : public ValuePtrGenerator<SRC, DST>
{
public:
	virtual ~ValuePtrConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// trivial version -- for conversion of the same non numeric type (scaling will fail)
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValuePtrConverter<false,false, SRC, DST> : public ValuePtrGenerator<SRC, DST>
{
	ValuePtrConverter() {
		LOG( Debug, verbose_info )  << "Creating trivial copy converter for " << ValuePtr<SRC>::staticName();
	};
public:
	static boost::shared_ptr<const ValuePtrConverterBase> get() {
		ValuePtrConverter<false,false, SRC, DST> *ret = new ValuePtrConverter<false,false, SRC, DST>;
		return boost::shared_ptr<const ValuePtrConverterBase>( ret );
	}
	void convert( const ValuePtrBase &src, ValuePtrBase &dst, const scaling_pair &scaling )const {
		SRC *dstPtr = &dst.castToValuePtr<SRC>()[0];
		const SRC *srcPtr = &src.castToValuePtr<SRC>()[0];
		LOG_IF( checkScale(scaling),  Runtime, error )	<< "Scaling is ignored when copying data of type "	<< src.getTypeName() << " to " << dst.getTypeName() ;
		memcpy( dstPtr, srcPtr, getConvertSize( src, dst )*src.bytesPerElem() );
	}
	virtual ~ValuePtrConverter() {}
};


/////////////////////////////////////////////////////////////////////////////
// Numeric version -- uses numeric_convert
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValuePtrConverter<true,true, SRC, DST> : public ValuePtrGenerator<SRC, DST>
{
	ValuePtrConverter() {
		LOG( Debug, verbose_info ) << "Creating numeric converter from " << ValuePtr<SRC>::staticName() << " to " << ValuePtr<DST>::staticName();
	};
public:
	static boost::shared_ptr<const ValuePtrConverterBase> get() {
		ValuePtrConverter<true,true, SRC, DST> *ret = new ValuePtrConverter<true,true, SRC, DST>;
		return boost::shared_ptr<const ValuePtrConverterBase>( ret );
	}
	void convert( const ValuePtrBase &src, ValuePtrBase &dst, const scaling_pair &scaling )const {
		const SRC *srcPtr=&src.castToValuePtr<SRC>()[0];
		DST *dstPtr= &dst.castToValuePtr<DST>()[0];
		NumConvImpl<SRC,DST,boost::is_same<SRC,DST>::value>::convert(srcPtr,dstPtr,scaling,getConvertSize(src,dst));
	}
	scaling_pair getScaling( const util::_internal::ValueBase &min, const util::_internal::ValueBase &max, autoscaleOption scaleopt = autoscale )const {
		return NumConvImpl<SRC,DST,boost::is_same<SRC,DST>::value >::getScaling(min,max,scaleopt);
	}
	virtual ~ValuePtrConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// complex to complex version
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValuePtrConverter<false,false, std::complex<SRC>, std::complex<DST> > : public ValuePtrGenerator<std::complex<SRC>, std::complex<DST> >
{
	ValuePtrConverter() {
		LOG( Debug, verbose_info )
				<< "Creating complex converter from "
				<< ValuePtr<std::complex<SRC> >::staticName() << " to " << ValuePtr<std::complex<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValuePtrConverterBase> get() {
		ValuePtrConverter<false,false, std::complex<SRC>, std::complex<DST> > *ret = new ValuePtrConverter<false,false, std::complex<SRC>, std::complex<DST> >;
		return boost::shared_ptr<const ValuePtrConverterBase>( ret );
	}
	void convert( const ValuePtrBase &src, ValuePtrBase &dst, const scaling_pair &scaling )const {
		//we do an evil hack here assuming std::complex is POD - at least check if the size of std::complex is reasonable
		BOOST_STATIC_ASSERT(sizeof(std::complex<SRC>) == sizeof(SRC)*2);
		BOOST_STATIC_ASSERT(sizeof(std::complex<DST>) == sizeof(DST)*2);
		
		const SRC *sp = &src.castToValuePtr<std::complex<SRC> >().begin()->real();
		DST *dp = &dst.castToValuePtr<std::complex<DST> >().begin()->real();

		NumConvImpl<SRC,DST,boost::is_same<SRC,DST>::value>::convert(sp,dp,scaling,getConvertSize(src,dst)*2);
	}
	scaling_pair getScaling( const util::_internal::ValueBase &min, const util::_internal::ValueBase &max, autoscaleOption scaleopt = autoscale )const {
		return getScalingToComplex<SRC,DST>(min,max,scaleopt);
	}
	virtual ~ValuePtrConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// color to color version - using numeric_convert on each color with a global scaling
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValuePtrConverter<false,false, util::color<SRC>, util::color<DST> > : public ValuePtrGenerator<util::color<SRC>, util::color<DST> >
{
	ValuePtrConverter() {
		LOG( Debug, verbose_info )
		<< "Creating complex converter from "
		<< ValuePtr<util::color<SRC> >::staticName() << " to " << ValuePtr<util::color<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValuePtrConverterBase> get() {
		ValuePtrConverter<false,false, util::color<SRC>, util::color<DST> > *ret = new ValuePtrConverter<false,false, util::color<SRC>, util::color<DST> >;
		return boost::shared_ptr<const ValuePtrConverterBase>( ret );
	}
	void convert( const ValuePtrBase &src, ValuePtrBase &dst, const scaling_pair &scaling )const {
		const SRC *sp = &src.castToValuePtr<util::color<SRC> >().begin()->r;
		DST *dp = &dst.castToValuePtr<util::color<DST> >().begin()->r;
		NumConvImpl<SRC,DST,false>::convert( sp, dp, scaling, getConvertSize( src, dst )*3);
	}
	scaling_pair getScaling( const util::_internal::ValueBase &min, const util::_internal::ValueBase &max, autoscaleOption scaleopt = autoscale )const {
		return getScalingToColor<SRC,DST>(min,max,scaleopt);
	}
	
	virtual ~ValuePtrConverter() {}
};


////////////////////////////////////////////////////////////////////////
//OK, thats about the foreplay. Now we get to the dirty stuff.
////////////////////////////////////////////////////////////////////////

///generate a ValuePtrConverter for conversions from SRC to any type from the "types" list
template<typename SRC> struct inner_ValuePtrConverter {
	std::map<int, boost::shared_ptr<const ValuePtrConverterBase> > &m_subMap;
	inner_ValuePtrConverter( std::map<int, boost::shared_ptr<const ValuePtrConverterBase> > &subMap ): m_subMap( subMap ) {}
	template<typename DST> void operator()( DST ) { //will be called by the mpl::for_each in outer_ValuePtrConverter for any DST out of "types"
		//create a converter based on the type traits and the types of SRC and DST
		boost::shared_ptr<const ValuePtrConverterBase> conv =
			ValuePtrConverter<boost::is_arithmetic<SRC>::value,boost::is_arithmetic<DST>::value, SRC, DST>::get();
		//and insert it into the to-conversion-map of SRC
		m_subMap.insert( m_subMap.end(), std::make_pair( ValuePtr<DST>::staticID, conv ) );
	}
};

///generate a ValuePtrConverter for conversions from any SRC from the "types" list
struct outer_ValuePtrConverter {
	std::map< int , std::map<int, boost::shared_ptr<const ValuePtrConverterBase> > > &m_map;
	outer_ValuePtrConverter( std::map< int , std::map<int, boost::shared_ptr<const ValuePtrConverterBase> > > &map ): m_map( map ) {}
	template<typename SRC> void operator()( SRC ) {//will be called by the mpl::for_each in ValuePtrConverterMap() for any SRC out of "types"
		boost::mpl::for_each<util::_internal::types>( // create a functor for from-SRC-conversion and call its ()-operator for any DST out of "types"
			inner_ValuePtrConverter<SRC>( m_map[ValuePtr<SRC>::staticID] )
		);
	}
};
/// @endcond

ValuePtrConverterMap::ValuePtrConverterMap()
{
#ifdef ISIS_USE_LIBOIL
	LOG( Debug, info ) << "Initializing liboil";
	oil_init();
#endif // ISIS_USE_LIBOIL
	boost::mpl::for_each<util::_internal::types>( outer_ValuePtrConverter( *this ) );
	LOG( Debug, info )
			<< "conversion map for " << size() << " array-types created";
}

}
}
}

