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

#include "value_converter.hpp"
#include "value_base.hpp"
#include "value.hpp"
#include <boost/mpl/for_each.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/mpl/and.hpp>

// @todo we need to know this for lexical_cast (toString)
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <complex>


/// @cond _internal
namespace isis
{
namespace util
{
API_EXCLUDE_BEGIN
namespace _internal
{

//Define generator - this can be global because its using convert internally
template<typename SRC, typename DST> class ValueGenerator: public ValueConverterBase
{
public:
	void create( boost::scoped_ptr<ValueBase>& dst )const {
		LOG_IF( dst.get(), Debug, error ) << "Generating into existing value " << dst->toString( true ) << " (dropping this).";
		Value<DST> *ref = new Value<DST>;
		dst.reset( ref );
	}
	boost::numeric::range_check_result generate( const ValueBase &src, boost::scoped_ptr<ValueBase>& dst )const {
		create( dst );
		assert( dst );
		const boost::numeric::range_check_result result = convert( src.castToType<SRC>(), *dst );
		return result;
	}
};

/////////////////////////////////////////////////////////////////////////////
// general converter version -- does nothing
/////////////////////////////////////////////////////////////////////////////
template<bool NUMERIC, bool SAME, typename SRC, typename DST> class ValueConverter : public ValueGenerator<SRC, DST>
{
public:
	//uncomment this to see which conversions are not generated - be carefull, thats f***king much
	/*  static boost::shared_ptr<const ValueConverterBase> get() {
	        std::cout <<
	            "There will be no " << (SAME?"copy":NUMERIC?"numeric":"non-numeric") <<  " conversion for " <<
	            util::Value<SRC>::staticName() << " to " << util::Value<DST>::staticName() << std::endl;
	        return boost::shared_ptr<const ValueConverterBase>();
	    }*/
	virtual ~ValueConverter() {}
};
/////////////////////////////////////////////////////////////////////////////
// trivial version -- for conversion of the same type
/////////////////////////////////////////////////////////////////////////////
template<bool NUMERIC, typename SRC, typename DST> class ValueConverter<NUMERIC, true, SRC, DST> : public ValueGenerator<SRC, DST>
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating trivial copy converter for " << Value<SRC>::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<NUMERIC, true, SRC, DST> *ret = new ValueConverter<NUMERIC, true, SRC, DST>;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		SRC &dstVal = dst.castTo<SRC>();
		const SRC &srcVal = src.castTo<SRC>();
		dstVal = srcVal;
		return boost::numeric::cInRange;
	}
	virtual ~ValueConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// some helper
/////////////////////////////////////////////////////////////////////////////

// global numeric overflow handler @todo this is NOT thread-safe
struct NumericOverflowHandler {
	static boost::numeric::range_check_result result;
	void operator() ( boost::numeric::range_check_result r ) { // throw bad_numeric_conversion derived
		result = r;
	}
};
boost::numeric::range_check_result NumericOverflowHandler::result = boost::numeric::cInRange;

// basic numeric to numeric conversion (does runding and handles overlow)
template<typename SRC, typename DST> boost::numeric::range_check_result num2num( const SRC &src, DST &dst )
{
	typedef boost::numeric::converter <
	DST, SRC,
	   boost::numeric::conversion_traits<DST, SRC>,
	   NumericOverflowHandler,
	   boost::numeric::RoundEven<SRC>
	   > converter;
	NumericOverflowHandler::result = boost::numeric::cInRange;
	dst = converter::convert( src );
	return NumericOverflowHandler::result;
}

template<typename DST> boost::numeric::range_check_result str2scalar( const std::string &src, DST &dst )
{
	try {
		if( boost::is_arithmetic<DST>::value ) { // if a converter from double is available first map to double and then convert that into DST
			return num2num<double, DST>( Value<double>( src ), dst );
		} else { // otherwise try direct mapping (rounding will fail)
			LOG( Debug, warning ) << "using lexical_cast to convert from string to "
								  << Value<DST>::staticName() << " no rounding can be done.";
			dst = boost::lexical_cast<DST>( src );
		}
	} catch( const boost::bad_lexical_cast & ) {
		dst = DST();
		LOG( Runtime, error ) << "Miserably failed to interpret " << MSubject( src ) << " as " << Value<DST>::staticName() << " returning " << MSubject( DST() );
	}

	return boost::numeric::cInRange;
}
//this is trivial
template<> boost::numeric::range_check_result str2scalar<std::string>( const std::string &src, std::string &dst )
{
	dst = src;
	return boost::numeric::cInRange;
}
// needs special handling
template<> boost::numeric::range_check_result str2scalar<boost::posix_time::ptime>( const std::string &src, boost::posix_time::ptime &dst )
{
	dst = boost::posix_time::time_from_string( src.c_str() ); //first try "2002-01-20 23:59:59.000"

	if( dst.is_not_a_date_time() ) // try iso formatting
		dst = boost::posix_time::from_iso_string( src.c_str() );

	LOG_IF( dst.is_not_a_date_time(), Runtime, error ) // if its still broken at least tell the user
			<< "Miserably failed to interpret " << MSubject( src ) << " as " << Value<boost::posix_time::ptime>::staticName() << " returning " << MSubject( dst );
	return boost::numeric::cInRange;
}
// this as well (interpret everything like true/false yes/no y/n)
template<> boost::numeric::range_check_result str2scalar<bool>( const std::string &src, bool &dst )
{
	const std::string srcVal = boost::algorithm::to_lower_copy<std::string>( src );

	if (  srcVal == "true" || srcVal == "y" || srcVal == "yes" ) {
		dst = true;
	} else if ( srcVal == "false" || srcVal == "n" || srcVal == "no" ) {
		dst = false;
	} else {
		LOG( Runtime, warning ) << util::MSubject( src ) << " is ambiguous while converting to " << Value<bool>::staticName();
		return boost::numeric::cPosOverflow;
	}

	return boost::numeric::cInRange;
}
//and this (lexical_cast doesnt work here, because it creates a temporary buffer which will screw our dst)
template<> boost::numeric::range_check_result str2scalar<Selection>( const std::string &src, Selection &dst )
{
	if ( dst.set( src.c_str() ) )
		return boost::numeric::cInRange;
	else
		return boost::numeric::cPosOverflow; //if the string is not "part" of the selection we count this as positive overflow
}

template<bool IS_NUM> struct Tokenizer { //jump from number to number in the string ignoring anything else
	static std::list<std::string> run( const std::string &src ) {
		std::list<std::string> ret;
		const char *mask = "0123456789-eE.";
		const char *start_mask = "0123456789-";

		for( size_t i = src.find_first_of( start_mask ), end; i < std::string::npos; i = src.find_first_of( start_mask, end ) ) {
			end = src.find_first_not_of( mask, i );
			const std::string numstr = src.substr( i, end - i );
			ret.push_back( numstr );
		}

		return ret;
	}
};
template<> struct Tokenizer<false> { // not for numbers / tokenize string at spaces,"," and ";"
	static std::list<std::string> run( const std::string &src ) {
		return util::stringToList<std::string>( src, boost::regex( "[\\s,;]+" ) );
	}
};

template<typename DST> struct StrTransformer {
	boost::numeric::range_check_result range_ok;
	StrTransformer(): range_ok( boost::numeric::cInRange ) {}
	DST operator()( const std::string &src ) {
		DST ret;
		const boost::numeric::range_check_result result = str2scalar( src, ret );

		if( result != boost::numeric::cInRange )
			range_ok = result; // keep the first error

		return ret;
	}
};

//helper to convert strings to FixedVectors
template<typename DST, int NUM> boost::numeric::range_check_result convertStr2Vector( const ValueBase &src, FixedVector<DST, NUM> &dstList )
{
	const std::list<std::string> srcList = Tokenizer<boost::is_arithmetic<DST>::value>::run( src.castTo<std::string>() ); // tokenize the string based on the target type
	std::list< std::string >::const_iterator end = srcList.begin();
	std::advance( end, std::min<size_t>( srcList.size(), NUM ) ); // use a max of NUM tokens
	StrTransformer<DST> transformer; // create a transformer from string to DST
	std::transform( srcList.begin(), end, dstList.begin(), transformer ); // transform the found strings to the destination
	return transformer.range_ok;
}

// additional base for converters which use another converter
template<typename SRC, typename DST> struct SubValueConv {
	boost::shared_ptr<const ValueConverterBase> sub_conv;
};
template<typename SRC, typename DST> struct IterableSubValueConv: SubValueConv<SRC, DST> {
	template<typename SRC_LST, typename DST_LST> boost::numeric::range_check_result
	convertIter2Iter( const SRC_LST &srcLst, DST_LST &dstLst )const {
		boost::numeric::range_check_result ret = boost::numeric::cInRange;

		typename SRC_LST::const_iterator srcAt = srcLst.begin(), srcEnd = srcLst.end();
		typename DST_LST::iterator dstBegin = dstLst.begin(), dstEnd = dstLst.end();

		while( srcAt != srcEnd ) { //slow and ugly, but flexible

			if( dstBegin != dstEnd ) {
				Value<DST> elem_dst;
				const boost::numeric::range_check_result result = SubValueConv<SRC, DST>::sub_conv->convert( Value<SRC>( *srcAt ), elem_dst );

				if ( ret == boost::numeric::cInRange && result != boost::numeric::cInRange )
					ret = result;

				*( dstBegin++ ) = ( DST )elem_dst;
			} else if( *srcAt != SRC() )
				return boost::numeric::cPosOverflow; // abort and send positive overflow if source wont fit into destination

			srcAt++;
		}

		return ret;
	}

};
template<typename CLASS, typename SRC, typename DST> static boost::shared_ptr<const ValueConverterBase> getFor()
{
	typedef boost::mpl::and_<boost::is_arithmetic<SRC>, boost::is_arithmetic<DST> > is_num;
	typedef boost::is_same<SRC, DST> is_same;
	boost::shared_ptr<const ValueConverterBase> sub_conv = ValueConverter<is_num::value, is_same::value, SRC, DST>::get();

	if ( sub_conv ) {
		boost::shared_ptr<CLASS > ret( new CLASS );
		ret->sub_conv = sub_conv;
		return ret;
	} else {
		return boost::shared_ptr<const ValueConverterBase>();
	}
}

// special to string conversions
template<typename T> std::string toStringConv( const T &src )
{
	std::stringstream s;
	s << std::boolalpha << src; // bool will be converted to true/false
	return s.str();
}
template<> std::string toStringConv<uint8_t>( const uint8_t &src ) {return toStringConv( static_cast<uint16_t>( src ) );}
template<> std::string toStringConv<int8_t> ( const  int8_t &src ) {return toStringConv( static_cast< int16_t>( src ) );}


/////////////////////////////////////////////////////////////////////////////
// Numeric version -- uses num2num
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValueConverter<true, false, SRC, DST> : public ValueGenerator<SRC, DST>
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating numeric converter from "
				<< Value<SRC>::staticName() << " to " << Value<DST>::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<true, false, SRC, DST> *ret = new ValueConverter<true, false, SRC, DST>;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return num2num( src.castTo<SRC>(), dst.castTo<DST>() );
	}
	virtual ~ValueConverter() {}
};

