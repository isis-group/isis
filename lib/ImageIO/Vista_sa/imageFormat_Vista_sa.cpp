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
#include <time.h>
#include "boost/date_time/posix_time/posix_time.hpp"


namespace isis
{

namespace image_io
{

namespace _internal
{
template<typename T> data::ValueArrayReference reader( data::FilePtr data, size_t offset, size_t size )
{
	return data.atByID( data::ValueArray<T>::staticID, offset, size );
}

template<> data::ValueArrayReference reader<bool>( data::FilePtr data, size_t offset, size_t size )
{
	return reader< uint8_t >( data, offset, size )->as<bool>(); //@todo check if scaling is computed
}

}

ImageFormat_VistaSa::VistaProtoImage::VistaProtoImage( data::FilePtr fileptr, data::ValueArray< uint8_t >::iterator data_start ): m_fileptr( fileptr ), m_data_start( data_start ), big_endian( true )
{
	vista2isis["bit"] =   _internal::reader<bool>;
	vista2isis["ubyte"] = _internal::reader<uint8_t>;
	vista2isis["short"] = _internal::reader<int16_t>;
	vista2isis["long"] =  _internal::reader<int32_t>;
	vista2isis["float"] = _internal::reader<float>;
	vista2isis["double"] = _internal::reader<double>;
}

bool ImageFormat_VistaSa::VistaProtoImage::add( util::PropertyMap props )
{
	util::PropertyMap &vistaTree = props.branch( "vista" );

	if( empty() ) {
		last_voxelsize = props.getPropertyAs<util::fvector4>( "voxelSize" );
		last_repn = vistaTree.getPropertyAs<std::string>( "repn" ).c_str();

		if( vistaTree.hasProperty( "component_repn" ) )last_component = vistaTree.propertyValue( "component_repn" ) ;

		if( !( m_reader = vista2isis[last_repn] ) )
			throwGenericError( std::string( "Cannot handle repn " ) + vistaTree.getPropertyAs<std::string>( "repn" ) );

	} else  if(
		last_voxelsize != props.getPropertyAs<util::fvector4>( "voxelSize" ) ||
		last_repn != vistaTree.getPropertyAs<std::string>( "repn" ).c_str() ||
		( vistaTree.hasProperty( "component_repn" ) && last_component != vistaTree.propertyValue( "component_repn" ) )
	) {
		return false;
	}

	const size_t ch_offset = vistaTree.getPropertyAs<uint64_t>( "data" );

	util::vector4<size_t> ch_size( vistaTree.getPropertyAs<uint32_t>( "ncolumns" ), vistaTree.getPropertyAs<uint32_t>( "nrows" ), vistaTree.getPropertyAs<uint32_t>( "nbands" ), 1 );

	data::ValueArrayReference ch_data = m_reader( m_fileptr, std::distance( m_fileptr.begin(), m_data_start ) + ch_offset, ch_size.product() );

	//those are not needed anymore
	vistaTree.remove( "ncolumns" );

	vistaTree.remove( "nrows" );

	vistaTree.remove( "nbands" );

	vistaTree.remove( "data" );

	vistaTree.remove( "repn" );


	if( last_component  == std::string( "rgb" ) ) { // if its color replace original data by an ValueArray<util::color<uint8_t> > (endianess swapping is done there as well)
		ch_data = toColor<uint8_t>( ch_data, ch_size.product() / ch_size[data::sliceDim] );
		ch_size[data::sliceDim] /= 3;
		vistaTree.remove( "component_repn" );
	}

	LOG( Runtime, verbose_info ) << "Creating " << ch_data->getTypeName() << "-Chunk of size "
								 << ch_size << " (offset was " << std::hex << std::distance( m_fileptr.begin(), m_data_start ) + ch_offset << "/"
								 << std::dec << std::distance( m_data_start + ch_offset + ch_data->getLength()*ch_data->bytesPerElem(), m_fileptr.end() ) << " bytes are left)";

	push_back( data::Chunk( ch_data, ch_size[0], ch_size[1], ch_size[2], ch_size[3] ) );
	static_cast<util::PropertyMap &>( back() ) = props;
	return true;
}

void ImageFormat_VistaSa::sanitize( util::PropertyMap &obj )
{
	if( obj.hasProperty( "vista/columnVec" ) && obj.hasProperty( "vista/rowVec" ) ) { // if we have the complete orientation
		obj.transform<util::fvector4>( "vista/columnVec", "rowVec" );
		obj.transform<util::fvector4>( "vista/rowVec", "columnVec" );
		transformOrTell<util::fvector4>( "vista/sliceVec", "sliceVec", obj, warning );
	} else if( obj.hasProperty( "vista/imageOrientationPatient" ) ) {
		const util::dlist vecs = obj.getPropertyAs<util::dlist>( "vista/imageOrientationPatient" ); // if we have the dicom style partial orientation
		util::dlist::const_iterator begin = vecs.begin(), middle = vecs.begin(), end = vecs.end();
		std::advance( middle, 3 );
		obj.setPropertyAs( "rowVec",    util::fvector4() ).castTo<util::fvector4>().copyFrom( begin, middle );
		obj.setPropertyAs( "columnVec", util::fvector4() ).castTo<util::fvector4>().copyFrom( middle, end );
	} else { // if we dont have an orientation
		obj.setPropertyAs( "rowVec",    util::fvector4( 1, 0 ) );
		obj.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
		obj.setPropertyAs( "sliceVec",  util::fvector4( 0, 0, -1 ) );
		LOG( Runtime, warning ) << "No orientation info was found, assuming identity matrix";
	}

	if(
		transformOrTell<util::fvector4>( "vista/imagePositionPatient", "indexOrigin", obj, info ) ||
		transformOrTell<util::fvector4>( "vista/indexOrigin", "indexOrigin", obj, warning )
	);
	else {
		LOG( Runtime, warning ) << "No position info was found, assuming 0 0 0";
		obj.setPropertyAs( "indexOrigin", util::fvector4( 0, 0 ) );
	}

	if( transformOrTell<util::fvector4>( "vista/lattice", "voxelSize", obj, info ) ) { // if we have lattice
		if( hasOrTell( "vista/voxel", obj, info ) ) { // use that as voxel size, and the difference to voxel as voxel gap
			obj.setPropertyAs<util::fvector4>( "voxelGap", obj.getPropertyAs<util::fvector4>( "vista/voxel" ) - obj.getPropertyAs<util::fvector4>( "voxelSize" ) ) ;
			obj.remove( "vista/voxel" );
		}
	} else if( transformOrTell<util::fvector4>( "vista/voxel", "voxelSize", obj, warning ) ) {
	} else
		obj.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1 ) );

