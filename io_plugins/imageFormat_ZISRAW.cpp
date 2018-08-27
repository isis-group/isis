#include "imageFormat_ZISRAW.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/iostreams/stream.hpp>
#include <functional>
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
        ret=data.at<uint16_t>(0,data.getLength(),__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
	case 12: //Gray32
        ret=data.at<uint32_t>(0,data.getLength(),__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
	case 2://Gray32Float
        ret=data.at<float>(0,data.getLength(),__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
	case 3://Bgr24
		ret=color_reshuffle<uint8_t>(data);break;
	case 4://Bgr48
        ret=color_reshuffle<uint16_t>(data.at<uint16_t>(0,data.getLength(),__BYTE_ORDER__==__ORDER_BIG_ENDIAN__));break;
	case 10: // Gray64ComplexFloat
        ret=data.at<std::complex<float>>(0,data.getLength(),__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
	case 11: // Bgr192ComplexFloat
        ret=data.at<std::complex<double>>(0,data.getLength(),__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
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
    auto buff=source.at<uint64_t>(offset+16,2,__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);
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
// 	boost::property_tree::write_xml(std::cerr,xml_data,boost::property_tree::xml_writer_settings<std::string>(' ',4));std::cerr<<std::endl;

	image_data = data.at<uint8_t>(off+MetadataSize,DataSize);
}

size_t ImageFormat_ZISRAW::SubBlock::writeDimsInfo(util::PropertyMap &map)const{
	for(auto d:DirectoryEntry.dims){
		util::PropertyMap &ch_dim=map.touchBranch(d.Dimension.c_str());
		ch_dim.setValueAs("StartCoordinate",d.StartCoordinate);
		ch_dim.setValueAs("start",d.start);
		ch_dim.setValueAs("size",d.size);
		ch_dim.setValueAs("stored_size",d.StoredSize);
	}
	return DirectoryEntry.dims.size();
}
bool ImageFormat_ZISRAW::SubBlock::isNormalImage()const{
	auto got=xml_data.get_optional<std::string>("METADATA.Tags");
	return got.operator bool();//if there are tags, its a normal image
}

data::Chunk ImageFormat_ZISRAW::SubBlock::jxrRead(util::PropertyMap dims,isis::data::ByteArray image_data,unsigned short isis_type,unsigned short pixel_size){
	static const bool verbose=false;
	const uint32_t xsize=dims.getValueAs<uint32_t>("X/stored_size"),ysize=dims.getValueAs<uint32_t>("Y/stored_size");
	
	data::ByteArray buffer(xsize*ysize*pixel_size);
	jxr_decode(
		image_data.getRawAddress().get(),image_data.getLength(),
		buffer.getRawAddress().get(),buffer.getLength(), 
		isis_type,verbose);
	
	data::Chunk decoded(buffer.atByID(isis_type,0),xsize,ysize);
	decoded.touchBranch("dims").transfer(dims);
	return decoded;
}

std::future<std::list<data::Chunk>> ImageFormat_ZISRAW::SubBlock::makeChunks()const{
	unsigned short isis_type,pixel_size;

	try{
		isis_type=PixelTypeMap.at(DirectoryEntry.PixelType);
		pixel_size=PixelSizeMap.at(DirectoryEntry.PixelType);
	} catch (std::out_of_range &){
		throwGenericError(std::string("Unsupportet pixel type ("+std::to_string(DirectoryEntry.PixelType)+")"));
	}
	
	std::function<std::list<data::Chunk>(isis::data::ByteArray image_data)> decoder;
	std::launch decode_policy;
	
	switch(DirectoryEntry.Compression){
		case 0:{
			// linear representation of the pixel data reinterpreted as the correct PixelType
			data::ValueArrayReference pixel_data=_internal::reinterpretData(image_data,DirectoryEntry.PixelType);//todo implement me
			throwGenericError("implement me ");
		}break;
		case 1:{throwGenericError("implement me jpeg");}break;//jpeg
		case 2:{throwGenericError("implement me LZW");}break;//LZW
		case 4:{
			if(DirectoryEntry.PixelType==10 || DirectoryEntry.PixelType==11)
				throwGenericError(std::string("Unsupportet pixel type ("+std::to_string(DirectoryEntry.PixelType)+") for compressed data"));

			util::PropertyMap dims;
			writeDimsInfo(dims);
			
			decoder = [dims,isis_type,pixel_size](isis::data::ByteArray image_data){
				return std::list<data::Chunk>(1,jxrRead(dims,image_data,isis_type,pixel_size));
			};
			decode_policy=std::launch::async;
		}
	}
	return std::async(decode_policy, std::bind(decoder,image_data));
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
		std::list<std::future<std::list<data::Chunk>>> segments;
		struct bounds{
			int32_t min=std::numeric_limits<int32_t>::max(),max=std::numeric_limits<int32_t>::min();
			size_t size()const{return max-min+1;}
		};
		std::map<std::string,bounds> boundaries;
		feedback->show(directory.entries.size(),std::string("Loading ")+std::to_string(directory.entries.size())+" data segments");

		for(const _internal::DirectoryEntryDV &e:directory.entries){
			const SubBlock s(source,e.FilePosition);
			if(s.isNormalImage()){
				
				// find boundaries (not existing dimsions will not be registered)
				for(const auto &d:e.dims){
					bounds &b=boundaries[d.Dimension];//select boundary by name
					const int end=d.start+d.StoredSize-1;
					if(b.min>d.start)b.min=d.start;
					if(b.max<end)b.max=end;//TODO shouldn't that be size and how do we deal with this
				}
				segments.push_back(s.makeChunks());
				feedback->progress();
// 				chunks.back().branch("dims").print(std::cerr) << std::endl;
			} else {
			}
		}
		LOG(Runtime,info) << "Got " << segments.size() << " Image objects";
		LOG(Runtime,verbose_info) << "Dimensions from the header are:";
		for(const auto &b:boundaries){
			LOG(Runtime,verbose_info) << b.first << ":" << std::make_pair(b.second.min,b.second.max) << " size: " << b.second.size();
		}
		
		if(boundaries["M"].size()>1){
			assert(boundaries["M"].size()==segments.size());
// 			segments.sort([](const data::Chunk &c1,const data::Chunk &c2){//sorting the segments by index M
// 				return c1.property("dims/M/start").lt(c2.property("dims/M/start"));}
// 			);


			std::unique_ptr<data::Chunk> dst;
			struct {int32_t x,y;}offset={-boundaries["X"].min,-boundaries["Y"].min};
			while(!segments.empty()){
				for(const data::Chunk &c:segments.front().get()){
					if(!dst){ // create dst on first iteration from first segment @todo this is ugly as f**k
						dst.reset(new data::Chunk(c.cloneToNew(boundaries["X"].size(),boundaries["Y"].size())));
						LOG(Runtime,info) << "Creating " << dst->getSizeAsString() << " image from " << segments.size() << " segments";
					}
					
					assert(c.getValueAs<int32_t>("dims/X/start")+offset.x>=0);
					assert(c.getValueAs<int32_t>("dims/Y/start")+offset.y>=0);
					const std::array<size_t,4> pos={
						size_t(c.getValueAs<int32_t>("dims/X/start")+offset.x),
						size_t(c.getValueAs<int32_t>("dims/Y/start")+offset.y),
						1,1
					};
					dst->copyFromTile(c,pos,false);
				}
				segments.pop_front();//get rid of segments we dont need anymore
			}
			
			//faking valid image
			dst->setValueAs( "indexOrigin", util::fvector3() );
			dst->setValueAs( "acquisitionNumber", 0 );
			dst->setValueAs( "voxelSize", util::fvector3({ 1, 1, 1 }) );
			dst->setValueAs( "rowVec", util::fvector3({1, 0} ));
			dst->setValueAs( "columnVec", util::fvector3({0, 1}) );
			dst->setValueAs( "sequenceNumber", ( uint16_t )0 );

			return std::list< data::Chunk >(1,*dst);
		}

	} else {
		throwGenericError("could not find segment directory");
	}


	return std::list< data::Chunk >();
}
}}

const std::map<uint32_t,uint16_t> isis::image_io::ImageFormat_ZISRAW::PixelTypeMap={
	 {0,isis::data::ValueArray<uint8_t>::staticID()} //Gray8
	,{1,isis::data::ValueArray<uint16_t>::staticID()} //Gray16
	,{2,isis::data::ValueArray<float>::staticID()} //Gray32Float
	,{3,isis::data::ValueArray<util::color24>::staticID()} //Bgr24
	,{4,isis::data::ValueArray<util::color48>::staticID()} //Bgr48
	,{10,isis::data::ValueArray<std::complex<float>>::staticID()} //Gray64ComplexFloat
	,{11,isis::data::ValueArray<std::complex<double>>::staticID()} //Bgr192ComplexFloat
	,{12,isis::data::ValueArray<int32_t>::staticID()} //Gray32
};
const std::map<uint32_t,uint16_t> isis::image_io::ImageFormat_ZISRAW::PixelSizeMap={
	 {0,sizeof(uint8_t)} //Gray8
	,{1,sizeof(uint16_t)} //Gray16
	,{2,sizeof(float)} //Gray32Float
	,{3,sizeof(util::color24)} //Bgr24
	,{4,sizeof(util::color48)} //Bgr48
	,{10,sizeof(std::complex<float>)} //Gray64ComplexFloat
	,{11,sizeof(std::complex<double>)} //Bgr192ComplexFloat
	,{12,sizeof(int32_t)} //Gray32
};

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