///////////////////////////////////////////////////////////////////////////////
// Conversion between complex numbers -- uses num2num
/////////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValueConverter<false, false, std::complex<SRC>, std::complex<DST> > : public ValueGenerator<std::complex<SRC>, std::complex<DST> >
{
	ValueConverter() {
		LOG( Debug, verbose_info ) << "Creating complex-complex converter from " << Value<std::complex<SRC> >::staticName() << " to " << Value<std::complex<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::complex<SRC>, std::complex<DST> > *ret = new ValueConverter<false, false, std::complex<SRC>, std::complex<DST> >;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return num2num( src.castTo<std::complex<SRC> >(), dst.castTo<std::complex<DST> >() );
	}
	virtual ~ValueConverter() {}
};

///////////////////////////////////////////////////////////////////////////////
// Conversion between color -- uses num2num
/////////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValueConverter<false, false, color<SRC>, color<DST> > : public ValueGenerator<color<SRC>, color<DST> >
{
	ValueConverter() {
		LOG( Debug, verbose_info ) << "Creating color converter from " << Value<color<SRC> >::staticName() << " to " << Value<color<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, color<SRC>, color<DST>  > *ret = new ValueConverter<false, false, color<SRC>, color<DST>  >;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		boost::numeric::range_check_result res = boost::numeric::cInRange;
		const SRC *srcVal = &src.castTo<color<SRC> >().r;
		DST *dstVal = &dst.castTo<color<DST> >().r;

		for( uint_fast8_t i = 0; i < 3; i++ ) {
			const boost::numeric::range_check_result result = num2num( srcVal[i], dstVal[i] );

			if( result != boost::numeric::cInRange )res = result;
		}

		return res;
	}
	virtual ~ValueConverter() {}
};