	if( obj.hasProperty( "vista/diffusionBValue" ) ) {
		const float len = obj.getPropertyAs<float>( "vista/diffusionBValue" );

		if( len > 0 && transformOrTell<util::fvector4>( "vista/diffusionGradientOrientation", "diffusionGradient", obj, warning ) ) {
			util::fvector4 &vec = obj.propertyValue( "diffusionGradient" ).castTo<util::fvector4>();
			vec.norm();
			vec *= len;
			obj.remove( "vista/diffusionBValue" );
		}
	}

	transformOrTell<std::string>( "vista/seriesDescription", "sequenceDescription", obj, warning ) ||
	transformOrTell<std::string>( "vista/protocol", "sequenceDescription", obj, warning ) ||
	transformOrTell<std::string>( "vista/name", "sequenceDescription", obj, warning );

	transformOrTell<std::string>( "vista/patient", "subjectName", obj, warning );

	transformOrTell<std::string>( "vista/coilID", "transmitCoil", obj, info ) ||
	transformOrTell<std::string>( "vista/transmitCoil", "transmitCoil", obj, info );

	transformOrTell<uint16_t>( "vista/repetitionTime", "repetitionTime", obj, info ) ||
	transformOrTell<uint16_t>( "vista/repetition_time", "repetitionTime", obj, info );

	if( hasOrTell( "vista/sex", obj, warning ) ) {
		util::Selection gender( "male,female,other" );
		gender.set( obj.getPropertyAs<std::string>( "vista/sex" ).c_str() );

		if( ( int )gender ) {
			obj.setPropertyAs( "subjectGender", gender );
			obj.remove( "vista/sex" );
		}
	}

	//@todo implement "vista/date" and "vista/time"

