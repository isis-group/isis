/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

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

#include "imageFormat_Vista_sa.hpp"
#include "VistaSaParser.hpp"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace isis
{

namespace image_io
{

namespace _internal
{
template<typename T> isis::data::ValueArrayReference reader( isis::data::FilePtr data, size_t offset, size_t size )
{
	return data.atByID( isis::data::ValueArray<T>::staticID, offset, size, true );
}

template<> isis::data::ValueArrayReference reader<uint8_t>( isis::data::FilePtr data, size_t offset, size_t size )
{
	return data.atByID( isis::data::ValueArray<uint8_t>::staticID, offset, size, false );
}

template<> isis::data::ValueArrayReference reader<bool>( isis::data::FilePtr data, size_t offset, size_t size )
{
	return reader< uint8_t >( data, offset, size )->as<bool>(); //@todo check if scaling is computed
}

template<typename T> data::ValueArray<util::color<T> > toColor( const data::ValueArrayReference ref, size_t slice_size )
{
	assert( ref->getLength() % 3 == 0 );
	const std::vector< data::ValueArrayReference > layers = ref->as<T>().splice( slice_size ); //colors are stored slice-wise in the 3d block
	data::ValueArray<util::color<T> > ret( ref->getLength() / 3 );

	typename data::ValueArray<util::color<T> >::iterator d_pix = ret.begin();

	for( std::vector< data::ValueArrayReference >::const_iterator l = layers.begin(); l != layers.end(); ) {
		BOOST_FOREACH( T s_pix, ( *l++ )->castToValueArray<T>() )( *( d_pix++ ) ).r = s_pix; //red
		d_pix -= slice_size; //return to the slice start
		BOOST_FOREACH( T s_pix, ( *l++ )->castToValueArray<T>() )( *( d_pix++ ) ).g = s_pix; //green
		d_pix -= slice_size; //return to the slice start
		BOOST_FOREACH( T s_pix, ( *l++ )->castToValueArray<T>() )( *( d_pix++ ) ).b = s_pix; //blue
	}

	return ret;
}

}

data::Chunk ImageFormat_VistaSa::makeChunk( isis::data::FilePtr data, isis::data::ValueArray< uint8_t >::iterator data_start, const isis::util::PropertyMap &props )
{
	const size_t ch_offset = props.getPropertyAs<uint64_t>( "data" );
	isis::util::vector4<size_t> ch_size(
		props.getPropertyAs<uint32_t>( "ncolumns" ),
		props.getPropertyAs<uint32_t>( "nrows" ),
		props.getPropertyAs<uint32_t>( "nbands" ),
		1
	);

	readerPtr reader = vista2isis[props.getPropertyAs<std::string>( "repn" ).c_str()];

	if( !reader )
		throwGenericError( std::string( "Cannot handle repn " ) + props.getPropertyAs<std::string>( "repn" ) );

	data::ValueArrayReference ch_data = reader( data, std::distance( data.begin(), data_start ) + ch_offset, ch_size.product() );

	if( props.hasProperty( "component_repn" ) && props.getPropertyAs<std::string>( "component_repn" ) == "rgb" ) { // if its color
		ch_data = _internal::toColor<uint8_t>( ch_data, ch_size.product() / ch_size[data::sliceDim] );
		ch_size[data::sliceDim] /= 3;
	}

	LOG( Runtime, verbose_info )
			<< "Creating " << ch_data->getTypeName() << "-Chunk of size "
			<< ch_size << " and range " << ch_data->getMinMax().first << "/" << ch_data->getMinMax().second << " (offset was " << std::hex << std::distance( data.begin(), data_start ) + ch_offset << "/" << std::dec << std::distance( data_start + ch_offset + ch_data->getLength()*ch_data->bytesPerElem(), data.end() ) << " bytes are left)";

	isis::data::Chunk ret = isis::data::Chunk( ch_data, ch_size[0], ch_size[1], ch_size[2], ch_size[3] );

	ret.branch( "vista" ) = props;
	sanitize( ret );

	return ret;
}

void ImageFormat_VistaSa::sanitize( util::PropertyMap &obj )
{
	obj.remove( "vista/data" );
	obj.remove( "vista/ncolumns" );
	obj.remove( "vista/nrows" );
	obj.remove( "vista/nbands" );
	obj.remove( "vista/repn" );

	if( obj.hasProperty( "vista/columnVec" ) && obj.hasProperty( "vista/rowVec" ) ) {
		obj.transform<isis::util::fvector4>( "vista/columnVec", "rowVec" );
		obj.transform<isis::util::fvector4>( "vista/rowVec", "columnVec" );
		transformOrTell<isis::util::fvector4>( "vista/sliceVec", "sliceVec", obj, warning );
	} else {
		obj.setPropertyAs( "rowVec",    util::fvector4( 1, 0 ) );
		obj.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
		obj.setPropertyAs( "sliceVec",  util::fvector4( 0, 0, -1 ) );
		LOG( Runtime, warning ) << "No orientation info was found, assuming identity matrix";
	}

	if( !transformOrTell<isis::util::fvector4>( "vista/indexOrigin", "indexOrigin", obj, warning ) ) {
		obj.setPropertyAs( "indexOrigin", util::fvector4( 0, 0 ) );
	}

	if( !transformOrTell<isis::util::fvector4>( "vista/voxel", "voxelSize", obj, warning ) ) {
		obj.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1 ) );
	}

	if( obj.hasProperty( "vista/diffusionBValue" ) ) {
		const float len = obj.getPropertyAs<float>( "vista/diffusionBValue" );

		if( len > 0 && transformOrTell<isis::util::fvector4>( "vista/diffusionGradientOrientation", "diffusionGradient", obj, warning ) ) {
			util::fvector4 &vec = obj.propertyValue( "diffusionGradient" ).castTo<util::fvector4>();
			vec.norm();
			vec *= len;
			obj.remove( "vista/diffusionBValue" );
		}
	}

	transformOrTell<std::string>( "vista/name", "sequenceDescription", obj, warning );
	transformOrTell<std::string>( "vista/patient", "subjectName", obj, warning );

	if( obj.hasProperty( "vista/sex" ) ) {
		util::Selection gender( "male,female,other" );
		gender.set( obj.getPropertyAs<std::string>( "vista/sex" ).c_str() );

		if( ( int )gender ) {
			obj.setPropertyAs( "subjectGender", gender );
			obj.remove( "vista/sex" );
		}
	}

	//  if(obj.hasProperty("vista/date") && obj.hasProperty("vista/time")){
	//      boost::posix_time::ptime stamp=boost::posix_time::time_from_string(obj.getPropertyAs<std::string>("vista/date") + " " + obj.getPropertyAs<std::string>("vista/time"));
	//      obj.setPropertyAs("hasProperty",stamp);
	//      obj.remove("vista/date");
	//      obj.remove("vista/time");
	//  }

}