///////////////////////////////////////////////////////////////////////////////
// Conversion for "all" to complex numbers -- uses ValueConverter on the real part
/////////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValueConverter<false, false, SRC, std::complex<DST> > :
	public ValueGenerator<SRC, std::complex<DST> >, private SubValueConv<SRC, DST >
{
	ValueConverter( ) {
		LOG( Debug, verbose_info )
				<< "Creating number-complex converter from "
				<< Value<SRC>::staticName() << " to " << Value<std::complex<DST> >::staticName();
	};
	friend boost::shared_ptr<const ValueConverterBase> getFor<ValueConverter<false, false, SRC, std::complex<DST> >, SRC, DST >();
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		return getFor<ValueConverter<false, false, SRC, std::complex<DST> >, SRC, DST >();
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		std::complex<DST> &dstVal = dst.castTo<std::complex<DST> >();
		Value<DST> real;
		boost::numeric::range_check_result ret = this->sub_conv->convert( src, real );

		if( ret == boost::numeric::cInRange )
			dstVal = std::complex<DST>( ( DST )real, DST() );

		return ret;
	}
	virtual ~ValueConverter() {}
};


/////////////////////////////////////////////////////////////////////////////
// vectorX to vectorX version -- uses IterableSubValueConv::convertIter2Iter
/////////////////////////////////////////////////////////////////////////////
// vector4 => vector4
template<typename SRC, typename DST > class ValueConverter<false, false, vector4<SRC>, vector4<DST> >:
	public ValueGenerator<vector4<SRC>, vector4<DST> >, private IterableSubValueConv<SRC, DST >
{
	ValueConverter( ) {
		LOG( Debug, verbose_info ) << "Creating vector converter from " << Value<vector4<SRC> >::staticName() << " to " << Value<vector4<DST> >::staticName();
	};
	friend boost::shared_ptr<const ValueConverterBase> getFor<ValueConverter<false, false, vector4<SRC>, vector4<DST> >, SRC, DST>();
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		return getFor<ValueConverter<false, false, vector4<SRC>, vector4<DST> >, SRC, DST>();
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return IterableSubValueConv<SRC, DST >::convertIter2Iter( src.castTo<vector4<SRC> >(), dst.castTo<vector4<DST> >() );
	}
	virtual ~ValueConverter() {}
};
// vector3 => vector4
template<typename SRC, typename DST > class ValueConverter<false, false, vector3<SRC>, vector4<DST> >:
	public ValueGenerator<vector3<SRC>, vector4<DST> >, private IterableSubValueConv<SRC, DST >
{
	ValueConverter( ) {
		LOG( Debug, verbose_info ) << "Creating vector converter from " << Value<vector3<SRC> >::staticName() << " to " << Value<vector4<DST> >::staticName();
	};
	friend boost::shared_ptr<const ValueConverterBase> getFor<ValueConverter<false, false, vector3<SRC>, vector4<DST> >, SRC, DST>();
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		return getFor<ValueConverter<false, false, vector3<SRC>, vector4<DST> >, SRC, DST>();
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return IterableSubValueConv<SRC, DST >::convertIter2Iter( src.castTo<vector3<SRC> >(), dst.castTo<vector4<DST> >() );
	}
	virtual ~ValueConverter() {}
};
// vector4 => vector3
template<typename SRC, typename DST > class ValueConverter<false, false, vector4<SRC>, vector3<DST> >:
	public ValueGenerator<vector4<SRC>, vector3<DST> >, private IterableSubValueConv<SRC, DST >
{
	ValueConverter( ) {
		LOG( Debug, verbose_info ) << "Creating vector converter from " << Value<vector4<SRC> >::staticName() << " to " << Value<vector3<DST> >::staticName();
	};
	friend boost::shared_ptr<const ValueConverterBase> getFor<ValueConverter<false, false, vector4<SRC>, vector3<DST> >, SRC, DST>();
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		return getFor<ValueConverter<false, false, vector4<SRC>, vector3<DST> >, SRC, DST>();
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return IterableSubValueConv<SRC, DST >::convertIter2Iter( src.castTo<vector4<SRC> >(), dst.castTo<vector3<DST> >() );
	}
	virtual ~ValueConverter() {}
};
// vector3 => vector3
template<typename SRC, typename DST > class ValueConverter<false, false, vector3<SRC>, vector3<DST> >:
	public ValueGenerator<vector3<SRC>, vector3<DST> >, private IterableSubValueConv<SRC, DST >
{
	ValueConverter( ) {
		LOG( Debug, verbose_info ) << "Creating vector converter from " << Value<vector3<SRC> >::staticName() << " to " << Value<vector3<DST> >::staticName();
	};
	friend boost::shared_ptr<const ValueConverterBase> getFor<ValueConverter<false, false, vector3<SRC>, vector3<DST> >, SRC, DST>();
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		return getFor<ValueConverter<false, false, vector3<SRC>, vector3<DST> >, SRC, DST>();
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return IterableSubValueConv<SRC, DST >::convertIter2Iter( src.castTo<vector3<SRC> >(), dst.castTo<vector3<DST> >() );
	}
	virtual ~ValueConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// list to list version -- uses IterableSubValueConv::convertIter2Iter
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST > class ValueConverter<false, false, std::list<SRC>, std::list<DST> >:
	public ValueGenerator<std::list<SRC>, std::list<DST> >, private IterableSubValueConv<SRC, DST >
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating list converter from "
				<< Value<std::list<SRC> >::staticName() << " to " << Value<std::list<DST> >::staticName();
	};
	friend boost::shared_ptr<const ValueConverterBase> getFor<ValueConverter<false, false, std::list<SRC>, std::list<DST> >, SRC, DST>();
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		return getFor<ValueConverter<false, false, std::list<SRC>, std::list<DST> >, SRC, DST>();
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		std::list<DST> &dstVal = dst.castTo<std::list<DST> >();
		LOG_IF( ! dstVal.empty(), CoreLog, warning )
				<< "Storing into non empty list while conversion from "
				<< Value<std::list<SRC> >::staticName() << " to " << Value<std::list<DST> >::staticName();
		const std::list<SRC> &srcVal = src.castTo<std::list<SRC> >();
		dstVal.resize( srcVal.size() );

		return IterableSubValueConv<SRC, DST >::convertIter2Iter( srcVal, dstVal );
	}
	virtual ~ValueConverter() {}
};
/////////////////////////////////////////////////////////////////////////////
// list to vectorX version -- uses IterableSubValueConv::convertIter2Iter
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST > class ValueConverter<false, false, std::list<SRC>, vector3<DST> >:
	public ValueGenerator<std::list<SRC>, vector3<DST> >, private IterableSubValueConv<SRC, DST >
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating list converter from "
				<< Value<std::list<SRC> >::staticName() << " to " << Value<vector3<DST> >::staticName();
	};
	friend boost::shared_ptr<const ValueConverterBase> getFor<ValueConverter<false, false, std::list<SRC>, vector3<DST> >, SRC, DST>();
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		return getFor<ValueConverter<false, false, std::list<SRC>, vector3<DST> >, SRC, DST>();
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return IterableSubValueConv<SRC, DST >::convertIter2Iter( src.castTo<std::list<SRC> >(), dst.castTo<vector3<DST> >() );
	}
	virtual ~ValueConverter() {}
};
template<typename SRC, typename DST > class ValueConverter<false, false, std::list<SRC>, vector4<DST> >:
	public ValueGenerator<std::list<SRC>, vector4<DST> >, private IterableSubValueConv<SRC, DST >
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating list converter from "
				<< Value<std::list<SRC> >::staticName() << " to " << Value<vector4<DST> >::staticName();
	};
	friend boost::shared_ptr<const ValueConverterBase> getFor<ValueConverter<false, false, std::list<SRC>, vector4<DST> >, SRC, DST>();
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		return getFor<ValueConverter<false, false, std::list<SRC>, vector4<DST> >, SRC, DST>();
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return IterableSubValueConv<SRC, DST >::convertIter2Iter( src.castTo<std::list<SRC> >(), dst.castTo<vector4<DST> >() );
	}
	virtual ~ValueConverter() {}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
