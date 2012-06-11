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
#ifndef IMAGEFORMAT_VISTA_SA_HPP
#define IMAGEFORMAT_VISTA_SA_HPP

#include <DataStorage/io_interface.h>
#include "DataStorage/fileptr.hpp"

#include "VistaSaParser.hpp"

namespace isis
{

namespace image_io
{

class ImageFormat_VistaSa: public FileFormat
{
private:
	typedef data::ValueArrayReference ( *readerPtr )( data::FilePtr data, size_t offset, size_t size );

	class VistaProtoImage: private std::list<data::Chunk>
	{
		readerPtr m_reader;
		data::FilePtr m_fileptr;
		data::ValueArray< uint8_t >::iterator m_data_start;

		unsigned short last_type;
		util::fvector4 last_voxelsize;
		util::istring last_repn;
		util::PropertyValue last_component;
		bool big_endian;

		std::map<util::istring, readerPtr> vista2isis;

		void swapEndian( data::ValueArrayBase &array );

		template<typename T> data::ValueArray<util::color<T> > toColor( const data::ValueArrayReference ref, size_t slice_size ) {
			assert( ref->getLength() % 3 == 0 );
			const std::vector< data::ValueArrayReference > layers = ref->as<T>().splice( slice_size ); //colors are stored slice-wise in the 3d block
			data::ValueArray<util::color<T> > ret( ref->getLength() / 3 );

			typename data::ValueArray<util::color<T> >::iterator d_pix = ret.begin();

			for( std::vector< data::ValueArrayReference >::const_iterator l = layers.begin(); l != layers.end(); ) {
				BOOST_FOREACH( T s_pix, ( *l++ )->castToValueArray<T>() )( *( d_pix++ ) ).r = data::endianSwap( s_pix ); //red
				d_pix -= slice_size; //return to the slice start
				BOOST_FOREACH( T s_pix, ( *l++ )->castToValueArray<T>() )( *( d_pix++ ) ).g = data::endianSwap( s_pix ); //green
				d_pix -= slice_size; //return to the slice start
				BOOST_FOREACH( T s_pix, ( *l++ )->castToValueArray<T>() )( *( d_pix++ ) ).b = data::endianSwap( s_pix ); //blue
			}

			big_endian = false;
			return ret;
		}

	public:
		VistaProtoImage( data::FilePtr fileptr, data::ValueArray< uint8_t >::iterator data_start );
		/// add a chunk to the protoimage
		bool add( util::PropertyMap props );
		bool isFunctional()const;
		void transformFunctional( );
		void fakeAcqNum();

		/// store the protoimage's' chunks into the output list, do byteswap if necessary
		void store( std::list< data::Chunk >& out, const util::PropertyMap &root_map, uint16_t sequence );
	};

public:
	std::string getName()const {return "Vista standalone";}
	int load ( std::list< data::Chunk >& chunks, const std::string &filename, const util::istring &dialect, boost::shared_ptr< util::ProgressFeedback > ) throw ( std::runtime_error & );
	void write( const data::Image &, const std::string &, const util::istring &, boost::shared_ptr< util::ProgressFeedback > )  throw( std::runtime_error & );

	bool tainted()const {return false;}//internal plugins are not tainted
	util::istring dialects( const std::string &/*filename*/ )const {return "";}
	static void sanitize( util::PropertyMap &obj );


protected:
	util::istring suffixes( io_modes /*mode = both */ )const {return ".v";}
};

}
}

#endif // IMAGEFORMAT_VISTA_SA_HPP