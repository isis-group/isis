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

#include "type_converter.hpp"
#include "type_base.hpp"
#include "type.hpp"
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
namespace _internal
{

//Define generator - this can be global because its using convert internally
template<typename SRC, typename DST> class ValueGenerator: public ValueConverterBase
{
public:
	void create( boost::scoped_ptr<ValueBase>& dst )const {
		LOG_IF( dst.get(), Debug, warning ) <<
											"Generating into existing value " << dst->toString( true ) << " (dropping this).";
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
/*	static boost::shared_ptr<const ValueConverterBase> get() {
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
// Numeric version -- uses boost::numeric_cast
/////////////////////////////////////////////////////////////////////////////
struct NumericOverflowHandler { //@todo this is NOT thread-safe
	static boost::numeric::range_check_result result;
	void operator() ( boost::numeric::range_check_result r ) { // throw bad_numeric_conversion derived
		result = r;
	}
};
boost::numeric::range_check_result NumericOverflowHandler::result = boost::numeric::cInRange;

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
		typedef boost::numeric::converter <
		DST, SRC,
		   boost::numeric::conversion_traits<DST, SRC>,
		   NumericOverflowHandler,
		   boost::numeric::RoundEven<SRC>
		   > converter;
		DST &dstVal = dst.castTo<DST>();
		const SRC &srcVal = src.castTo<SRC>();
		NumericOverflowHandler::result = boost::numeric::cInRange;
		dstVal = converter::convert( srcVal );
		return NumericOverflowHandler::result;
	}
	virtual ~ValueConverter() {}
};

///////////////////////////////////////////////////////////////////////////////
// Conversion for complex numbers
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
		typedef boost::numeric::converter <
			std::complex<DST> , std::complex<SRC> ,
			boost::numeric::conversion_traits<std::complex<DST>, std::complex<SRC> >,
			NumericOverflowHandler,
			boost::numeric::RoundEven<std::complex<SRC> >
		> converter;
		NumericOverflowHandler::result = boost::numeric::cInRange;
		const std::complex<SRC> &srcVal = src.castTo<std::complex<SRC> >();
		std::complex<DST> &dstVal = dst.castTo<std::complex<DST> >();

		dstVal = converter::convert( srcVal );
		return NumericOverflowHandler::result;
	}
	virtual ~ValueConverter() {}
};

///////////////////////////////////////////////////////////////////////////////
// Conversion for "normal" to complex numbers -- uses ValueConverter on the real part
/////////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST> class ValueConverter<false, false, SRC, std::complex<DST> > : public ValueGenerator<SRC, std::complex<DST> >
{
	boost::shared_ptr<const ValueConverterBase> m_conv;
	ValueConverter( boost::shared_ptr<const ValueConverterBase> elem_conv ): m_conv( elem_conv ) {
		LOG( Debug, verbose_info )
		<< "Creating number-complex converter from "
		   << Value<SRC>::staticName() << " to " << Value<std::complex<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		typedef boost::mpl::and_<boost::is_arithmetic<SRC>, boost::is_arithmetic<DST> > is_num;
		typedef boost::is_same<SRC, DST> is_same;
		boost::shared_ptr<const ValueConverterBase> elem_conv =
		ValueConverter<is_num::value, is_same::value, SRC, DST>::get();

		if ( elem_conv ) { // if there is a conversion from SRC to DST create a conversion SRC => complex<DST> using that
			ValueConverter<false, false, SRC, std::complex<DST> > *ret = new ValueConverter<false, false, SRC, std::complex<DST> >( elem_conv );
			return boost::shared_ptr<const ValueConverterBase>( ret );
		} else {
			return boost::shared_ptr<const ValueConverterBase>();
		}
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		std::complex<DST> &dstVal = dst.castTo<std::complex<DST> >();
		Value<DST> real;
		boost::numeric::range_check_result ret = m_conv->convert( src, real );
		if(ret==boost::numeric::cInRange)
			dstVal=std::complex<DST>((DST)real,DST());

		return NumericOverflowHandler::result;
	}
	virtual ~ValueConverter() {}
};


/////////////////////////////////////////////////////////////////////////////
// vector4 version -- uses ValueConverter on every element
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST > class ValueConverter<false, false, vector4<SRC>, vector4<DST> >: public ValueGenerator<vector4<SRC>, vector4<DST> >
{
	boost::shared_ptr<const ValueConverterBase> m_conv;
	ValueConverter( boost::shared_ptr<const ValueConverterBase> elem_conv ): m_conv( elem_conv ) {
		LOG( Debug, verbose_info )
				<< "Creating vector converter from "
				<< Value<vector4<SRC> >::staticName() << " to " << Value<vector4<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		typedef boost::mpl::and_<boost::is_arithmetic<SRC>, boost::is_arithmetic<DST> > is_num;
		typedef boost::is_same<SRC, DST> is_same;
		boost::shared_ptr<const ValueConverterBase> elem_conv =
			ValueConverter<is_num::value, is_same::value, SRC, DST>::get();

		if ( elem_conv ) {
			ValueConverter<false, false, vector4<SRC>, vector4<DST> > *ret = new ValueConverter<false, false, vector4<SRC>, vector4<DST> >( elem_conv );
			return boost::shared_ptr<const ValueConverterBase>( ret );
		} else {
			return boost::shared_ptr<const ValueConverterBase>();
		}
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		vector4<DST> &dstVal = dst.castTo<vector4<DST> >();
		const vector4<SRC> &srcVal = src.castTo<vector4<SRC> >();
		boost::numeric::range_check_result ret = boost::numeric::cInRange;

		for ( int i = 0; i < 4; i++ ) {//slow and ugly, but flexible
			Value<DST> elem_dst;
			const boost::numeric::range_check_result result = m_conv->convert( Value<SRC>( srcVal[i] ), elem_dst );

			if ( result != boost::numeric::cInRange )
				ret = result;

			dstVal[i] = ( DST )elem_dst;
		}

		return ret;
	}
	virtual ~ValueConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// list version -- uses ValueConverter on every element
/////////////////////////////////////////////////////////////////////////////
template<typename SRC, typename DST > class ValueConverter<false, false, std::list<SRC>, std::list<DST> >: public ValueGenerator<std::list<SRC>, std::list<DST> >
{
	boost::shared_ptr<const ValueConverterBase> m_conv;
	ValueConverter( boost::shared_ptr<const ValueConverterBase> elem_conv ): m_conv( elem_conv ) {
		LOG( Debug, verbose_info )
				<< "Creating list converter from "
				<< Value<std::list<SRC> >::staticName() << " to " << Value<std::list<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		typedef boost::mpl::and_<boost::is_arithmetic<SRC>, boost::is_arithmetic<DST> > is_num;
		typedef boost::is_same<SRC, DST> is_same;
		boost::shared_ptr<const ValueConverterBase> elem_conv =
			ValueConverter<is_num::value, is_same::value, SRC, DST>::get();

		if ( elem_conv ) {
			ValueConverter<false, false, std::list<SRC>, std::list<DST> > *ret = new ValueConverter<false, false, std::list<SRC>, std::list<DST> >( elem_conv );
			return boost::shared_ptr<const ValueConverterBase>( ret );
		} else {
			return boost::shared_ptr<const ValueConverterBase>();
		}
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		std::list<DST> &dstVal = dst.castTo<std::list<DST> >();
		LOG_IF( ! dstVal.empty(), CoreLog, warning )
				<< "Storing into non empty list while conversion from "
				<< Value<std::list<SRC> >::staticName() << " to " << Value<std::list<DST> >::staticName();
		const std::list<SRC> &srcVal = src.castTo<std::list<SRC> >();
		boost::numeric::range_check_result ret = boost::numeric::cInRange;

		for ( typename std::list<SRC>::const_iterator i = srcVal.begin(); i != srcVal.end(); i++ ) {//slow and ugly, but flexible
			Value<DST> elem_dst;
			const boost::numeric::range_check_result result = m_conv->convert( Value<SRC>( *i ), elem_dst );

			if ( result != boost::numeric::cInRange )
				ret = result;

			dstVal.push_back( ( DST )elem_dst );
		}

		return ret;
	}
	virtual ~ValueConverter() {}
};


/////////////////////////////////////////////////////////////////////////////
// string version -- uses lexical_cast to convert from/to string
/////////////////////////////////////////////////////////////////////////////
template<typename DST> class ValueConverter<false, false, std::string, DST> : public ValueGenerator<std::string, DST>
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating from-string converter for " << Value<DST>::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, DST> *ret = new ValueConverter<false, false, std::string, DST>;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		DST &dstVal = dst.castTo<DST>();
		const std::string &srcVal = src.castTo<std::string>();
		dstVal = boost::lexical_cast<DST>( srcVal );
		return boost::numeric::cInRange; //@todo handle bad casts
	}
	virtual ~ValueConverter() {}
};
template<typename DST> class ValueConverter<false, false, std::string, std::complex<DST> > : public ValueGenerator<std::string, std::complex<DST> >
{
	ValueConverter() {
		LOG( Debug, verbose_info )
		<< "Creating from-string converter for " << Value<std::complex<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, std::complex<DST> > *ret = new ValueConverter<false, false, std::string, std::complex<DST> >;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		std::complex<DST> &dstVal = dst.castTo<std::complex<DST> >();
		const std::string &srcVal = src.castTo<std::string>();
		dstVal = boost::lexical_cast<std::complex<DST> >( srcVal );
		return boost::numeric::cInRange; //@todo handle bad casts
	}
	virtual ~ValueConverter() {}
};
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
		std::string &dstVal = dst.castTo<std::string>();
		const SRC &srcVal = src.castTo<SRC>();
		dstVal = boost::lexical_cast<std::string, SRC>( srcVal );
		return boost::numeric::cInRange; // this should allways be ok
	}
	virtual ~ValueConverter() {}
};

/////////////////////////////////////////////////////////////////////////////
// special version to convert from string to a given selection
// (lexical_cast doesnt work here, because it creates a temporary buffer)
/////////////////////////////////////////////////////////////////////////////
template<> class ValueConverter<false, false, std::string, Selection> : public ValueGenerator<std::string, Selection>
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating special from-string converter for " << Value<Selection>::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, Selection> *ret = new ValueConverter<false, false, std::string, Selection>;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		Selection &dstVal = dst.castTo<Selection>();
		const std::string &srcVal = src.castTo<std::string>();

		if ( dstVal.set( srcVal.c_str() ) )
			return boost::numeric::cInRange;
		else
			return boost::numeric::cPosOverflow; //if the string is not "part" of the selection we count this as positive overflow
	}
	virtual ~ValueConverter() {}
};


/////////////////////////////////////////////////////////////////////////////
// string/bool version -- uses decision based on text
/////////////////////////////////////////////////////////////////////////////
template<> class ValueConverter<false, false, std::string, bool> : public ValueGenerator<std::string, bool>
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating special from-string converter for " << Value<bool>::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, bool> *ret = new ValueConverter<false, false, std::string, bool>;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		bool &dstVal = dst.castTo<bool>();
		const std::string srcVal = boost::algorithm::to_lower_copy<std::string>( src.castTo<std::string>() );

		if (  srcVal == "true" || srcVal == "y" || srcVal == "yes" ) {
			dstVal = true;
		} else if ( srcVal == "false" || srcVal == "n" || srcVal == "no" ) {
			dstVal = false;
		} else {
			LOG( Runtime, warning ) << src.toString( true ) << " is ambiguous while converting to " << Value<bool>::staticName();
			return boost::numeric::cPosOverflow;
		}

		return boost::numeric::cInRange;
	}
	virtual ~ValueConverter() {}
};
template<> class ValueConverter<false, false, bool, std::string> : public ValueGenerator<bool, std::string>
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating special to-string converter for " << Value<bool>::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, bool, std::string> *ret = new ValueConverter<false, false, bool, std::string>;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		std::string &dstVal = dst.castTo<std::string>();
		const bool &srcVal = src.castTo<bool>();
		dstVal = srcVal ? "true" : "false";
		return boost::numeric::cInRange;
	}
	virtual ~ValueConverter() {}
};


/////////////////////////////////////////////////////////////////////////////
// string => list/vector version -- uses util::stringToList
/////////////////////////////////////////////////////////////////////////////
template<typename DST> class ValueConverter<false, false, std::string, std::list<DST> >: public ValueGenerator<std::string, std::list<DST> >  //string => list
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
		std::list<DST> &dstVal = dst.castTo<std::list<DST> >();
		LOG_IF( ! dstVal.empty(), CoreLog, warning )
				<< "Conversion from " << Value<std::string>::staticName()
				<< " into non empty list  " << Value<std::list<DST> >::staticName()
				<< " previous content will be lost";
		const std::string &srcVal = src.castTo<std::string>();
		const std::list<DST> buff = util::stringToList<DST>( srcVal, boost::regex( "[\\s,;]+" ) );
		dstVal.assign( buff.begin(), buff.end() );
		return boost::numeric::cInRange;  //@todo handle bad casts
	}
	virtual ~ValueConverter() {}
};
template<typename DST> class ValueConverter<false, false, std::string, vector4<DST> >: public ValueGenerator<std::string, vector4<DST> >  //string => vector4
{
	ValueConverter() {
		LOG( Debug, verbose_info )
				<< "Creating from-string converter for " << Value<vector4<DST> >::staticName();
	};
public:
	static boost::shared_ptr<const ValueConverterBase> get() {
		ValueConverter<false, false, std::string, vector4<DST> > *ret = new ValueConverter<false, false, std::string, vector4<DST> >;
		return boost::shared_ptr<const ValueConverterBase>( ret );
	}
	boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const {
		vector4<DST> &dstVal = dst.castTo<vector4<DST> >();
		const std::string &srcVal = src.castTo<std::string>();
		const std::list<DST> buff = stringToList<DST>( srcVal, boost::regex( "[\\s,;]+" ) );
		dstVal.copyFrom( buff.begin(), buff.end() );
		return boost::numeric::cInRange; //@todo handle bad casts
	}
	virtual ~ValueConverter() {}
};


// @todo we cannot parse this stuff yet
template<> class ValueConverter<false, false, std::string, color24 >: public ValueGenerator<std::string, color24 >  //string => color
{
public:
	virtual ~ValueConverter() {}
};
template<> class ValueConverter<false, false, std::string, color48 >: public ValueGenerator<std::string, color48 >  //string => color
{
public:
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
/// @endcond

ValueConverterMap::ValueConverterMap()
{
	boost::mpl::for_each<types>( outer_TypeConverter( *this ) );
	LOG( Debug, info ) << "conversion map for " << size() << " types created";
}

}
}
}
