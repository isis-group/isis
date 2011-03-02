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
// general converter version -- does nothing
/////////////////////////////////////////////////////////////////////////////
template<bool NUMERIC, bool SAME, typename SRC, typename DST> class ValuePtrConverter : public ValuePtrGenerator<SRC, DST>
{
public:
	virtual ~ValuePtrConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// trivial version -- for conversion of the same type
/////////////////////////////////////////////////////////////////////////////
template<bool NUMERIC, typename SRC, typename DST> class ValuePtrConverter<NUMERIC, true, SRC, DST> : public ValuePtrGenerator<SRC, DST>
{
	ValuePtrConverter() {
		LOG( Debug, verbose_info )
				<< "Creating trivial copy converter for " << ValuePtr<SRC>::staticName();
	};
public:
	static boost::shared_ptr<const ValuePtrConverterBase> get() {
		ValuePtrConverter<NUMERIC, true, SRC, DST> *ret = new ValuePtrConverter<NUMERIC, true, SRC, DST>;
		return boost::shared_ptr<const ValuePtrConverterBase>( ret );
	}
	void convert( const ValuePtrBase &src, ValuePtrBase &dst, const scaling_pair &/*scaling*/ )const {
		ValuePtr<SRC> &dstVal = dst.castToValuePtr<SRC>();
		const SRC *srcPtr = &src.castToValuePtr<SRC>()[0];
		LOG_IF( src.getLength() < dst.getLength(), Debug, info ) << "The target is longer than the the source (" << dst.getLength() << ">" << src.getLength() << "). Will only copy/convert " << src.getLength() << " elements";
		LOG_IF( src.getLength() > dst.getLength(), Debug, error ) << "The target is shorter than the the source (" << dst.getLength() << "<" << src.getLength() << "). Will only copy/convert " << dst.getLength() << " elements";
		dstVal.copyFromMem( srcPtr, std::min( src.getLength(), dstVal.getLength() ) );
	}
	virtual scaling_pair getScaling( const util::_internal::ValueBase &/*min*/, const util::_internal::ValueBase &/*max*/, autoscaleOption /*scaleopt*/ )const {
		//as we're just copying - its 1/0
		return std::make_pair(
				   util::ValueReference( util::Value<uint8_t>( 1 ) ),
				   util::ValueReference( util::Value<uint8_t>( 0 ) )
			   );
	}
	virtual ~ValuePtrConverter() {}
};


/////////////////////////////////////////////////////////////////////////////
// VC90 thinks bool is numeric - so tell him it isn't
/////////////////////////////////////////////////////////////////////////////
template<typename SRC> class ValuePtrConverter<true, false, SRC, bool> : public ValuePtrGenerator<SRC, bool> {virtual ~ValuePtrConverter() {}};
template<typename DST> class ValuePtrConverter<true, false, bool, DST> : public ValuePtrGenerator<bool, DST> {virtual ~ValuePtrConverter() {}};

/////////////////////////////////////////////////////////////////////////////
// Numeric version -- uses numeric_convert
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValuePtrConverter<true, false, SRC, DST> : public ValuePtrGenerator<SRC, DST>
{
	ValuePtrConverter() {
		LOG( Debug, verbose_info )
				<< "Creating numeric converter from "
				<< ValuePtr<SRC>::staticName() << " to " << ValuePtr<DST>::staticName();
	};
public:
	static boost::shared_ptr<const ValuePtrConverterBase> get() {
		ValuePtrConverter<true, false, SRC, DST> *ret = new ValuePtrConverter<true, false, SRC, DST>;
		return boost::shared_ptr<const ValuePtrConverterBase>( ret );
	}
	void convert( const ValuePtrBase &src, ValuePtrBase &dst, const scaling_pair &scaling )const {
		LOG_IF( scaling.first.isEmpty() || scaling.first.isEmpty(), Debug, error ) << "Running conversion with invalid scaling (" << scaling << ") this won't work";
		numeric_convert( src.castToValuePtr<SRC>(), dst.castToValuePtr<DST>(), scaling.first->as<double>(), scaling.second->as<double>() );
	}
	scaling_pair getScaling( const util::_internal::ValueBase &min, const util::_internal::ValueBase &max, autoscaleOption scaleopt = autoscale )const {
		const std::pair<double, double> scale = getNumericScaling<SRC, DST>( min, max, scaleopt );
		return std::make_pair(
				   util::ValueReference( util::Value<double>( scale.first ) ),
				   util::ValueReference( util::Value<double>( scale.second ) )
			   );
	}
	virtual ~ValuePtrConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// complex version
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValuePtrConverter<false, false, std::complex<SRC>, std::complex<DST> > : public ValuePtrGenerator<std::complex<SRC>, std::complex<DST> >
{
	ValuePtrConverter() {
		LOG( Debug, verbose_info )
		<< "Creating complex converter from "
		   << ValuePtr<std::complex<SRC> >::staticName() << " to " << ValuePtr<std::complex<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValuePtrConverterBase> get() {
		ValuePtrConverter<false, false, std::complex<SRC>, std::complex<DST> > *ret = new ValuePtrConverter<false, false, std::complex<SRC>, std::complex<DST> >;
		return boost::shared_ptr<const ValuePtrConverterBase>( ret );
	}
	void convert( const ValuePtrBase &src, ValuePtrBase &dst, const scaling_pair &scaling )const {
		LOG_IF( scaling.first.isEmpty() || scaling.first.isEmpty(), Debug, error ) << "Running conversion with invalid scaling (" << scaling << ") this won't work";
		LOG_IF( scaling.first->as<float>() != 1 || scaling.second->as<float>() != 0, Debug, warning)<< "Sorry scaling of complex values is not supportet yet";
		numeric_convert( src.castToValuePtr<std::complex<SRC> >(), dst.castToValuePtr<std::complex<DST> >(), scaling.first->as<double>(), scaling.second->as<double>() );
	}
	scaling_pair getScaling( const util::_internal::ValueBase &min, const util::_internal::ValueBase &max, autoscaleOption scaleopt = autoscale )const {
		const std::pair<double, double> scale = getComplexScaling<SRC, DST>( min, max, scaleopt );
		return std::make_pair(
			util::ValueReference( util::Value<double>( scale.first ) ),
			util::ValueReference( util::Value<double>( scale.second ) )
		);
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
		typedef boost::mpl::and_<boost::is_arithmetic<SRC>, boost::is_arithmetic<DST> > is_num;
		typedef boost::is_same<SRC, DST> is_same;
		boost::shared_ptr<const ValuePtrConverterBase> conv =
			ValuePtrConverter<is_num::value, is_same::value, SRC, DST>::get();
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