// string to scalar -- uses lexical_cast (and in some cases mumeric conversion) to convert from string
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename DST> class ValueConverter<false, false, std::string, DST> : public ValueGenerator<std::string, DST>
{
	ValueConverter() {
		LOG( Debug, verbose_info ) << "Creating from-string converter for " << Value<DST>::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, DST> *ret = new ValueConverter<false, false, std::string, DST>;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return str2scalar( src.castTo<std::string>(), dst.castTo<DST>() );
	}
	virtual ~ValueConverter() {}
};
// cannot use the general str to all because that would be ambiguous with "all to complex"
template<typename DST> class ValueConverter<false, false, std::string, std::complex<DST> > :
	public ValueGenerator<std::string, std::complex<DST> >, private SubValueConv<std::complex<double>, std::complex<DST> >
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating from-string converter for " << Value<std::complex<DST> >::staticName();
	};
	friend boost::shared_ptr<const ValueConverterBase> getFor<ValueConverter<false, false, std::string, std::complex<DST> >, std::complex<double>, std::complex<DST> >();
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		return getFor<ValueConverter<false, false, std::string, std::complex<DST> >, std::complex<double>, std::complex<DST> >();
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		try {
			const util::Value<std::complex<double> > srcDbl( src.castTo<std::string>() ); // make a double from the string
			return num2num( srcDbl.castTo<std::complex<double> >(), dst.castTo<std::complex<DST> >() );
		} catch( const boost::bad_lexical_cast & ) {
			dst.castTo<DST>() = DST();
			LOG( Runtime, error ) << "Miserably failed to interpret " << MSubject( src ) << " as " << Value<DST>::staticName() << " returning " << MSubject( DST() );
		}

		return boost::numeric::cInRange;
	}
	virtual ~ValueConverter() {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// all to string -- just use formatted print into a string buffer
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename SRC> class ValueConverter<false, false, SRC, std::string> : public ValueGenerator<SRC, std::string>
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating to-string converter for " << Value<SRC>::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, SRC, std::string> *ret = new ValueConverter<false, false, SRC, std::string>;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		dst.castTo<std::string>() = toStringConv( src.castTo<SRC>() );
		return boost::numeric::cInRange; // this should allways be ok
	}
	virtual ~ValueConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// string => list/vector/color version -- uses util::stringToList
