/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  <copyright holder> <email>
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VISTAPROTOIMAGE_HPP
#define VISTAPROTOIMAGE_HPP

#include <list>
#include <DataStorage/chunk.hpp>
#include <DataStorage/fileptr.hpp>
#include <boost/type_traits/is_float.hpp>
#include <boost/type_traits/is_integral.hpp>


namespace isis{namespace image_io{namespace _internal{

class WriterSpec{
protected:
	bool m_isInt,m_isFloat;
	WriterSpec(std::string repn,std::string name, uint8_t prio, bool isInt, bool isFloat, uint8_t sizeFact, uint16_t storeTypeID);
	uint16_t m_storeTypeID;
public:
	uint8_t m_sizeFact,m_priority;
	std::string m_vistaRepnName,m_vistaImageName;
	bool isCompatible(const WriterSpec &other)const{
		return (m_isInt != other.m_isInt || m_isFloat != other.m_isFloat );
	}
	virtual uint16_t storeVImageImpl(std::list< isis::data::Chunk >& chunks, std::ofstream& out, data::scaling_pair scaling );
	virtual void modHeaderImpl(isis::util::PropertyMap& props, const isis::util::vector4< size_t > &size);
	virtual ~WriterSpec(){};
};
template<typename T> class typeSpecImpl:public WriterSpec{
public:
	typeSpecImpl(std::string repn,uint8_t prio):
		WriterSpec(repn,"image",prio,boost::is_integral<T>::value,boost::is_float<T>::value,sizeof(T),data::ValueArray<T>::staticID){}
};
template<> class typeSpecImpl<util::color24>:public WriterSpec{
public:
	typeSpecImpl(std::string repn,uint8_t prio):
		WriterSpec(repn,"3DVectorfield",prio,false,false,3,data::ValueArray<util::color24>::staticID){}
	uint16_t storeVImageImpl(std::list< isis::data::Chunk >& chunks, std::ofstream& out, data::scaling_pair scaling );
	void modHeaderImpl(isis::util::PropertyMap& props, const isis::util::vector4< size_t > &size);
};

class VistaProtoImage: protected std::list<data::Chunk>{};

class VistaInputImage: public VistaProtoImage{
	typedef data::ValueArrayReference ( *readerPtr )( data::FilePtr data, size_t offset, size_t size );
	readerPtr m_reader;
	data::FilePtr m_fileptr;
	data::ValueArray< uint8_t >::iterator m_data_start;
	
	unsigned short last_type;
	util::fvector3 last_voxelsize;
	util::istring last_repn;
	util::PropertyValue last_component;
	
	std::map<util::istring, readerPtr> vista2isis;
	bool big_endian;
	
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
	VistaInputImage( data::FilePtr fileptr, data::ValueArray< uint8_t >::iterator data_start );
	/// add a chunk to the protoimage
	bool add( util::PropertyMap props );
	bool isFunctional()const;
	void transformFromFunctional( );
	void fakeAcqNum();
	
	/// store the protoimage's' chunks into the output list, do byteswap if necessary
	void store( std::list< data::Chunk >& out, const util::PropertyMap &root_map, uint16_t sequence );
};
class VistaOutputImage:public VistaProtoImage{
	template<typename T> void insertSpec(std::map<unsigned short,boost::shared_ptr<WriterSpec> > &map,std::string name,uint8_t prio){
		map[(uint16_t)data::ValueArray<T>::staticID].reset(new typeSpecImpl<T>(name,prio));
	}
	size_t chunksPerVistaImage;
	util::PropertyMap imageProps;
	void writeMetadata(std::ofstream& out, const isis::util::PropertyMap& data, const std::string& title, size_t indent=0);
	std::map<unsigned short,boost::shared_ptr<WriterSpec> > isis2vista;
	unsigned short storeTypeID;
	data::scaling_pair scaling;
    std::_Rb_tree_iterator< std::pair< const int, boost::shared_ptr< WriterSpec > > > me;
	template<typename FIRST,typename SECOND> static void typeFallback(unsigned short typeID){
		LOG(Runtime,notice) 
			<< util::MSubject(data::ValueArray<FIRST>::staticName()) << " is not supported in vista falling back to " 
			<< util::MSubject(data::ValueArray<SECOND>::staticName());
		typeID=data::ValueArray<SECOND>::staticID;
	}
public:
	VistaOutputImage(data::Image src);
	void storeVImages(std::ofstream &out);
	void extractHistory(util::slist &ref);
	void storeHeaders(std::ofstream& out, size_t& offset);
	void storeHeader( const isis::util::PropertyMap& ch, const isis::util::vector4< size_t > size, size_t data_offset, std::ofstream& out );
};

	
}}}

#endif //VISTAPROTOIMAGE_HPP