	transformOrTell<uint16_t>( "vista/seriesNumber", "sequenceNumber", obj, warning );
	transformOrTell<float>( "vista/echoTime", "echoTime", obj, warning );

	tm buff;
	util::istring date_string;
	memset( &buff, 0, sizeof( tm ) );

	if( hasOrTell( "vista/date", obj, info ) ) {
		strptime( obj.getPropertyAs<std::string>( "vista/date" ).c_str(), "%d.%m.%Y", &buff );
		date_string = "vista/date";
	}

	if( hasOrTell( "vista/time", obj, info ) ) {
		strptime( obj.getPropertyAs<std::string>( "vista/time" ).c_str(), "%T", &buff );
		date_string = "vista/time";
		obj.remove( "vista/time" );
		obj.setPropertyAs( "sequenceStart", boost::posix_time::ptime_from_tm( buff ) );
	}

	if( !date_string.empty() )try {
			obj.setPropertyAs( "sequenceStart", boost::posix_time::ptime_from_tm( buff ) );
			obj.remove( "vista/date" );
		} catch( std::out_of_range &e ) {
			LOG( Runtime, warning ) << "Failed to parse " << std::make_pair( date_string, obj.propertyValue( date_string ) ) << " " << e.what();
		}
}

bool ImageFormat_VistaSa::VistaProtoImage::isFunctional() const
{
	//if we have only one chunk, its not functional
	if( size() > 1 ) {
		if( front().hasProperty( "vista/diffusionGradientOrientation" ) )
			return false; // dwi data look the same, but must not be transformed

		//if we have proper vista/ntimesteps its functional
		if( front().hasProperty( "vista/ntimesteps" ) && front().getPropertyAs<uint32_t>( "vista/ntimesteps" ) == front().getSizeAsVector()[data::sliceDim] ) {
			LOG_IF( front().is<int16_t>(), Runtime, warning ) << "Functional data found which is not VShort";
			return true;
		} else if( front().is<int16_t>() ) { //if we have short, its functional
			LOG( Runtime, warning ) << "Functional data found, but ntimesteps is not set or wrong (should be " << front().getSizeAsVector()[data::sliceDim] << ")";
			return true;
		}
	}

	return false;
}


void ImageFormat_VistaSa::VistaProtoImage::transformFunctional()
{

	LOG( Debug, info ) << "Transforming " << size() << " functional slices";

	const uint32_t slices = size(); // the amount of slices in the image (amount of slice/time chunks in this proto image)
	util::vector4<size_t> size = front().getSizeAsVector();
	const float Tr = front().getPropertyAs<float>( "repetitionTime" ); //the time between to volumes taken (time between two slices in a slice/time chunk)

	// first combine the slices
	data::Chunk dst = front().cloneToNew( size[0], size[1], slices, size[2] );
	dst.join( front() );

	for( size_t timestep = 0; timestep < size[2]; timestep++ ) {
		size_t slice_num = 0;

		for( iterator slice = begin(); slice != end(); slice++, slice_num++ ) {
			slice->copySlice( timestep, 0, dst, slice_num, timestep );
		}
	}

	//then splice up into volumes again
	dst.remove( "vista/slice_time" );
	dst.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
	list< data::Chunk > ret = dst.autoSplice();
	util::PropertyMap::DiffMap differences = front().getDifference( back() ); // figure out which properties differ between the timesteps
	differences.erase( "acquisitionNumber" ); // this will be set later

	// and fill the properties which differ

	// if we have a repetitionTime and differing slice_timing set acquisitionTime/Number per slice
	if( Tr && differences.find( "vista/slice_time" ) != differences.end() ) {
		uint32_t s = 0;
		LOG( Debug, info ) << "Computing acquisitionTime from vista/slice_time";

		for( iterator i = begin(); i != end(); i++, s++ ) {
			const float acq_first = i->getPropertyAs<float>( "vista/slice_time" ); //slice_time of the chunk is the acquisitionTime of this slice on the first repetition
			uint32_t t = 0;
			BOOST_FOREACH( data::Chunk & ref, ret ) {
				ref.propertyValueAt( "acquisitionTime", s ) = acq_first + Tr * t;
				ref.propertyValueAt( "acquisitionNumber", s ) = s + slices * t;
				t++;
			}
		}
	} else { // we need at least an acquisitionNumber per volume
		uint32_t a = 0;
		LOG( Debug, info ) << "NoComputing acquisitionTime from vista/slice_time";
		BOOST_FOREACH( data::Chunk & ref, ret ) {
			ref.setPropertyAs<uint32_t>( "acquisitionNumber", a++ );
		}
	}

	// fill the remaining props (e.g. indexOrigin)
	BOOST_FOREACH( util::PropertyMap::DiffMap::const_reference diff, differences ) {
		size_t n = 0;

		for( const_iterator i = begin(); i != end(); i++, n++ ) {
			const util::PropertyValue p = i->propertyValue( diff.first );
			LOG( Debug, info ) << "Copying per timestep property " << std::make_pair( diff.first, p ) << " into " << ret.size() << " volumes";
			BOOST_FOREACH( data::Chunk & ref, ret ) {
				ref.propertyValueAt( diff.first, n ) = p;
			}
		}
	}

	// replace old chunks by the new
	this->assign( ret.begin(), ret.end() );
}