/////////////////////////////////////////////////////////////////////////////
template<typename DST> class ValueConverter<false, false, std::string, std::list<DST> >:
	public ValueGenerator<std::string, std::list<DST> >
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating from-string converter for " << Value<std::list<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, std::list<DST> > *ret = new ValueConverter<false, false, std::string, std::list<DST> >;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		std::list<DST> &dstList = dst.castTo<std::list<DST> >();
		const std::list<std::string> srcList = Tokenizer<boost::is_arithmetic<DST>::value>::run( src.castTo<std::string>() ); // tokenize the strin based on the target type
		dstList.resize( srcList.size() ); // resize target to the ammount of found tokens
		StrTransformer<DST> transformer; // create a transformer from string to DST
		std::transform( srcList.begin(), srcList.end(), dstList.begin(), transformer ); // transform the found strings to the destination
		return transformer.range_ok;
	}
	virtual ~ValueConverter() {}
};
template<typename DST> class ValueConverter<false, false, std::string, vector4<DST> >: public ValueGenerator<std::string, vector4<DST> >  //string => vector4
{
	ValueConverter() {
		LOG( Debug, verbose_info ) << "Creating from-string converter for " << Value<vector4<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, vector4<DST> > *ret = new ValueConverter<false, false, std::string, vector4<DST> >;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return _internal::convertStr2Vector<DST, 4>( src, dst.castTo<vector4<DST> >() );
	}
	virtual ~ValueConverter() {}
};

