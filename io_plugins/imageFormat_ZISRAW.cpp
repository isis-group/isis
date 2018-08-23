#include "imageFormat_ZISRAW.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/iostreams/stream.hpp>
#include "imageFormat_ZISRAW_jxr.h"

namespace isis{
namespace image_io{

namespace _internal{

DirectoryEntryDV getDVEntry(data::ByteArray &data, size_t offset){
	const std::string type(&data[offset],&data[offset+2]);
	assert(type=="DV");
	DirectoryEntryDV ret;
	getScalar(data,ret.PixelType,offset+2);
	getScalar(data,ret.FilePosition,offset+6);
	getScalar(data,ret.Compression,offset+18);
	getScalar(data,ret.PyramidType,offset+22);
	getScalar(data,ret.DimensionCount,offset+28);
	ret.dims.resize(ret.DimensionCount);
	size_t d_offset=offset+32;
	for(DimensionEntry &dim:ret.dims){
		dim.Dimension= std::string((char*)&data[d_offset]);
		assert(dim.Dimension.size()<=4);
		getScalar(data,dim.start,d_offset+4);
		getScalar(data,dim.size,d_offset+8);
		getScalar(data,dim.StartCoordinate,d_offset+12);
		getScalar(data,dim.StoredSize,d_offset+16);
		d_offset+=20;
	}
	return ret;
}

boost::property_tree::ptree getXML(data::ByteArray &data, size_t offset, size_t length){
	typedef  boost::iostreams::basic_array_source<std::streambuf::char_type> my_source_type; // must be compatible to std::streambuf
	boost::property_tree::ptree ret;
	const uint8_t *start=data.begin()+offset;

	boost::iostreams::stream<my_source_type> stream;
	stream.open(my_source_type((const std::streambuf::char_type*)start,(const std::streambuf::char_type*)start+length));

	boost::property_tree::read_xml(stream,ret,boost::property_tree::xml_parser::no_comments|boost::property_tree::xml_parser::trim_whitespace);
	return ret;
}

data::ValueArrayReference reinterpretData(const data::ByteArray &_data, int32_t PixelType){
	data::ValueArrayReference ret;
	data::ByteArray &data=const_cast<data::ByteArray &>(_data);
	switch(PixelType){
	case 0://Gray8 - no reinterpretation needed
		ret=data;break;
	case 1: //Gray16
		ret=data.at<uint16_t>(0,data.getLength(),__BYTE_ORDER==__BIG_ENDIAN);break;
	case 12: //Gray32
		ret=data.at<uint32_t>(0,data.getLength(),__BYTE_ORDER==__BIG_ENDIAN);break;
	case 2://Gray32Float
		ret=data.at<float>(0,data.getLength(),__BYTE_ORDER==__BIG_ENDIAN);break;
	case 3://Bgr24
		ret=color_reshuffle<uint8_t>(data);break;
	case 4://Bgr48
		ret=color_reshuffle<uint16_t>(data.at<uint16_t>(0,data.getLength(),__BYTE_ORDER==__BIG_ENDIAN));break;
	case 10: // Gray64ComplexFloat
		ret=data.at<std::complex<float>>(0,data.getLength(),__BYTE_ORDER==__BIG_ENDIAN);break;
	case 11: // Bgr192ComplexFloat
		ret=data.at<std::complex<double>>(0,data.getLength(),__BYTE_ORDER==__BIG_ENDIAN);break;
	default:
		LOG(Runtime,error) << "Pixel Type " << PixelType << " not implemented";break;
	}
	return ret;
}

}

ImageFormat_ZISRAW::Segment::Segment(data::ByteArray &source, const size_t offset){
	// get segment type
	id=std::string((char*)&source[offset]);
	// read  size data
	auto buff=source.at<uint64_t>(offset+16,2,__BYTE_ORDER==__BIG_ENDIAN);
	allocated_size=buff[0];
	used_size=buff[1];
	data=source.at<uint8_t>(offset+16+16,used_size);
}

ImageFormat_ZISRAW::FileHeader::FileHeader(data::ByteArray &source, const size_t offset):Segment(source,offset){
	getScalar(version.major,0);
	getScalar(version.minor,4);
	data.at<uint8_t>(16,16).copyToMem(PrimaryFileGuid,16);
	data.at<uint8_t>(32,16).copyToMem(FileGuid,16);
	getScalar(FilePart,48);
	getScalar(DirectoryPosition,52);
	getScalar(MetadataPosition,60);
	getScalar(UpdatePending,68);
	getScalar(AttachmentDirectoryPosition,72);
}

ImageFormat_ZISRAW::MetaData::MetaData(data::ByteArray &source, const size_t offset):Segment(source,offset){
	getScalar(XMLSize,0);
	getScalar(AttachmentSize,4);
	xml_data=_internal::getXML(data,256,XMLSize);
	// 			boost::property_tree::write_xml(std::cout,xml_data,boost::property_tree::xml_writer_settings<std::string>(' ',4));std::cout<<std::endl;
}

ImageFormat_ZISRAW::SubBlock::SubBlock(data::ByteArray &source, const size_t offset):Segment(source,offset){
	getScalar(MetadataSize,0);
	getScalar(AttachmentSize,4);
	getScalar(DataSize,8);
	DirectoryEntry=_internal::getDVEntry(data,16);
	size_t off=std::max(DirectoryEntry.size()+16,(size_t)256);
	xml_data=_internal::getXML(data,off,MetadataSize);
	// 			boost::property_tree::write_xml(std::cout,xml_data,boost::property_tree::xml_writer_settings<std::string>(' ',4));std::cout<<std::endl;

	image_data = data.at<uint8_t>(off+MetadataSize,DataSize);
}

size_t ImageFormat_ZISRAW::SubBlock::writeDimsInfo(util::PropertyMap &map)const{
	for(auto d:DirectoryEntry.dims){
		util::PropertyMap &ch_dim=map.touchBranch(d.Dimension.c_str());
		ch_dim.setValueAs("StartCoordinate",d.StartCoordinate);
		ch_dim.setValueAs("start",d.StartCoordinate);
		ch_dim.setValueAs("size",d.StartCoordinate);
		ch_dim.setValueAs("stored_size",d.StartCoordinate);
	}
	return DirectoryEntry.dims.size();
}

std::list<data::Chunk> ImageFormat_ZISRAW::SubBlock::makeChunks()const{
	// 			auto x_iter = std::find(DirectoryEntry.dims.begin(),DirectoryEntry.dims.end(),[](const DimensionEntry &d){return d.Dimension=="X";});
	// 			auto y_iter = std::find(DirectoryEntry.dims.begin(),DirectoryEntry.dims.end(),[](const DimensionEntry &d){return d.Dimension=="Y";});
	// 			auto z_iter = std::find(DirectoryEntry.dims.begin(),DirectoryEntry.dims.end(),[](const DimensionEntry &d){return d.Dimension=="Z";});
	// 			auto t_iter = std::find(DirectoryEntry.dims.begin(),DirectoryEntry.dims.end(),[](const DimensionEntry &d){return d.Dimension=="T";});
	// 			auto mosaic_iter = std::find(DirectoryEntry.dims.begin(),DirectoryEntry.dims.end(),[](const DimensionEntry &d){return d.Dimension=="M";});

	switch(DirectoryEntry.Compression){
		case 0:{
			// linear representation of the pixel data reinterpreted as the correct PixelType
			data::ValueArrayReference pixel_data=_internal::reinterpretData(image_data,DirectoryEntry.PixelType);//todo implement me
			throwGenericError("implement me ");
		}break;
		case 1:{throwGenericError("implement me jpeg");}break;//jpeg
		case 2:{throwGenericError("implement me LZW");}break;//LZW
		case 4:{
			static const bool verbose=false;
			uint8_t *img;size_t size[2];uint16_t type;
			jxr_decode(image_data.getRawAddress().get(),image_data.getLength(),(void**)&img,size, &type,verbose);
			
			auto typed=data::ByteArray(img,size[0]*size[1]).atByID(type,0);
			
			data::Chunk decoded(typed,size[0]/typed->bytesPerElem(),size[1],1,1,true);
			writeDimsInfo(decoded.touchBranch("dims"));
// 			data::IOFactory::write(decoded,"/tmp/test.png");
			return std::list<data::Chunk>(1,decoded);
		}
	}
}

ImageFormat_ZISRAW::Directory::Directory(data::ByteArray &source, const size_t offset):Segment(source,offset){
	int32_t cnt;
	getScalar(cnt,0);
	entries.resize(cnt);
	size_t e_offset=128;
	for(_internal::DirectoryEntryDV &dv:entries){
		dv=_internal::getDVEntry(data,e_offset);
		e_offset+=std::max(dv.size(),(size_t)128);
	}
	LOG(Runtime,info) << "Found dictionary with " << entries.size() << " entries";
}

std::list<data::Chunk> ImageFormat_ZISRAW::load(
		data::ByteArray source,
		std::list<util::istring> /*formatstack*/,
		std::list<util::istring> dialects,
		std::shared_ptr<util::ProgressFeedback> feedback
	)throw( std::runtime_error & ) {
	FileHeader header(source,0);
	if(header.id!="ZISRAWFILE")
		throwGenericError("could not find ZISRAWFILE header");

	//get MetaData if there are some
	if(header.MetadataPosition){
		MetaData(source,header.MetadataPosition);
	}

	if(header.DirectoryPosition){
		Directory directory(source,header.DirectoryPosition);
		std::list<data::Chunk> chunks;
		feedback->show(directory.entries.size(),std::string("Loading ")+std::to_string(directory.entries.size())+" data segments");
		for(const _internal::DirectoryEntryDV &e:directory.entries){
			const SubBlock s(source,e.FilePosition);
			chunks.splice(chunks.end(),s.makeChunks());
			feedback->progress();
		}

	} else {
		throwGenericError("could not find segment directory");
	}



	return std::list< data::Chunk >();
}
}}

isis::image_io::FileFormat *factory()
{
	isis_types.color.c24bit=isis::data::ValueArray<isis::util::color24>::staticID();
	isis_types.color.c48bit=isis::data::ValueArray<isis::util::color48>::staticID();
	isis_types.scalar.u8bit=isis::data::ValueArray<uint8_t>::staticID();
	isis_types.scalar.u16bit=isis::data::ValueArray<uint16_t>::staticID();
	isis_types.scalar.u32bit=isis::data::ValueArray<uint32_t>::staticID();
	isis_types.scalar.float32bit=isis::data::ValueArray<float>::staticID();
	return new isis::image_io::ImageFormat_ZISRAW();
}
