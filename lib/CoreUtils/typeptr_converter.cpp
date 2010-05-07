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

#include "typeptr_converter.h"

#include "type_converter.hpp"
#include "type_base.hpp"
#include "numeric_convert.hpp"
#include "propmap.hpp" // we must have all types here and PropMap was only forward-declared in types.hpp
#include <boost/mpl/for_each.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/mpl/and.hpp>

/// @cond _internal
namespace isis
{
namespace util
{
namespace _internal
{

//Define generator - this can be global because its using convert internally
template<typename SRC, typename DST> class TypePtrGenerator: public TypePtrConverterBase
{
public:
	void generate( const boost::scoped_ptr<TypePtrBase>& src, boost::scoped_ptr<TypePtrBase>& dst, const TypeBase &min, const TypeBase &max )const {
		LOG_IF( dst.get(), Debug, warning ) <<
											"Generating into existing value " << dst->toString( true );
		TypePtr<DST> *ref = new TypePtr<DST>;
		convert( src->cast_to_TypePtr<SRC>(), *ref, min, max );
		dst.reset( ref );
	}
};
void TypePtrConverterBase::convert( const TypePtrBase &src, TypePtrBase &dst, const TypeBase &min, const TypeBase &max ) const
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
	void convert( const TypePtrBase &src, TypePtrBase &dst, const TypeBase &min, const TypeBase &max )const {
		TypePtr<SRC> &dstVal = dst.cast_to_TypePtr<SRC>();
		const SRC *srcPtr = &src.cast_to_TypePtr<SRC>()[0];
		dstVal.copyFromMem( srcPtr, src.len() );
	}
	virtual ~TypePtrConverter() {}
};

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
	void convert( const TypePtrBase &src, TypePtrBase &dst, const TypeBase &min, const TypeBase &max )const {
		numeric_convert( src.cast_to_TypePtr<SRC>(), dst.cast_to_TypePtr<DST>(), min, max );
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
		boost::mpl::for_each<types>( // create a functor for from-SRC-conversion and call its ()-operator for any DST out of "types"
			inner_TypePtrConverter<SRC>( m_map[TypePtr<SRC>().typeID()] )
		);
	}
};
/// @endcond

TypePtrConverterMap::TypePtrConverterMap()
{
	boost::mpl::for_each<types>( outer_TypePtrConverter( *this ) );
	LOG( Debug, info )
			<< "conversion map for " << size() << " array-types created";
}

}
}
}

