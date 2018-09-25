#include "imageFormat_ZISRAW.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/iostreams/stream.hpp>
#include <functional>
#include "imageFormat_ZISRAW_jxr.h"
#include <fstream>

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

util::PropertyMap getXML(data::ByteArray &data, size_t offset, size_t length, std::shared_ptr<std::ofstream> dump_stream){
	const uint8_t *start=data.begin()+offset;
	util::PropertyMap ret;
	if(dump_stream){
		dump_stream->write((char*)start,length) << std::endl;
	}
	ret.readXML(start,start+length,boost::property_tree::xml_parser::no_comments|boost::property_tree::xml_parser::trim_whitespace);
	return ret;
}

data::ValueArrayReference reinterpretData(const data::ByteArray &_data, int32_t PixelType){
	data::ValueArrayReference ret;
	data::ByteArray &data=const_cast<data::ByteArray &>(_data);
	switch(PixelType){
	case 0://Gray8 - no reinterpretation needed
		ret=data;break;
	case 1: //Gray16
        ret=data.at<uint16_t>(0,0,__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
	case 12: //Gray32
        ret=data.at<uint32_t>(0,0,__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
	case 2://Gray32Float
        ret=data.at<float>(0,0,__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
	case 3://Bgr24
		ret=color_reshuffle(data);break;
	case 4://Bgr48
        ret=color_reshuffle(data.at<uint16_t>(0,0,__BYTE_ORDER__==__ORDER_BIG_ENDIAN__));break;
	case 10: // Gray64ComplexFloat
        ret=data.at<std::complex<float>>(0,0,__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
	case 11: // Bgr192ComplexFloat
        ret=data.at<std::complex<double>>(0,0,__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);break;
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
	data=source.at<uint8_t>(offset+16+8+8,used_size);
}
size_t ImageFormat_ZISRAW::Segment::getSegmentSize(){
	return allocated_size+16+8+8;
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

ImageFormat_ZISRAW::MetaData::MetaData(data::ByteArray &source, const size_t offset, std::shared_ptr<std::ofstream> dump_stream):Segment(source,offset){
	getScalar(XMLSize,0);
	getScalar(AttachmentSize,4);
	xml_data=_internal::getXML(data,256,XMLSize,dump_stream);
}
util::PropertyMap ImageFormat_ZISRAW::MetaData::get()const{
	return xml_data;
}

ImageFormat_ZISRAW::SubBlock::SubBlock(data::ByteArray &source, const size_t offset, std::shared_ptr<std::ofstream> dump_stream):Segment(source,offset){
	getScalar(MetadataSize,0);
	getScalar(AttachmentSize,4);
	getScalar(DataSize,8);
	DirectoryEntry=_internal::getDVEntry(data,16);
	size_t off=std::max(DirectoryEntry.size()+16,(size_t)256);
	xml_data=_internal::getXML(data,off,MetadataSize,dump_stream);

	image_data = data.at<uint8_t>(off+MetadataSize,DataSize);
}

std::map<std::string,_internal::DimensionEntry> ImageFormat_ZISRAW::SubBlock::getDimsInfo()const{
	std::map<std::string,_internal::DimensionEntry> ret;
	for(auto d:DirectoryEntry.dims)
		ret[d.Dimension]=d;
	
	return ret;
}
std::array<size_t,4> ImageFormat_ZISRAW::SubBlock::getSize()const{
	std::map<std::string,_internal::DimensionEntry> map=getDimsInfo();
	std::array<size_t,4> ret={
		map["X"].StoredSize?:map["X"].size,
		map["Y"].StoredSize?:map["Y"].size,
		map["Z"].StoredSize?:map["Z"].size,
		1
	};
	if(!ret[2])ret[2]=1;
	
	return ret;
}
bool ImageFormat_ZISRAW::SubBlock::isNormalImage()const{
	return !DirectoryEntry.dims.empty();
	//xml_data.hasBranch("METADATA/Tags");//if there are tags, its a normal image
}
std::string ImageFormat_ZISRAW::SubBlock::getPlaneID()const{
	std::map<std::string,int32_t> dims;
	for(auto d:DirectoryEntry.dims){
		dims[d.Dimension]=d.start;
	}
	return 
		std::string("C")+(dims.find("C")!=dims.end() ? std::to_string(dims["C"]):"-")+
		std::string("Z")+(dims.find("Z")!=dims.end() ? std::to_string(dims["Z"]):"-")+
		std::string("T")+(dims.find("T")!=dims.end() ? std::to_string(dims["T"]):"-")+
		std::string("R")+(dims.find("R")!=dims.end() ? std::to_string(dims["R"]):"-")+
		std::string("S")+(dims.find("S")!=dims.end() ? std::to_string(dims["S"]):"-")+
		std::string("I")+(dims.find("I")!=dims.end() ? std::to_string(dims["I"]):"-")+
		std::string("H")+(dims.find("H")!=dims.end() ? std::to_string(dims["H"]):"-")+
		std::string("V")+(dims.find("V")!=dims.end() ? std::to_string(dims["V"]):"-")+
		(dims.find("M")!=dims.end() ? "M":"-");
}

data::Chunk ImageFormat_ZISRAW::SubBlock::jxrRead(size_t xsize,size_t ysize,isis::data::ByteArray image_data,unsigned short isis_type,unsigned short pixel_size){
	static const bool verbose=false;
	
	data::ByteArray buffer(xsize*ysize*pixel_size);
	
	jxr_decode(
		image_data.getRawAddress().get(),image_data.getLength(),
		buffer.getRawAddress().get(),&xsize,&ysize, 
		isis_type,verbose
	);
	
	return data::Chunk(buffer.atByID(isis_type,0,xsize*ysize),xsize,ysize,1,1,true);
}

std::function<data::Chunk()> ImageFormat_ZISRAW::SubBlock::getChunkGenerator()const{
	unsigned short isis_type,pixel_size;

	try{
		isis_type=PixelTypeMap.at(DirectoryEntry.PixelType);
		pixel_size=PixelSizeMap.at(DirectoryEntry.PixelType);
	} catch (std::out_of_range &){
		throwGenericError(std::string("Unsupportet pixel type ("+std::to_string(DirectoryEntry.PixelType)+")"));
	}
	
	std::function<data::Chunk(data::ByteArray image_data)> decoder;
	auto size=getSize();

	switch(DirectoryEntry.Compression){
		case 0:{
			auto pixel_type=DirectoryEntry.PixelType;
			// linear representation of the pixel data reinterpreted as the correct PixelType
			decoder = [size,pixel_type](isis::data::ByteArray image_data){
				data::ValueArrayReference ref=_internal::reinterpretData(image_data, pixel_type);
				return data::Chunk(ref,size[0],size[1],size[2],size[3],true);
			};
		}break;
		case 1:{throwGenericError("implement me jpeg");}break;//jpeg
		case 2:{throwGenericError("implement me LZW");}break;//LZW
		case 4:{
			if(DirectoryEntry.PixelType==10 || DirectoryEntry.PixelType==11)
				throwGenericError(std::string("Unsupportet pixel type ("+std::to_string(DirectoryEntry.PixelType)+") for compressed data"));

			decoder = [size,isis_type,pixel_size](data::ByteArray image_data){
				data::Chunk chk=jxrRead(size[0],size[1],image_data,isis_type,pixel_size);
				return chk;
			};
		}
	}
	return std::bind(decoder,image_data);
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

void ImageFormat_ZISRAW::storeProperties(data::Chunk &dst,std::string plane_id){
	//faking valid image
	dst.setValueAs( "plane",plane_id);
	dst.setValueAs( "indexOrigin", util::fvector3() );
	dst.setValueAs( "acquisitionNumber", 0 );
	dst.setValueAs( "voxelSize", util::fvector3({ 1, 1, 1 }) );
	dst.setValueAs( "rowVec", util::fvector3({1, 0} ));
	dst.setValueAs( "columnVec", util::fvector3({0, 1}) );
	dst.setValueAs( "sequenceNumber", ( uint16_t )0 );
}

data::Chunk ImageFormat_ZISRAW::transferFromMosaic(std::list<SubBlock> segments, unsigned short type_id,std::shared_ptr<util::ProgressFeedback> feedback){
	struct bounds{
		int32_t min=std::numeric_limits<int32_t>::max(),max=std::numeric_limits<int32_t>::min();
		size_t size()const{return max-min+1;}
	};
	struct {bounds x,y;} boundaries;
	
	for(auto &s:segments){
		for(const auto &d:s.DirectoryEntry.dims){
			if(d.Dimension[0]<'X' || d.Dimension[0]>'Y')continue; // we're only interested in X and Y here
			bounds &b=d.Dimension[0]=='X'?boundaries.x:boundaries.y;//select boundary by name
			const int scale = d.StoredSize?d.size/d.StoredSize:1;
			const int start = d.start/scale;
			const int end=start+d.StoredSize-1;
			if(b.min>start)b.min=start;
			if(b.max<end)b.max=end;
		}
	}

	auto dst=data::Chunk::createByID(type_id, boundaries.x.size(),boundaries.y.size(),1,1,true);
	assert(dst.getVolume());
	
	int32_t xoffset=-boundaries.x.min, yoffset=-boundaries.y.min;
	
	std::list<std::thread> jobs;
	for(auto &s:segments){
		auto dims=s.getDimsInfo();
		auto &X=dims["X"],&Y=dims["Y"];;
		const int xscale = X.StoredSize?X.size/X.StoredSize:1;
		const int yscale = Y.StoredSize?Y.size/Y.StoredSize:1;
		assert(X.start/xscale+xoffset>=0);
		assert(Y.start/yscale+yoffset>=0);

		const std::array<size_t,4> pos={
			size_t(X.start/xscale+xoffset),
			size_t(Y.start/yscale+yoffset),
			0,0
		};

		auto op = [&s,pos,&feedback,&dst](){
			data::Chunk c= s.getChunkGenerator()();
			dst.copyFromTile(c,pos,false);
			if(feedback)feedback->progress("",s.getSegmentSize());
		};
		jobs.emplace_back(op);
	}
	for(auto &j:jobs)
		j.join();
	return dst;
}

std::list<data::Chunk> ImageFormat_ZISRAW::load(
		data::ByteArray source,
		std::list<util::istring> /*formatstack*/,
		std::list<util::istring> dialects,
		std::shared_ptr<util::ProgressFeedback> feedback
	) {
		
	FileHeader header(source,0);
	if(header.id!="ZISRAWFILE")
		throwGenericError("could not find ZISRAWFILE header");
	
	std::list< data::Chunk > ret;
	std::shared_ptr<std::ofstream> dump_stream;
	
	if(checkDialect(dialects,"dump_xml")){
		LOG(Runtime,notice) << "Storing xml header data in /tmp/ZISRAW_dump.xml";
		dump_stream.reset(new std::ofstream("/tmp/ZISRAW_dump.xml"));
	}
	
	struct { 
		std::string name;
		struct {size_t x,y,z;}size;
		float pixel_size;
		struct {size_t layers;float factor;}pyramid;
		size_t mosaic_tiles;
		unsigned short type_id;
	}image_info;
	memset(&image_info,0,sizeof(image_info));

	//get MetaData if there are some
	if(header.MetadataPosition){
		util::PropertyMap meta=MetaData(source,header.MetadataPosition,dump_stream).get().branch("ImageDocument/Metadata");
// 		meta.print(std::cout)<< std::endl;
		util::PropertyMap image_props=meta.branch("Information/Image");
		image_info.size = {
			image_props.getValueAs<size_t>("SizeX"),
			image_props.getValueAs<size_t>("SizeY"),
			image_props.getValueAsOr<size_t>("SizeZ",1)
		};
		image_info.type_id = PixelTypeMapStr.at(image_props.getValueAs<std::string>("PixelType"));
		
		if(image_props.getValueAs<size_t>("SizeS")>1){
			throwGenericError("Sorry, multi scene images are not supportet");
		}
		
		auto scene=meta.queryBranch("Information/Image/Dimensions/S/Scenes/Scene");
		if(scene){
			image_info.name=scene->getValueAs<std::string>("<xmlattr>/Name");
			auto pyramid=scene->queryBranch("PyramidInfo");
			if(pyramid){
				image_info.pyramid.layers=pyramid->getValueAs<size_t>("PyramidLayersCount");
				image_info.pyramid.factor=pyramid->getValueAs<float>("MinificationFactor");
			}
		}
		if(image_props.hasProperty("SizeM"))
			image_info.mosaic_tiles=image_props.getValueAs<size_t>("SizeM");
		
		image_info.pixel_size = meta.getValueAsOr<float>("Scaling/Items/Distance/Value",1./1000)*1000;
		
	} else 
		throwGenericError("could not find metadata");
	
	LOG_IF(image_info.pyramid.layers>0,Runtime,info) 
		<< "Header says its a " << image_info.size.x << "*" << image_info.size.y 
		<< " pyramid image with " << image_info.pyramid.layers << " subsampled layers";
	LOG_IF(image_info.pyramid.layers==0,Runtime,info) 
		<< "Header says its a " << image_info.size.x << "*" << image_info.size.y << " image";

	std::vector<std::list<SubBlock>> pyramid(image_info.pyramid.layers+1);

	//generate planes
	if(header.DirectoryPosition){
		Directory directory(source,header.DirectoryPosition);

		struct bounds{
			int32_t min=std::numeric_limits<int32_t>::max(),max=std::numeric_limits<int32_t>::min();
			size_t size()const{return max-min+1;}
		};
		struct plane{
			std::list<SubBlock> segments;
			std::map<std::string,bounds> boundaries;
		};
		std::map<std::string,plane> planes;

		for(const _internal::DirectoryEntryDV &e:directory.entries){
			if(e.PyramidType){
				int scale = e.dims[0].size / e.dims[0].StoredSize;
				int scaleFactor= image_info.pyramid.factor;
				assert(scaleFactor>1);
				int level = std::log10(scale)/std::log10(scaleFactor);
				LOG(Runtime,verbose_info) 
					<< "Got Pyramid segment " << std::make_pair(e.dims[0].StoredSize,e.dims[1].StoredSize) 
					<< "-Image at level " << level << " (scale: " << scale << ")";
						
				pyramid[level].emplace_back(source,e.FilePosition,dump_stream);
				assert(pyramid[level].back().isNormalImage());
			} else {
				LOG(Runtime,verbose_info) 
					<< "Got base segment " << std::make_pair(e.dims[0].StoredSize,e.dims[1].StoredSize) << "-Image";
				pyramid[0].emplace_back(source,e.FilePosition,dump_stream);
				assert(pyramid[0].back().isNormalImage());
			}
		}
		
		LOG(Runtime,info) << "Found " << pyramid.size() << " level Pyramid";
		std::array<size_t, 2> size{image_info.size.x,image_info.size.y};
		for(size_t i=0;i<pyramid.size();i++){
			assert(!pyramid[i].empty());
			const size_t estimated_size=util::product(size)*PixelSizeMap.at(pyramid[i].front().DirectoryEntry.PixelType)/(1024*1024);
			if(checkDialect(dialects,"max16G") && estimated_size> 16*1024){
				LOG(Runtime,notice) << "Skipping " << size << " image as its resulting in-memory size " << std::to_string(estimated_size)+"MB" << " would exceed the limit of 16G";
			}else if(checkDialect(dialects,"max8G") && estimated_size> 8*1024){
				LOG(Runtime,notice) << "Skipping " << size << " image as its resulting in-memory size " << std::to_string(estimated_size)+"MB" << " would exceed the limit of 8G";
			} else if(checkDialect(dialects,"max4G") && estimated_size> 4*1024){
				LOG(Runtime,notice) << "Skipping " << size << " image as its resulting in-memory size " << std::to_string(estimated_size)+"MB" << " would exceed the limit of 4G";
			} else {
				ret.push_back(transferFromMosaic(pyramid[i],image_info.type_id,feedback));
				LOG(Runtime,info) <<  ret.back().getSizeAsString() << " Image created for pyramid level " << i << " from " << pyramid[i].size() << " tiles";

				ret.back().touchBranch("XML").transfer(pyramid[i].front().xml_data);
				
				const float pixel_size=image_info.pixel_size*std::pow(image_info.pyramid.factor,i);
				ret.back().setValueAs<util::fvector3>("voxelSize",{pixel_size,pixel_size,1});
				ret.back().setValueAs("pyramidLevel",i);
				ret.back().setValueAs("sequenceNumber",i);
			}

			for(size_t &s:size)
				s=std::ceil(double(s)/image_info.pyramid.factor);
		}
		return ret;

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
const std::map<std::string,uint16_t> isis::image_io::ImageFormat_ZISRAW::PixelTypeMapStr={
	 {"Gray8",isis::data::ValueArray<uint8_t>::staticID()} //Gray8
	,{"Gray16",isis::data::ValueArray<uint16_t>::staticID()} //Gray16
	,{"Gray32Float",isis::data::ValueArray<float>::staticID()} //Gray32Float
	,{"Bgr24",isis::data::ValueArray<util::color24>::staticID()} //Bgr24
	,{"Bgr48",isis::data::ValueArray<util::color48>::staticID()} //Bgr48
	,{"Gray64ComplexFloat",isis::data::ValueArray<std::complex<float>>::staticID()} //Gray64ComplexFloat
	,{"Bgr192ComplexFloat",isis::data::ValueArray<std::complex<double>>::staticID()} //Bgr192ComplexFloat
	,{"Gray32",isis::data::ValueArray<int32_t>::staticID()} //Gray32
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