bool ImageFormat_VistaSa::isFunctional( const std::list< data::Chunk >& chunks )
{
	//if we have only one chunk, its not functional
	if( chunks.size() <= 1 )return false;

	const data::Chunk &front = chunks.front();
	const bool is_short = front.is<int16_t>();

	if( front.hasProperty( "vista/diffusionGradientOrientation" ) )
		return false; // dwi data look the same, but must not be transformed

	//if we have proper vista/ntimesteps its functional
	if( front.hasProperty( "vista/ntimesteps" ) && front.getPropertyAs<uint32_t>( "vista/ntimesteps" ) == front.getSizeAsVector()[data::sliceDim] ) {
		LOG_IF( is_short, Runtime, warning ) << "Functional data found which is not VShort";
		return true;
	} else if( is_short ) { //if we have short, its functional
		LOG( Runtime, warning ) << "Functional data found, but ntimesteps is not set or wrong (should be " << front.getSizeAsVector()[data::sliceDim] << ")";
		return true;
	}

	return  false;
}


std::list< data::Chunk > ImageFormat_VistaSa::transformFunctional( const std::list< data::Chunk >& in_chunks )
{
	LOG( Runtime, info ) << "Transforming " << in_chunks.size() << " chunks for fMRI";
	std::list< data::Chunk > ret;
	const uint32_t acqStride = in_chunks.size();
	BOOST_FOREACH( data::Chunk ch, in_chunks ) {
		size_t timestep = 0;
		ch.remove( "vista/ntimesteps" );
		BOOST_FOREACH( data::Chunk ch, ch.splice( data::sliceDim ) ) {
			uint32_t &acq = ch.propertyValue( "acquisitionNumber" ).castTo<uint32_t>();
			acq = acq + acqStride * timestep;
			ret.push_back( ch );
			timestep++;
		}
	}
	return ret;
}