void ImageFormat_VistaSa::VistaProtoImage::fakeAcqNum()
{
	uint32_t acqNum = 0;

	for( iterator i = begin(); i != end(); i++ )
		i->setPropertyAs( "acquisitionNumber", acqNum++ );

}

void ImageFormat_VistaSa::VistaProtoImage::swapEndian( data::ValueArrayBase &array )
{
	uint_fast8_t elemsize = array.bytesPerElem();
	boost::shared_ptr<uint8_t> raw = boost::shared_static_cast<uint8_t>( array.getRawAddress() );
	uint8_t *ptr = raw.get();

	for( size_t i = 0; i < array.getLength(); i++ ) {
		for( uint8_t p = 0; p < elemsize / 2; p++ ) {
			std::swap( ptr[p], ptr[elemsize - 1 - p] );
		}

		ptr += elemsize;
	}
}

void ImageFormat_VistaSa::VistaProtoImage::store( std::list< data::Chunk >& out, const util::PropertyMap &root_map, uint16_t sequence )
{
	while( !empty() ) {
		out.push_back( front() );
		pop_front();
		out.back().branch( "vista" ).join( root_map );

		if( !out.back().hasProperty( "sequenceNumber" ) )
			out.back().setPropertyAs( "sequenceNumber", sequence );

		if( big_endian )
			swapEndian( out.back().asValueArrayBase() ); //if endianess wasn't swapped till now, do it now
	}
}



int ImageFormat_VistaSa::load( std::list<data::Chunk> &chunks, const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> /*progress*/  ) throw( std::runtime_error & )
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
	data::ValueArray< uint8_t >::iterator data_start = mfile.begin();
	util::PropertyMap root_map;
	std::list<util::PropertyMap> ch_list;
	size_t old_size = chunks.size();

	if ( _internal::parse_vista( data_start, mfile.end(), root_map, ch_list ) ) {

		std::list<VistaProtoImage> groups;
		groups.push_back( VistaProtoImage( mfile, data_start ) );

		BOOST_FOREACH( const util::PropertyMap & chMap, ch_list ) {
			util::PropertyMap root;
			root.branch( "vista" ) = chMap;
			sanitize( root );

			if( !groups.back().add( root ) ) { //if current ProtoImage doesnt like
				groups.push_back( VistaProtoImage( mfile, data_start ) ); // try a new one
				assert( groups.back().add( root ) ); //a new one should always work
			}
		}
		LOG( Runtime, info ) << "Parsing vista succeeded " << groups.size() << " chunk-groups created";

		uint16_t sequence = 0;
		BOOST_FOREACH( VistaProtoImage & group, groups ) {
			if( group.isFunctional() )
				group.transformFunctional();
			else
				group.fakeAcqNum(); // we have to the acquisitionNumber

			group.store( chunks, root_map, sequence++ );
		}
		return chunks.size() - old_size;
	} else {
		LOG( Runtime, error ) << "Parsing vista failed";
		return -1;
	}
}



void ImageFormat_VistaSa::write( const data::Image &/*image*/, const std::string &/*filename*/, const util::istring &/*dialect*/, boost::shared_ptr<util::ProgressFeedback> /*progress*/  )  throw( std::runtime_error & )
{
	throwGenericError( "Not available yet" );
}



}

}

isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_VistaSa();
}
