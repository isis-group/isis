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


#include "vistaprotoimage.hpp"
#include "imageFormat_Vista_sa.hpp"
#include <DataStorage/io_interface.h>
#include <fstream>

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

VistaInputImage::VistaInputImage( data::FilePtr fileptr, data::ValueArray< uint8_t >::iterator data_start ): m_fileptr( fileptr ), m_data_start( data_start )
{
	big_endian=true;
	vista2isis["bit"] =   _internal::reader<bool>;
	vista2isis["ubyte"] = _internal::reader<uint8_t>;
	vista2isis["short"] = _internal::reader<int16_t>;
	vista2isis["long"] =  _internal::reader<int32_t>;
	vista2isis["float"] = _internal::reader<float>;
	vista2isis["double"] = _internal::reader<double>;
}

bool VistaInputImage::add( util::PropertyMap props )
{
	util::PropertyMap &vistaTree = props.branch( "vista" );

	if( empty() ) {
		last_voxelsize = props.getPropertyAs<util::fvector3>( "voxelSize" );
		last_repn = vistaTree.getPropertyAs<std::string>( "repn" ).c_str();

		if( vistaTree.hasProperty( "component_repn" ) )last_component = vistaTree.propertyValue( "component_repn" ) ;

		if( !( m_reader = vista2isis[last_repn] ) )
			FileFormat::throwGenericError( std::string( "Cannot handle repn " ) + vistaTree.getPropertyAs<std::string>( "repn" ) );

	} else  if(
		last_voxelsize != props.getPropertyAs<util::fvector3>( "voxelSize" ) ||
		last_repn != vistaTree.getPropertyAs<std::string>( "repn" ).c_str() ||
		( vistaTree.hasProperty( "component_repn" ) && last_component != vistaTree.propertyValue( "component_repn" ) )
	) {
		return false;
	}

	const size_t ch_offset = vistaTree.getPropertyAs<uint64_t>( "data" );

	util::vector4<size_t> ch_size( vistaTree.getPropertyAs<uint32_t>( "ncolumns" ), vistaTree.getPropertyAs<uint32_t>( "nrows" ), vistaTree.getPropertyAs<uint32_t>( "nbands" ), 1 );
	if(vistaTree.propertyValue( "nbands" ) == vistaTree.propertyValue( "nframes" ))
		vistaTree.remove("nframes");
	else
		LOG(Runtime,warning) 
		<< "Don't know what to do with nframes="<< vistaTree.propertyValue( "nframes" ) 
		<< " that differs from nbands=" << vistaTree.propertyValue( "nbands" );

	data::ValueArrayReference ch_data = m_reader( m_fileptr, std::distance( m_fileptr.begin(), m_data_start ) + ch_offset, ch_size.product() );

	//those are not needed anymore
	vistaTree.remove( "ncolumns" );

	vistaTree.remove( "nrows" );

	vistaTree.remove( "nbands" );

	vistaTree.remove( "data" );

	vistaTree.remove( "repn" );
	
	if(vistaTree.hasProperty( "length" )){
		LOG_IF(vistaTree.getPropertyAs<uint64_t>("length") != (ch_data->getLength()*ch_data->bytesPerElem()),Runtime,warning)
			<< "Length given in the header (" << util::MSubject( vistaTree.propertyValue("length")) <<") does not fit the images size " << util::MSubject(ch_size);
		vistaTree.remove( "length" );
	}


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

bool VistaInputImage::isFunctional() const
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


void VistaInputImage::transformFromFunctional()
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
	list< data::Chunk > ret = dst.autoSplice(1);
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
		LOG( Debug, info ) << "No valid vista/slice_time found - faking acquisitionNumber";
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

void VistaInputImage::fakeAcqNum()
{
	uint32_t acqNum = 0;

	for( iterator i = begin(); i != end(); i++ )
		i->setPropertyAs( "acquisitionNumber", acqNum++ );

}

void VistaInputImage::store( std::list< data::Chunk >& out, const util::PropertyMap &root_map, uint16_t sequence )
{
	while( !empty() ) {
		out.push_back( front() );
		pop_front();
		out.back().branch( "vista" ).join( root_map );

		if( !out.back().hasProperty( "sequenceNumber" ) )
			out.back().setPropertyAs( "sequenceNumber", sequence );

		if( big_endian )
			out.back().asValueArrayBase().endianSwap(); //if endianess wasn't swapped till now, do it now
	}
}

VistaOutputImage::VistaOutputImage(data::Image src){
	bool functional=false;
	big_endian = false;
	
	typeInfo::insert<bool>(isis2vista,"bit",1);
	typeInfo::insert<uint8_t>(isis2vista,"ubyte",2);
	typeInfo::insert<int16_t>(isis2vista,"short",3);
	typeInfo::insert<int32_t>(isis2vista,"long",4);
	typeInfo::insert<float>(isis2vista,"long",5);
	typeInfo::insert<double>(isis2vista,"long",6);
	
	if(src.getRelevantDims()>3){
		functional=true;
	}

	util::vector4<size_t> imgSize=src.getSizeAsVector();

	data::dimensions disiredDims;
	imageProps=src; // copy common metata from the image
	const util::PropertyMap::PropPath indexOrigin("indexOrigin");
	
	if(functional){
		LOG(Runtime,info) << "Got a functional "  << imgSize << "-Image for writing";
		const util::PropertyMap::PropPath acquisitionNumber("acquisitionNumber"),acquisitionTime("acquisitionTime");
		chunksPerVistaImage=imgSize[data::timeDim];
		disiredDims=data::sliceDim;
		src.spliceDownTo(data::sliceDim);// @todo this will do an unneeded reIndex
		for(size_t slice=0;slice<imgSize[data::sliceDim];slice++)
			for(size_t time=0;time<imgSize[data::timeDim];time++){
				push_back(src.getChunk(0,0,slice,time,false)); // store the chunks in the list dim-swapped
				back().remove(acquisitionNumber);
				back().remove(acquisitionTime);
			}
		imageProps.remove(indexOrigin); // we use the positions of the chunks in this case
	} else {
		LOG(Runtime,info) << "Got a normal "  << imgSize << "-Image for writing";
		const std::vector< data::Chunk > chunks=src.copyChunksToVector(false);
		assign(chunks.begin(),chunks.end());
		BOOST_FOREACH(data::Chunk &ref,static_cast<std::list<data::Chunk>&>(*this)){
			ref.remove(indexOrigin);
		}

		chunksPerVistaImage=1;
		disiredDims=data::timeDim;
	}
	
	
	storeTypeID=0;
	for(const_iterator c=begin();c!=end();c++){
		unsigned short myID=c->getTypeID();
		switch(myID){ // some types need fallbacks @todo messages should not be repeating
			case data::ValueArray<int8_t>::staticID: typeFallback<int8_t,int16_t>(myID);//fall back to short
			case data::ValueArray<uint16_t>::staticID:typeFallback<uint16_t,int32_t>(myID); //fall back to int
			case data::ValueArray<util::color48>::staticID:typeFallback<util::color48,util::color24>(myID);
		}

		const std::map< unsigned short, typeInfo >::const_iterator me = isis2vista.find(myID);
		if(me!=isis2vista.end()){//if myID is a supported type
			if(storeTypeID==0){
				storeTypeID=myID;
				continue;
			}
			if(storeTypeID!=myID){ // if we already have a type but its not the same, check if we can switch
				const typeInfo &myInfo=isis2vista[myID],&storeInfo=isis2vista[storeTypeID];
				if(myInfo.isInt != storeInfo.isInt || myInfo.isFloat != storeInfo.isFloat ){
					LOG(Runtime,error) << "Cannot store image of incompatible data types " << util::MSubject(myInfo.vistaName) << " and " << util::MSubject(storeInfo.vistaName);
					ImageFormat_VistaSa::throwGenericError("incompatible types");
				}
				if(myInfo.priority>storeInfo.priority)
					storeTypeID=myID;
			}
				
		} else {
			LOG(Runtime,error) << "Chunk data type " << util::MSubject(c->getTypeName()) << " is not supported, aborting ..";
			ImageFormat_VistaSa::throwGenericError("unsupported type");
		}
	}
	assert(storeTypeID);

}

void VistaOutputImage::storeVImages(std::ofstream& out)
{
	while(!empty()){
		data::Chunk &ch=front();
		if(!ch.convertToType(storeTypeID)){
			LOG(Runtime,error) << "Failed to store "  << ch.getTypeName() << "-Chunk as " << isis2vista[storeTypeID].vistaName;
		}

		if(!big_endian)
			ch.asValueArrayBase().endianSwap();
		
		storeVImage(ch.asValueArrayBase(),out);
		pop_front();
	}
}
void VistaOutputImage::storeVImage(const isis::data::ValueArrayBase& ref, std::ofstream& out)
{
	const size_t size = ref.bytesPerElem()*ref.getLength();
	boost::shared_ptr<const char> raw = boost::shared_static_cast<const char>( ref.getRawAddress() );
	out.write(raw.get(),size);
}
void VistaOutputImage::storeHeaders(std::ofstream& out,size_t &offset)
{
	ImageFormat_VistaSa::unsanitize(imageProps);
	for(std::list<data::Chunk>::const_iterator c=begin();c!=end();){
		util::PropertyMap store;
		util::vector4<size_t> size=c->getSizeAsVector();
		size[c->getRelevantDims()]=chunksPerVistaImage;
		for(size_t i=0;i<chunksPerVistaImage;i++,c++){
			const util::PropertyMap::KeyList rejected=store.join(*c);
			LOG_IF(!rejected.empty(),Runtime,warning) << "Failed to merge chunk properties into VImage because there are already some with the same name: " << rejected;
		}
		storeHeader(store,size,offset,out);
		offset+=size.product()*isis2vista[storeTypeID].elemSize;
	}
}
void VistaOutputImage::storeHeader(const util::PropertyMap &ch, const util::vector4<size_t> size, size_t data_offset, std::ofstream& out)
{
	util::PropertyMap store(imageProps);
	util::PropertyMap::KeyList rejected=store.join(ch);
	LOG_IF(!rejected.empty(),Runtime,warning) << "Failed to store chunk properties because there are already some from the image with the same name: " << rejected;

	//store offset and length of the data
	store.setPropertyAs("data",data_offset);
	store.setPropertyAs("length",size.product()*isis2vista[storeTypeID].elemSize);
	store.setPropertyAs("repn",isis2vista[storeTypeID].vistaName);

	// store chunks size
	store.setPropertyAs<uint64_t>("ncolumns",size[data::rowDim]);
	store.setPropertyAs<uint64_t>("nrows",   size[data::columnDim]);
	store.setPropertyAs<uint64_t>("nframes", size[data::sliceDim]);
	store.setPropertyAs<uint64_t>("nbands",  size[data::sliceDim]);
	
	// those names are swapped in vista
	std::swap(store.propertyValue("rowVec"),store.propertyValue("columnVec"));
	
	if(store.hasBranch("vista")){
		util::PropertyMap vista;vista.branch("vista")=store.branch("vista"); //workaround for #66
		store.remove(vista);
		util::PropertyMap::KeyList rejected= store.join(vista.branch("vista"));
		LOG_IF(!rejected.empty(),Runtime,warning) << "Failed to store properties from the vista branch because there are already some with the same name: " << rejected;
	}

	writeMetadata(out,store,"image: image");
}

void VistaOutputImage::extractHistory(util::slist& ref)
{
	static const util::PropertyMap::PropPath history("vista/history");
	
	if(imageProps.hasProperty(history)){
		const util::slist hist=imageProps.getPropertyAs<util::slist>(history);
		if(ref.empty() || ref==hist)imageProps.remove(history);
		if(ref.empty())ref=hist;
		LOG_IF(ref!=hist,Runtime,warning)<< "Not extracting history conflicting history " << util::MSubject(hist);
	}
}

void VistaOutputImage::writeMetadata(std::ofstream& out, const isis::util::PropertyMap& data, const std::string& title, size_t indent)
{
	std::string indenter(indent,'\t');
	out << indenter << title << " {" << std::endl ;
	BOOST_FOREACH(const util::PropertyMap::FlatMap::value_type &ref,data.getFlatMap()){
		out << indenter  << "\t" << ref.first << ": ";
		util::slist list;
		switch(ref.second.getTypeID()){
			case util::Value<std::string>::staticID: // string need to be in ""
				out << "\"" << ref.second.castTo<std::string>() << "\"";
				break;
			case util::Value<util::fvector3>::staticID: //value lists and vectors need to be in "" and space separated
			case util::Value<util::fvector4>::staticID:
			case util::Value<util::dvector3>::staticID: 
			case util::Value<util::dvector4>::staticID:
			case util::Value<util::ivector4>::staticID:
			case util::Value<util::ilist>::staticID:
			case util::Value<util::dlist>::staticID:
			case util::Value<util::slist>::staticID:
				list=ref.second.as<util::slist>();
				util::listToOStream(list.begin(),list.end(),out," ","\"","\"");
				break;
			default:
				out << ref.second.as<std::string>();
				break;
		}
		out << std::endl;
	}
	out << indenter << "}" << std::endl;
}


}
}
}