ImageFormat_VistaSa::ImageFormat_VistaSa()
{
	vista2isis["bit"] =   _internal::reader<bool>;
	vista2isis["ubyte"] = _internal::reader<uint8_t>;
	vista2isis["short"] = _internal::reader<int16_t>;
	vista2isis["long"] =  _internal::reader<int32_t>;
	vista2isis["float"] = _internal::reader<float>;
	vista2isis["double"] = _internal::reader<double>;

}

int ImageFormat_VistaSa::load( std::list<data::Chunk> &chunks, const std::string &filename, const util::istring &dialect ) throw( std::runtime_error & )
{
	data::FilePtr mfile ( filename );

	if ( !mfile.good() ) {
		if ( errno ) {
			throwSystemError ( errno, filename + " could not be opened" );
			errno = 0;
		} else
			throwGenericError ( filename + " could not be opened" );
	}


	//parse the vista header
	isis::data::ValueArray< uint8_t >::iterator data_start = mfile.begin();
	isis::util::PropertyMap root_map;
	std::list<isis::util::PropertyMap> ch_list;
	size_t old_size = chunks.size();
	std::list<std::list<data::Chunk> > groups;

	if ( _internal::parse_vista( data_start, mfile.end(), root_map, ch_list ) ) {
		unsigned short last_type = 0;
		util::fvector4 last_voxelsize;

		uint32_t acqNum;
		BOOST_FOREACH( const isis::util::PropertyMap & chMap, ch_list ) {
			isis::data::Chunk ch = makeChunk( mfile, data_start, chMap );

			// start new group if the chunk's type differs from the last, or its the first chunk (different type probably means a new image)
			if( last_type != ch.getTypeID() || last_voxelsize != ch.getPropertyAs<util::fvector4>( "voxelSize" ) ) {
				groups.push_back( std::list<data::Chunk>() );
				last_type = ch.getTypeID();
				last_voxelsize = ch.getPropertyAs<util::fvector4>( "voxelSize" );
				acqNum = 0;
			}

			ch.setPropertyAs( "acquisitionNumber", acqNum++ );
			groups.back().push_back( ch );
		}
		LOG( Runtime, info ) << "Parsing vista succeeded " << groups.size() << " chunk-groups created";



		BOOST_FOREACH( std::list<data::Chunk> &group, groups ) {
			if( isFunctional( group ) )
				group = transformFunctional( group );

			BOOST_FOREACH( data::Chunk & ch, group ) {
				ch.branch( "vista" ).join( root_map );
				chunks.push_back( ch );
			}
		}
		return chunks.size() - old_size;
	} else {
		LOG( Runtime, error ) << "Parsing vista failed";
		return -1;
	}
}



void ImageFormat_VistaSa::write( const data::Image &/*image*/, const std::string &/*filename*/, const util::istring &/*dialect*/ )  throw( std::runtime_error & )
{
	throwGenericError( "Not available yet" );
}



}

}

isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_VistaSa();
}
