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
#include <isis/core/io_interface.h>
#include <fstream>

namespace isis
{
namespace image_io
{

namespace _internal
{
	
template<typename T> data::ValueArrayReference reader( data::ByteArray data, size_t offset, size_t size )
{
	return data.atByID( data::ValueArray<T>::staticID(), offset, size );
}

template<> data::ValueArrayReference reader<bool>( data::ByteArray data, size_t offset, size_t size )
{
	return reader< uint8_t >( data, offset, size )->as<bool>(); //@todo check if scaling is computed
}

VistaInputImage::VistaInputImage( data::ByteArray data, data::ValueArray< uint8_t >::iterator data_start ): m_data( data ), m_data_start( data_start )
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
	util::PropertyMap &vistaTree = *props.queryBranch( "vista" );

	if( empty() ) {
		last_voxelsize = props.getValueAs<util::fvector3>( "voxelSize" );
		last_repn = vistaTree.getValueAs<std::string>( "repn" ).c_str();

		if( vistaTree.hasProperty( "component_repn" ) )last_component = *vistaTree.queryProperty( "component_repn" ) ;

		if( !( m_reader = vista2isis[last_repn] ) )
			FileFormat::throwGenericError( std::string( "Cannot handle repn " ) + vistaTree.getValueAs<std::string>( "repn" ) );

	} else  if(
		last_voxelsize != props.getValueAs<util::fvector3>( "voxelSize" ) ||
		last_repn != vistaTree.getValueAs<std::string>( "repn" ).c_str() ||
		( vistaTree.hasProperty( "component_repn" ) && last_component != *vistaTree.queryProperty( "component_repn" ) )
	) {
		return false;
	}

	const size_t ch_offset = vistaTree.getValueAs<uint64_t>( "data" );

	util::vector3<size_t> ch_size{
		vistaTree.getValueAs<uint32_t>( "ncolumns" ), 
		vistaTree.getValueAs<uint32_t>( "nrows" ), 
		vistaTree.getValueAs<uint32_t>( "nbands" )
	};
	if(*vistaTree.queryProperty( "nbands" ) == *vistaTree.queryProperty( "nframes" ))
		vistaTree.remove("nframes");
	else if(
		vistaTree.hasProperty( "ncomponents" ) && 
		vistaTree.getValueAs<uint64_t>( "nbands" ) / vistaTree.getValueAs<uint64_t>( "ncomponents" ) == vistaTree.getValueAs<uint64_t>( "nframes" )
	) {
		vistaTree.remove("nframes");
		vistaTree.remove( "ncomponents" );
	} else
		LOG(Runtime,warning) 
		<< "Don't know what to do with nframes="<< vistaTree.queryProperty( "nframes" ) 
		<< " that differs from nbands=" << vistaTree.queryProperty( "nbands" );

	data::ValueArrayReference ch_data = m_reader( m_data, std::distance( m_data.begin(), m_data_start ) + ch_offset, util::product(ch_size) );

	//those are not needed anymore
	vistaTree.remove( "ncolumns" );

	vistaTree.remove( "nrows" );

	vistaTree.remove( "nbands" );

	vistaTree.remove( "data" );

	vistaTree.remove( "repn" );
	
	if(vistaTree.hasProperty( "length" )){
		LOG_IF(vistaTree.getValueAs<uint64_t>("length") != (ch_data->getLength()*ch_data->bytesPerElem()),Runtime,warning)
			<< "Length given in the header (" << *vistaTree.queryProperty("length") <<") does not fit the images size " << ch_size;
		vistaTree.remove( "length" );
	}


	if( last_component  == std::string( "rgb" ) ) { // if its color replace original data by an ValueArray<util::color<uint8_t> > (endianess swapping is done there as well)
		ch_data = toColor<uint8_t>( ch_data, util::product(ch_size) / ch_size[data::sliceDim] );
		ch_size[data::sliceDim] /= 3;
		vistaTree.remove( "component_repn" );
	}

	LOG( Runtime, verbose_info ) << "Creating " << ch_data->getTypeName() << "-Chunk of size "
								 << ch_size << " (offset was " << std::distance( m_data.begin(), m_data_start ) + ch_offset << " / "
								 << std::distance( m_data_start + ch_offset + ch_data->getLength()*ch_data->bytesPerElem(), m_data.end() ) << " bytes are left)";

