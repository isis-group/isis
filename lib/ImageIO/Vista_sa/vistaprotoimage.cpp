/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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
		last_voxelsize = props.getPropertyAs<util::fvector3>( "voxelSize" );
		last_repn = vistaTree.getPropertyAs<std::string>( "repn" ).c_str();

		if( vistaTree.hasProperty( "component_repn" ) )last_component = vistaTree.propertyValue( "component_repn" ) ;

		if( !( m_reader = vista2isis[last_repn] ) )
			throwGenericError( std::string( "Cannot handle repn " ) + vistaTree.getPropertyAs<std::string>( "repn" ) );

	} else  if(
		last_voxelsize != props.getPropertyAs<util::fvector3>( "voxelSize" ) ||
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




}
}