template<typename DST> class ValueConverter<false, false, std::string, vector3<DST> >: public ValueGenerator<std::string, vector3<DST> >  //string => vector3
{
	ValueConverter() {
		LOG( Debug, verbose_info ) << "Creating from-string converter for " << Value<vector3<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, vector3<DST> > *ret = new ValueConverter<false, false, std::string, vector3<DST> >;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		return _internal::convertStr2Vector<DST, 3>( src, dst.castTo<vector3<DST> >() );
	}
	virtual ~ValueConverter() {}
};

template<typename T> class ValueConverter<false, false, std::string, color<T> >: public ValueGenerator<std::string, color<T> >  //string => color
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating from-string converter for " << Value<color<T> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, color<T> > *ret = new ValueConverter<false, false, std::string, color<T> >;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		color<T> &dstVal = dst.castTo<color<T> >();
		const std::list<std::string> srcList = Tokenizer<boost::is_arithmetic<T>::value>::run( src.castTo<std::string>() ); // tokenize the string based on the target type
		std::list< std::string >::const_iterator end = srcList.begin();
		std::advance( end, std::min<size_t>( srcList.size(), 3 ) ); // use a max of 3 tokens
		StrTransformer<T> transformer; // create a transformer from string to DST
		std::transform( srcList.begin(), end, &dstVal.r, transformer ); // transform the found strings to the destination
		return transformer.range_ok;
	}
	virtual ~ValueConverter() {}
};