	push_back( data::Chunk( ch_data, ch_size[0], ch_size[1], ch_size[2] ) );
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
		if( front().hasProperty( "vista/ntimesteps" ) && front().getValueAs<uint32_t>( "vista/ntimesteps" ) == front().getSizeAsVector()[data::sliceDim] ) {
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
	const float Tr = front().getValueAs<float>( "repetitionTime" ); //the time between to volumes taken (time between two slices in a slice/time chunk)

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
	dst.setValueAs<uint32_t>( "acquisitionNumber", 0 );
	list< data::Chunk > ret = dst.autoSplice(1);
	util::PropertyMap::DiffMap differences = front().getDifference( back() ); // figure out which properties differ between the timesteps
	differences.erase( "acquisitionNumber" ); // this will be set later

	// and fill the properties which differ

	// if we have a repetitionTime and differing slice_timing set acquisitionTime/Number per slice
	if( Tr && differences.find( "vista/slice_time" ) != differences.end() ) {
		uint32_t s = 0;
		LOG( Debug, info ) << "Computing acquisitionTime from vista/slice_time";

		for( iterator i = begin(); i != end(); i++, s++ ) {
			const float acq_first = i->getValueAs<float>( "vista/slice_time" ); //slice_time of the chunk is the acquisitionTime of this slice on the first repetition
			uint32_t t = 0;
			for( data::Chunk & ref: ret ) {
#pragma message "Implement me"
//				ref.refValueAs( "acquisitionTime", s ) = acq_first + Tr * t;
				ref.refValueAs<uint32_t>( "acquisitionNumber", s ) = s + slices * t;
				t++;
			}
		}
	} else { // we need at least an acquisitionNumber per volume
		uint32_t a = 0;
		LOG( Debug, info ) << "No valid vista/slice_time found - faking acquisitionNumber";
		for( data::Chunk & ref: ret ) {
			ref.setValueAs<uint32_t>( "acquisitionNumber", a++ );
		}
	}

	// fill the remaining props (e.g. indexOrigin)
	for( util::PropertyMap::DiffMap::const_reference diff: differences ) {
		size_t n = 0;

		for( const_iterator i = begin(); i != end(); i++, n++ ) {
			const util::PropertyValue p = *i->queryProperty( diff.first );
			LOG( Debug, info ) << "Copying per timestep property " << std::make_pair( diff.first, p ) << " into " << ret.size() << " volumes";
			for( data::Chunk & ref: ret ) {
				ref.setValue( diff.first, p.front(), n);
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
		i->setValueAs( "acquisitionNumber", acqNum++ );

}

void VistaInputImage::store( std::list< data::Chunk >& out, const util::PropertyMap &root_map, uint16_t sequence, const std::shared_ptr<util::ProgressFeedback> &feedback )
{
	while( !empty() ) {
		out.push_back( front() );
		pop_front();
		const auto rejected=out.back().touchBranch( "vista" ).join( root_map );
		if(!rejected.empty()){
			LOG(Runtime,warning) << "The global entries " << rejected << " where rejected";
		} 

		if( !out.back().hasProperty( "sequenceNumber" ) )
			out.back().setValueAs( "sequenceNumber", sequence );

		if( big_endian )
			out.back().asValueArrayBase().endianSwap(); //if endianess wasn't swapped till now, do it now
		if(feedback) 
			++(*feedback);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// writing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//general writer spec
WriterSpec::WriterSpec(std::string repn, std::string name, uint8_t prio, bool isInt, bool isFloat, uint8_t sizeFact, uint16_t storeTypeID):
	m_isInt(isInt),m_isFloat(isFloat),m_storeTypeID(storeTypeID),m_sizeFact(sizeFact),m_priority(prio),m_vistaRepnName(repn),m_vistaImageName(name){}
void WriterSpec::modHeaderImpl(isis::util::PropertyMap& props, const isis::util::vector4<size_t>& size)
{
	props.setValueAs<uint64_t>("length",util::product(size)*m_sizeFact);
	props.setValueAs("repn",m_vistaRepnName);

	// store chunks size
	props.setValueAs<uint64_t>("ncolumns",size[data::rowDim]);
	props.setValueAs<uint64_t>("nrows",   size[data::columnDim]);
	props.setValueAs<uint64_t>("nframes", size[data::sliceDim]);
	props.setValueAs<uint64_t>("nbands",  size[data::sliceDim]);

}
uint16_t WriterSpec::storeVImageImpl(std::list< isis::data::Chunk >& chunks, std::ofstream& out, isis::data::scaling_pair scaling)
{
	while(!chunks.empty()){
		data::ValueArrayReference ref = chunks.front().asValueArrayBase().convertByID( m_storeTypeID, scaling );
		if(ref.isEmpty()) // if conversion failed
			return chunks.front().getTypeID(); // abort writing and return the failed type
		
		ref->endianSwap();
		const size_t size = m_sizeFact*ref->getLength();
		std::shared_ptr<const char> raw = std::static_pointer_cast<const char>( ref->getRawAddress() );
		out.write(raw.get(),size);
		chunks.pop_front(); //remove written chunk - we wont need it anymore
	}
	return 0;
}
// special writer spec for color images
void typeSpecImpl< util::color24 >::modHeaderImpl(util::PropertyMap& props, const util::vector4<size_t>& size)
{
	WriterSpec::modHeaderImpl(props, size);
	props.setValueAs<uint64_t>("nbands",  size[data::sliceDim]*3); //nbands is nframes*3 in color images
}
uint16_t typeSpecImpl< util::color24 >::storeVImageImpl(std::list< isis::data::Chunk >& chunks, std::ofstream& out, isis::data::scaling_pair scaling )
{
	
	for(data::Chunk &ref:chunks){ //first make shure all are util::color24 (they might be color48)
		if(!ref.convertToType( data::ValueArray<util::color24>::staticID(), scaling )) // if conversion failed
			return ref.getTypeID(); // abort writing and return the failed type
	}
	
	for(data::Chunk &ch:chunks)
		for(util::color24 &col:ch.asValueArray<util::color24>())out.put(col.r);//write red from each voxel from each chunk
	for(data::Chunk &ch:chunks)
		for(util::color24 &col:ch.asValueArray<util::color24>())out.put(col.g);//same for green
	for(data::Chunk &ch:chunks)
		for(util::color24 &col:ch.asValueArray<util::color24>())out.put(col.b);//same for blue
	return 0;
}


VistaOutputImage::VistaOutputImage(data::Image src){
	bool functional=false;
	
	insertSpec<bool>(isis2vista,"bit",1);
	insertSpec<uint8_t>(isis2vista,"ubyte",2);
	insertSpec<int16_t>(isis2vista,"short",3);
	insertSpec<int32_t>(isis2vista,"long",4);
	
	insertSpec<float>(isis2vista,"float",10);
	insertSpec<double>(isis2vista,"double",11);
	
	insertSpec<util::color24>(isis2vista,"ubyte",20);
	
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
		for(data::Chunk &ref:static_cast<std::list<data::Chunk>&>(*this)){
			if(ref.hasProperty(indexOrigin)) ref.remove(indexOrigin);
		}

		chunksPerVistaImage=1;
		disiredDims=data::timeDim;
	}
	
	
	storeTypeID=0;
	for(const_iterator c=begin();c!=end();c++){
		unsigned short myID=c->getTypeID();
		switch(myID){ // some types need fallbacks @todo messages should not be repeating
			case data::ValueArray<int8_t>::staticID(): typeFallback<int8_t,int16_t>(myID);//fall back to short
			case data::ValueArray<uint16_t>::staticID():typeFallback<uint16_t,int32_t>(myID); //fall back to int
			case data::ValueArray<util::color48>::staticID():typeFallback<util::color48,util::color24>(myID);
		}

		std::map<unsigned short,boost::shared_ptr<WriterSpec> >::const_iterator me = isis2vista.find(myID);
		if(me!=isis2vista.end()){//if myID is a supported type
			if(storeTypeID==0){
				storeTypeID=myID;
				continue;
			}
			if(storeTypeID!=myID){ // if we already have a type but its not the same, check if we can switch
				const WriterSpec &mySpec=*isis2vista[myID],&storeSpec=*isis2vista[storeTypeID];
				if(mySpec.isCompatible(storeSpec)){
					LOG(Runtime,error) << "Cannot store image of incompatible data types " << util::MSubject(mySpec.m_vistaRepnName) << " and " << util::MSubject(storeSpec.m_vistaRepnName);
					ImageFormat_VistaSa::throwGenericError("incompatible types");
				}
				if(mySpec.m_priority>storeSpec.m_priority)
					storeTypeID=myID;
			}
				
		} else {
			LOG(Runtime,error) << "Chunk data type " << util::MSubject(c->getTypeName()) << " is not supported, aborting ..";
			ImageFormat_VistaSa::throwGenericError("unsupported type");
		}
	}
	scaling = src.getScalingTo(storeTypeID); //@todo we only need this if we do a type conversion
	assert(storeTypeID);

}

void VistaOutputImage::storeVImages(std::ofstream& out)
{
	const uint16_t fail=isis2vista[storeTypeID]->storeVImageImpl(*this,out,scaling);
	
	if(fail){
		LOG(Runtime,error) << "Failed to store "  << util::getTypeMap(false,true)[fail] << "-Chunk as " << isis2vista[storeTypeID]->m_vistaRepnName;
		clear();//clear this proto image
	}
}
void VistaOutputImage::storeHeaders(std::ofstream& out,size_t &offset)
{
	ImageFormat_VistaSa::unsanitize(imageProps);
	for(std::list<data::Chunk>::const_iterator c=begin();c!=end();){
		util::PropertyMap store;
		util::vector4<size_t> size=c->getSizeAsVector();
		size[c->getRelevantDims()]=chunksPerVistaImage;
		for(size_t i=0;i<chunksPerVistaImage;i++,c++){
			const util::PropertyMap::PathSet rejected=store.join(*c);
			LOG_IF(!rejected.empty(),Runtime,warning) << "Failed to merge chunk properties into VImage because there are already some with the same name: " << rejected;
		}
		storeHeader(store,size,offset,out);
		offset+=util::product(size)*isis2vista[storeTypeID]->m_sizeFact;
	}
}
void VistaOutputImage::storeHeader(const util::PropertyMap &ch, const util::vector4<size_t> size, size_t data_offset, std::ofstream& out)
{
	util::PropertyMap store(imageProps);
	util::PropertyMap::PathSet rejected=store.join(ch);
	LOG_IF(!rejected.empty(),Runtime,warning) << "Failed to store chunk properties because there are already some from the image with the same name: " << rejected;

	//store offset and length of the data
	store.setValueAs<uint64_t>("data",data_offset);
	isis2vista[storeTypeID]->modHeaderImpl(store,size); // type specific properties
	
	// those names are swapped in vista
	store.queryProperty("rowVec")->swap(*store.queryProperty("columnVec"));
	
	if(store.hasBranch("vista")){
		util::PropertyMap vista;vista.branch("vista")=store.branch("vista"); //workaround for #66
		store.remove(vista);
		util::PropertyMap::PathSet rejected= store.join(vista.branch("vista"));
		LOG_IF(!rejected.empty(),Runtime,warning) << "Failed to store properties from the vista branch because there are already some with the same name: " << rejected;
	}

	writeMetadata(out,store,isis2vista[storeTypeID]->m_vistaImageName+": image");
}

void VistaOutputImage::extractHistory(util::slist& ref)
{
	static const util::PropertyMap::PropPath history("vista/history");
	
	if(imageProps.hasProperty(history)){
		const util::slist hist=imageProps.getValueAs<util::slist>(history);
		if(ref.empty() || ref==hist)imageProps.remove(history);
		if(ref.empty())ref=hist;
		LOG_IF(ref!=hist,Runtime,warning)<< "Not extracting history conflicting history " << util::MSubject(hist);
	}
}

void VistaOutputImage::writeMetadata(std::ofstream& out, const isis::util::PropertyMap& data, const std::string& title, size_t indent)
{
	std::string indenter(indent,'\t');
	out << indenter << title << " {" << std::endl ;
	for(const util::PropertyMap::FlatMap::value_type &ref:data.getFlatMap()){
		out << indenter  << "\t" << ref.first << ": ";
		util::slist list;
		switch(ref.second.getTypeID()){
			case util::Value<std::string>::staticID(): // string need to be in ""
				out << "\"" << ref.second.castTo<std::string>() << "\"";
				break;
			case util::Value<util::fvector3>::staticID(): //value lists and vectors need to be in "" and space separated
			case util::Value<util::fvector4>::staticID():
			case util::Value<util::dvector3>::staticID(): 
			case util::Value<util::dvector4>::staticID():
			case util::Value<util::ivector4>::staticID():
			case util::Value<util::ilist>::staticID():
			case util::Value<util::dlist>::staticID():
			case util::Value<util::slist>::staticID():
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
