/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  Enrico Reimer <reimer@cbs.mpg.de>

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

// #define BOOST_SPIRIT_DEBUG_PRINT_SOME 100
// #define BOOST_SPIRIT_DEBUG_INDENT 5

#include <boost/foreach.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <CoreUtils/vector.hpp>
#include "imageFormat_nifti_dcmstack.hpp"
#include "imageFormat_nifti_sa.hpp"
#include <CoreUtils/value_base.hpp>
#include <CoreUtils/property.hpp>
#include <limits.h>

namespace isis
{
namespace image_io
{

using boost::posix_time::ptime;
using boost::gregorian::date;

namespace _internal
{

JsonMap::JsonMap( const util::PropertyMap &src ): util::PropertyMap( src ) {}

////////////////////////////////////////////////////////////////////////////////
// low level json parser
////////////////////////////////////////////////////////////////////////////////
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

typedef BOOST_TYPEOF( ascii::space | '\t' | boost::spirit::eol ) SKIP_TYPE;
typedef data::ValueArray< uint8_t >::iterator ch_iterator;
typedef qi::rule<ch_iterator, JsonMap(), SKIP_TYPE>::context_type JsonMapContext;

template<typename T> struct read {typedef qi::rule<ch_iterator, T(), SKIP_TYPE> rule;};

struct add_member {
	char extra_token;
	add_member( char _extra_token ): extra_token( _extra_token ) {}
	void operator()( const fusion::vector2<std::string, JsonMap::value_cont> &a, JsonMapContext &context, bool & )const {
		const JsonMap::PropPath label = extra_token ?
										util::stringToList<JsonMap::KeyType>( a.m0, extra_token ) :
										JsonMap::PropPath( a.m0.c_str() );
		JsonMap &target = context.attributes.car;

		if( target.hasBranch( label ) || target.hasProperty( label ) )
			LOG( Runtime, error ) << "There is already an entry " << target << " skipping this one" ;
		else
			target.insertObject( label, a.m1 );
	}
};

void JsonMap::insertObject( const JsonMap::PropPath &label, const JsonMap::value_cont &container )
{
	switch( container.which() ) {
	case 2: {
		const std::list<util::PropertyValue> &src = boost::get<std::list<util::PropertyValue> >( container );
		propertyValueVec( label ) = std::vector<util::PropertyValue>( src.begin(), src.end() );
	}
	break;
	case 1:
		branch( label ) = boost::get<JsonMap>( container );
		break;
	case 0:
		propertyValue( label ) = boost::get<util::PropertyValue>( container );
		break;
	}
}

bool JsonMap::readJson( data::ValueArray< uint8_t > stream, char extra_token )
{
	using qi::lit;
	using namespace boost::spirit;

	int version;
	read<util::PropertyValue>::rule value;
	read<fusion::vector2<std::string, value_cont> >::rule member;

	read<std::string>::rule string( lexeme['"' >> *( ascii::print - '"' ) >> '"'], "string" ), label( string >> ':', "label" );
	read<int>::rule integer( int_ >> !lit( '.' ), "integer" ) ; // an integer followed by a '.' is not an integer
	read<util::dlist>::rule dlist( lit( '[' ) >> double_ % ',' >> ']', "dlist" );
	read<util::ilist>::rule ilist( lit( '[' ) >> integer % ',' >> ']', "ilist" );
	read<util::slist>::rule slist( lit( '[' ) >> string  % ',' >> ']', "slist" );
	read<std::list<util::PropertyValue> >::rule vallist( lit( '[' ) >> value % ',' >> ']', "value_list" );

	read<JsonMap>::rule
	object( lit( '{' ) >> ( member[add_member( extra_token )] % ',' || eps ) >> '}',                    "object" ),
			list_object( lit( '{' ) >> ( ( label >> vallist )[add_member( extra_token )] % ',' || eps ) >> '}', "list_object" );

	member = //samples and slices are expected to consist only of json-arrays
		lexeme['"' >> ascii::string( "samples" ) >> '"'] >> ':' >> list_object |
		lexeme['"' >> ascii::string( "slices" ) >> '"'] >> ':' >> list_object |
		label >> ( value | vallist | object );
	value = integer | double_ | string | ilist | dlist | slist;

	data::ValueArray< uint8_t >::iterator begin = stream.begin(), end = stream.end();
	bool erg = phrase_parse( begin, end, object[phoenix::ref( *this ) = _1], ascii::space | '\t' | eol );
	return end == stream.end();
}

////////////////////////////////////////////////////////////////////////////////
// high level translation to ISIS properties
////////////////////////////////////////////////////////////////////////////////

// some basic functors for later use
struct Plus {
	util::PropertyValue operator()( double other, const util::PropertyValue &val )const {return util::Value<float>( other + val.as<float>() );}
	util::PropertyValue operator()( const util::PropertyValue &val )const {return util::Value<float>( fixedAcq + val.as<float>() );}
	float fixedAcq;
};

struct ComputeTimeDist {
	boost::posix_time::ptime sequenceStart;
	util::PropertyValue operator()( const util::PropertyValue &val )const {
		const boost::posix_time::time_duration acDist = val.castTo<boost::posix_time::ptime>() - sequenceStart;
		return float( acDist.ticks() ) / acDist.ticks_per_second() * 1000;
	}
};

struct TmParser {
	util::PropertyValue operator()( const util::PropertyValue &val )const {
		short shift = 0;
		bool ok = true;
		boost::posix_time::time_duration ret;
		std::string time_str = val.as<std::string>();

		// Insert the ":" -- make it hh:mm:ss.frac
		if ( time_str.at( 2 ) != ':' ) {
			time_str.insert( 2, 1, ':' );
			shift++;
		}

		if ( ( time_str.size() > size_t( 4 + shift ) ) && ( time_str.at( 4 + shift ) != ':' ) ) {
			time_str.insert( 4 + shift, 1, ':' );
			shift++;
		}

		//Try standard-parser for hh:mm:ss.frac
		try {
			ret = boost::posix_time::duration_from_string( time_str.c_str() );
			ok = not ret.is_not_a_date_time();
		} catch ( std::logic_error e ) {
			ok = false;
		}

		if ( ok ) {
			LOG( Debug, verbose_info ) << "Parsed time " <<  time_str << " as " << ret;
			return ptime( date( 1400, 1, 1 ), ret );
			//because TM is defined as time of day we dont have a day here, so we fake one
		} else
			LOG( Runtime, warning ) << "Cannot parse Time from \"" << val << "\"";

		return val;
	}
};

std::list< data::Chunk > JsonMap::translateToISIS( data::Chunk orig )
{
	// translate some of the entries and clean up the tree we got from DcmMeta (don't touch anything we got from the normal header though)
	size_t dosplice = false;

	if( hasBranch( "time" ) ) { //store pervolume data (samples and slices) prior to possible splicing as DICOM
		util::PropertyMap &time = branch( "time" );
		//get "samples" and "slices" into DICOM
		branch( "DICOM" ).join( time.branch( "samples" ) ); //@todo cleanup when we have moving join
		branch( "DICOM" ).join( time.branch( "slices" ) );
		remove( "time/samples" );
		remove( "time/slices" );
	}

	if( hasBranch( "global/const" ) ) { //store const data prior to possible splicing as DICOM
		branch( "DICOM" ).join( branch( "global/const" ) );
	}

	remove( "global/const" );

	if( hasBranch( "global/slices" ) ) { //store slice data prior to possible splicing as DICOM
		branch( "DICOM" ).join( branch( "global/slices" ) );
	}

	remove( "global/slices" );

	// if we have a DICOM/AcquisitionNumber-list or DICOM/InstanceNumber rename that to acquisitionNumber and splice the chunk in necessary
	if( hasProperty( "DICOM/InstanceNumber" ) ) {
		rename( "DICOM/InstanceNumber", "acquisitionNumber" );
		dosplice |= propertyValueVec( "acquisitionNumber" ).size() > 1;
	} else if( hasProperty( "DICOM/AcquisitionNumber" ) ) {
		rename( "DICOM/AcquisitionNumber", "acquisitionNumber" );
		dosplice |= propertyValueVec( "acquisitionNumber" ).size() > 1;
	}

	//translate TM sets we know about to proper Timestamps
	const char *TMs[] = {"DICOM/ContentTime", "DICOM/AcquisitionTime"};
	BOOST_FOREACH( const util::PropertyMap::PropPath tm, TMs ) {
		if( hasProperty( tm ) ) {
			std::vector< util::PropertyValue > &v = propertyValueVec( tm );
			std::transform( v.begin(), v.end(), v.begin(), TmParser() );
			dosplice |= v.size() > 1;
		}
	}


	// compute acquisitionTime as relative to DICOM/SeriesTime @todo include date
	const char *time_stor[] = {"DICOM/ContentTime", "DICOM/AcquisitionTime"};
	BOOST_FOREACH( const char * time, time_stor ) {
		const TmParser p;
		const ComputeTimeDist comp = {p( propertyValue( "DICOM/SeriesTime" ) ).castTo<ptime>()};

		if( hasProperty( time ) && hasProperty( "DICOM/SeriesTime" ) ) {
			std::vector< util::PropertyValue > &dst = propertyValueVec( "acquisitionTime" );
			const std::vector< util::PropertyValue > &src = propertyValueVec( time );
			dst.resize( src.size() );
			std::transform( src.begin(), src.end(), dst.begin(), comp );
			remove( time );
			break;
		}
	}

	// deal with mosaic
	bool is_mosaic = false;

	if( hasProperty( "DICOM/ImageType" ) ) {
		util::slist &iType = propertyValue( "DICOM/ImageType" ).castTo<util::slist>();
		is_mosaic = ( std::find( iType.begin(), iType.end(), std::string( "MOSAIC" ) ) != iType.end() );
	}

	if( is_mosaic ) {
		decodeMosaic();
	} else if( hasProperty( "DICOM/ImagePositionPatient" ) ) { //if it wasn't a mosaic, we may actually have a proper ImagePositionPatient-list
		dosplice |= propertyValueVec( "DICOM/ImagePositionPatient" ).size() > 1;
	}

	// compute voxelGap (must be done after mosaic because it removes SpacingBetweenSlices)
	if ( hasProperty( "DICOM/SliceThickness" ) && hasProperty( "DICOM/SpacingBetweenSlices" ) ) {
		const float gap = getPropertyAs<float>( "DICOM/SpacingBetweenSlices" ) - getPropertyAs<float>( "DICOM/SliceThickness" );

		if( gap )setPropertyAs( "voxelGap", util::fvector3( 0, 0, gap ) );

		remove( "DICOM/SpacingBetweenSlices" );
	}

	// flatten matrizes
	const char *matrizes[] = {"dcmmeta_affine", "dcmmeta_reorient_transform"};
	BOOST_FOREACH( util::istring matrix, matrizes ) {
		int cnt = 0;
		BOOST_FOREACH( const util::PropertyValue & val, propertyValueVec( matrix ) ) {
			setPropertyAs( matrix + "[" + boost::lexical_cast<util::istring>( cnt++ ) + "]", val.as<util::fvector4>() );
		}
		remove( matrix );
	}

	orig.join( *this, true );

	std::list< data::Chunk > ret = ( dosplice ? orig.autoSplice( 1 ) : std::list<data::Chunk>( 1, orig ) ); //splice at (probably) timedim

	return ret;
}

void JsonMap::decodeMosaic()
{
	// prepare variables
	const util::PropertyMap::KeyType mosaicTimes = find( "MosaicRefAcqTimes" );
	static const util::PropertyMap::PropPath NumberOfImagesInMosaicProp =  "DICOM/CSAImage/NumberOfImagesInMosaic";
	static const util::PropertyMap::PropPath MosaicOrigin =  "DICOM/ImagePositionPatient";

	// if we have geometric data
	if(
		hasProperty( NumberOfImagesInMosaicProp ) && hasProperty( NumberOfImagesInMosaicProp ) &&
		hasProperty( "DICOM/Columns" ) && hasProperty( "DICOM/Rows" ) &&
		hasProperty( "DICOM/SliceThickness" ) && hasProperty( "DICOM/PixelSpacing" ) &&
		propertyValueVec( MosaicOrigin ).size() == 1
	) {
		// All is fine, lets start
		uint16_t slices = getPropertyAs<uint16_t>( NumberOfImagesInMosaicProp );
		const uint16_t matrixSize = std::ceil( std::sqrt( slices ) );
		const util::vector3<size_t> size( getPropertyAs<uint64_t>( "DICOM/Columns" ) / matrixSize, getPropertyAs<uint64_t>( "DICOM/Rows" ) / matrixSize, slices );
		const util::dlist orientation = getPropertyAs<util::dlist>( "DICOM/ImageOrientationPatient" );
		util::dlist::const_iterator middle = orientation.begin();
		std::advance( middle, 3 );
		util::fvector3 rowVec, columnVec;
		rowVec.copyFrom( orientation.begin(), middle );
		columnVec.copyFrom( middle, orientation.end() );

		// fix the properties of the source (we 'll need them later)
		util::fvector3 voxelSize = getPropertyAs<util::fvector3>( "DICOM/PixelSpacing" );
		voxelSize[2] = getPropertyAs<float>( "DICOM/SpacingBetweenSlices" );
		//remove the additional mosaic offset
		//eg. if there is a 10x10 Mosaic, substract the half size of 9 Images from the offset
		const util::fvector3 fovCorr = ( voxelSize ) * size * ( matrixSize - 1 ) / 2;
		util::fvector3 origin = getPropertyAs<util::fvector3>( MosaicOrigin );
		origin += ( rowVec * fovCorr[0] ) + ( columnVec * fovCorr[1] );
		remove( NumberOfImagesInMosaicProp ); // we dont need that anymore

		// store the proper origin
		propertyValue( MosaicOrigin ) = util::Value<util::fvector3>( origin );

		// replace "MOSAIC" ImageType by "WAS_MOSAIC"
		util::slist &iType = propertyValue( "DICOM/ImageType" ).castTo<util::slist>();
		std::replace( iType.begin(), iType.end(), std::string( "MOSAIC" ), std::string( "WAS_MOSAIC" ) );
	} else {
		LOG( Runtime, error ) << "Failed to decode mosaic geometry data, won't touch " << MosaicOrigin;
	}

	//flatten MosaicRefAcqTimes and add it to acquisitionTime
	if( mosaicTimes != "" ) { // if there are MosaicRefAcqTimes recompute acquisitionTime

		std::vector< util::PropertyValue > &acq = propertyValueVec( "acquisitionTime" );
		const std::vector< util::PropertyValue > &mos = propertyValueVec( mosaicTimes );

		if( !acq.front().isEmpty() && ( acq.front().is<util::ilist>() || acq.front().is<util::dlist>() || acq.front().is<util::slist>() ) ) {
			LOG( Runtime, warning ) << "There is already an acquisitionTime for each slice, won't recompute it from " << mosaicTimes;
		} else if( !acq.front().isEmpty() && mos.size() != acq.size() ) {
			LOG( Runtime, warning ) << "The list size of " << mosaicTimes << "(" << mos.size() << ") and acquisitionTime (" << acq.size() << ") don't match, won't touch them";
		} else {
			std::vector< util::PropertyValue > old_acq;

			if( acq.size() == mos.size() )
				old_acq = acq;

			size_t slices = mos.front().as<util::dlist>().size() * mos.size();
			acq.resize( slices );
			std::vector< util::PropertyValue >::iterator acq_iter = acq.begin();

			for( size_t i = 0; i < mos.size(); i++ ) {
				const util::dlist inner_mos = mos[i].as<util::dlist>();
				const Plus adder = {old_acq.empty() ? 0 : old_acq[i].as<float>()};
				acq_iter = std::transform( inner_mos.begin(), inner_mos.end(), acq_iter, adder );
			}

			assert( acq_iter == acq.end() ); // new acq should be filled completely

			remove( mosaicTimes ); // remove the original prop if the distribution was successful
		}
	}
}

}

}
}