////////////////////////////////////////////////////////////////////////
//OK, thats about the foreplay. Now we get to the dirty stuff.
////////////////////////////////////////////////////////////////////////

///generate a ValueConverter for conversions from SRC to any type from the "types" list
template<typename SRC> struct inner_TypeConverter {
	std::map<int, boost::shared_ptr<const ValueConverterBase> > &m_subMap;
	inner_TypeConverter( std::map<int, boost::shared_ptr<const ValueConverterBase> > &subMap ): m_subMap( subMap ) {}
	template<typename DST> void operator()( DST ) { //will be called by the mpl::for_each in outer_TypeConverter for any DST out of "types"
		//create a converter based on the type traits and the types of SRC and DST
		typedef boost::mpl::and_<boost::is_arithmetic<SRC>, boost::is_arithmetic<DST> > is_num;
		typedef boost::is_same<SRC, DST> is_same;
		boost::shared_ptr<const ValueConverterBase> conv =
			ValueConverter<is_num::value, is_same::value, SRC, DST>::get();
		//and insert it into the to-conversion-map of SRC
		m_subMap.insert( m_subMap.end(), std::make_pair( Value<DST>::staticID, conv ) );
	}
};

///generate a ValueConverter for conversions from any SRC from the "types" list
struct outer_TypeConverter {
	std::map< int , std::map<int, boost::shared_ptr<const ValueConverterBase> > > &m_map;
	outer_TypeConverter( std::map< int , std::map<int, boost::shared_ptr<const ValueConverterBase> > > &map ): m_map( map ) {}
	template<typename SRC> void operator()( SRC ) {//will be called by the mpl::for_each in ValueConverterMap() for any SRC out of "types"
		boost::mpl::for_each<types>( // create a functor for from-SRC-conversion and call its ()-operator for any DST out of "types"
			inner_TypeConverter<SRC>( m_map[Value<SRC>().getTypeID()] )
		);
	}
};

ValueConverterMap::ValueConverterMap()
{
	boost::mpl::for_each<types>( outer_TypeConverter( *this ) );
	LOG( Debug, info ) << "conversion map for " << size() << " types created";
}

}
API_EXCLUDE_END
}
}
/// @endcond
