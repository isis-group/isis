#include <isis/data/io_factory.hpp>
#include <stdint.h>
#include <stdio.h>
#include <turbojpeg.h>
#include "imageFormat_tiff_sa.hpp"
#include <memory>

// https://www.itu.int/itudoc/itu-t/com16/tiff-fx/docs/tiff6.pdf
// http://www.awaresystems.be/imaging/tiff/bigtiff.html

namespace isis{
namespace image_io{
	
namespace _internal {
	
	typedef std::function<data::Chunk(data::ValueArray<uint8_t> tile,std::array<uint64_t,2> tilesize)> transfer_func;
	
	transfer_func jpeg_transfer = [](data::ValueArray<uint8_t> tile_src, std::array<uint64_t,2> tilesize){
		data::MemChunk<util::color24> tile_buff(tilesize[0],tilesize[1]);
		assert(tile_buff.getBytesPerVoxel()==tjPixelSize[TJPF_RGB]);
		auto decompressor=tjInitDecompress();
		auto *p_tile_buff= std::static_pointer_cast<unsigned char>(tile_buff.asValueArrayBase().getRawAddress()).get();
		const int err=tjDecompress2(
			decompressor,
			tile_src.begin(),tile_src.getLength(),
			p_tile_buff,
			tilesize[0],tilesize[0]*tjPixelSize[TJPF_RGB],tilesize[1],
			TJPF_RGB,
			0
		);
		if(err){
			LOG(Runtime,error) << "jpeg decompression failed with " << tjGetErrorStr();
			#warning implement error handling
		} 
		return tile_buff;
	};

	template<typename T> transfer_func make_direct_transfer(){
		return [](data::ValueArray<uint8_t> tile_src, std::array<uint64_t,2> tilesize){
			auto dstsize=tilesize[0]*tilesize[1];
			assert(tile_src.getLength()==dstsize*sizeof(T));
			data::ValueArray<T> casted_src(std::static_pointer_cast<T>(tile_src.getRawAddress()),dstsize);
			return data::Chunk(casted_src,tilesize[0],tilesize[1]);;
		};
	};

	enum compression_type{ //http://www.awaresystems.be/imaging/tiff/tifftags/compression.html
		none = 1, 
		ccittrle = 2, ccittfax3 = 3, ccittfax4 = 4,
		lzw = 5,
		ojpeg = 6, jpeg = 7,
		next = 32766,
		ccittrlew = 32771,
		packbits = 32773,
		thunderscan = 32809,
		it8ctpad = 32895,it8lw = 32896,it8mp = 32897,it8bl = 32898,
		pixarfilm = 32908,pixarlog = 32909,
		deflate = 32946,adobe_deflate = 8,
		dcs = 32947,
		jbig = 34661,
		sgilog = 34676, sgilog24 = 34677,
		jp2000 = 34712
	};
	enum photometric_type { // http://www.awaresystems.be/imaging/tiff/tifftags/photometricinterpretation.html
		miniswhite = 0,minisblack = 1,
		rgb = 2,
		palette = 3,
		mask = 4,
		separated = 5,
		ycbcr = 6,
		cielab = 8, icclab = 9,itulab = 10,
		logl = 32844,logluv = 32845
	};
	
	static const std::map<uint16_t,util::istring> tag_registry={
		{254,"NewSubfileType"},// general indication of the kind of data contained in this subfile.
		{255,"SubfileType"}, //A general indication of the kind of data contained in this subfile.
		{256,"ImageWidth"}, //The number of columns in the image, i.e., the number of pixels per row.
		{257,"ImageLength"}, //The number of rows of pixels in the image.
		{258,"BitsPerSample"}, //Number of bits per component.
		{259,"Compression"}, //Compression scheme used on the image data.
		{262,"PhotometricInterpretation"}, //The color space of the image data.
		{263,"Threshholding"}, //For black and white TIFF files that represent shades of gray, the technique used to convert from gray to black and white pixels.
		{264,"CellWidth"}, //The width of the dithering or halftoning matrix used to create a dithered or halftoned bilevel file.
		{265,"CellLength"}, //The length of the dithering or halftoning matrix used to create a dithered or halftoned bilevel file.
		{266,"FillOrder"}, //The logical order of bits within a byte.
		{270,"ImageDescription"}, //A string that describes the subject of the image.
		{271,"Make"}, //The scanner manufacturer.
		{272,"Model"}, //The scanner model name or number.
		{273,"StripOffsets"}, //For each strip, the byte offset of that strip.
		{274,"Orientation"}, //The orientation of the image with respect to the rows and columns.
		{277,"SamplesPerPixel"}, //The number of components per pixel.
		{278,"RowsPerStrip"}, //The number of rows per strip.
		{279,"StripByteCounts"}, //For each strip, the number of bytes in the strip after compression.
		{280,"MinSampleValue"}, //The minimum component value used.
		{281,"MaxSampleValue"}, //The maximum component value used.
		{282,"XResolution"}, //The number of pixels per ResolutionUnit in the ImageWidth direction.
		{283,"YResolution"}, //The number of pixels per ResolutionUnit in the ImageLength direction.
		{284,"PlanarConfiguration"}, //How the components of each pixel are stored.
		{288,"FreeOffsets"}, //For each string of contiguous unused bytes in a TIFF file, the byte offset of the string.
		{289,"FreeByteCounts"}, //For each string of contiguous unused bytes in a TIFF file, the number of bytes in the string.
		{290,"GrayResponseUnit"}, //The precision of the information contained in the GrayResponseCurve.
		{291,"GrayResponseCurve"}, //For grayscale data, the optical density of each possible pixel value.
		{296,"ResolutionUnit"}, //The unit of measurement for XResolution and YResolution.
		{305,"Software"}, //Name and version number of the software package(s) used to create the image.
		{306,"DateTime"}, //Date and time of image creation.
		{315,"Artist"}, //Person who created the image.
		{316,"HostComputer"}, //The computer and/or operating system in use at the time of image creation.
		{320,"ColorMap"}, //A color map for palette color images.
		{338,"ExtraSamples"}, //Description of extra components.
		{33432,"Copyright"} //Copyright notice.
	};

	template<typename T> util::PropertyValue getPropVal_impl(data::ByteArray &source,bool byteswap,uint64_t offset, uint64_t number_of_values){
		util::PropertyValue ret;
		for(auto val:source.at<T>(offset,number_of_values,byteswap))
			ret.push_back(val);
		return ret;
	}

	template<> util::PropertyValue getPropVal_impl<std::string>(data::ByteArray &source,bool byteswap,uint64_t offset, uint64_t number_of_values){
		const char *str=(char*)source.at<uint8_t>(offset,number_of_values,byteswap).getRawAddress().get();
		return util::Value<std::string>(str);
	}
	
	//rational (Two LONGs: the first represents the numerator of a fraction; the second, the denominator.)
	template<> util::PropertyValue getPropVal_impl<double>(data::ByteArray &source,bool byteswap,uint64_t offset, uint64_t number_of_values){
		util::PropertyValue ret=getPropVal_impl<uint32_t>(source,byteswap,offset,number_of_values*2);
		if(number_of_values==1){
			return util::Value<double>(ret[0].as<double>()/ret[1].as<double>());
		} else {
			util::Value<util::dlist> values;
			for(int i=0;i<number_of_values*2;i+=2)
				static_cast<util::dlist&>(values).push_back(ret[i].as<double>()/ret[i+1].as<double>());
			return values;
		}
	}

	class TiffSource:protected data::ByteArray{
		bool m_big_tiff=false, m_byteswap=false;
		uint64_t m_offset=0;
		
	public:
		template<typename T> ValueArray<T> at( uint64_t offset, uint64_t len = 0){
			return data::ByteArray::at<T>(offset,len,m_byteswap);
		}
		void seek(uint64_t _offset){m_offset=_offset;}
		template<typename T> T readVal(){
			T ret=at<T>(m_offset,1)[0];
			m_offset+=sizeof(T);
			return ret;
		}
		template<typename T=uint16_t> uint64_t readValAuto(){
			return m_big_tiff ? readVal<uint64_t>():readVal<T>();
		}
		TiffSource(data::ByteArray source):ByteArray(source){
			switch(readVal<uint16_t>()){
				case 0x4949://little endian
					m_byteswap=(__BYTE_ORDER__==__ORDER_BIG_ENDIAN__);
					break; 
				case 0x4D4D: // big endian
					m_byteswap=(__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__);
					break;
				default: // wrong magic
					FileFormat::throwGenericError("this is no tiff file");
					break;
			}
			
			const uint16_t magic=readVal<uint16_t>();
			switch(magic){
				case 43:{ // BigTIFF
					const uint16_t off_size=readVal<uint16_t>();
					LOG_IF(off_size!=8, Runtime, error) << "Unexpected offset value size " << off_size << " (should be 8)";
					if(readVal<uint16_t>()!=0)
						FileFormat::throwGenericError("broken BigTIFF file");
					m_big_tiff=true;
				}break;
				case 42:break;
				default:
					FileFormat::throwGenericError("this is no tiff file (invalid version: " + std::to_string(magic) + " should be 42 or 43)");
			}
		}
		template<typename T> util::PropertyValue getPropVal(){
			const uint64_t number_of_values= readValAuto<uint32_t>();
			const uint64_t projected_size=number_of_values*sizeof(T);
			
			if((projected_size<=4) || (m_big_tiff && projected_size <= 8)) // inlined data
				return getPropVal_impl<T>(*this,m_byteswap,m_offset,number_of_values);
			else{
				return getPropVal_impl<T>(*this,m_byteswap,readValAuto<uint32_t>(),number_of_values);
			}
		}
		std::pair<uint16_t,util::PropertyValue> readTag(){
			uint64_t next=m_offset+ (m_big_tiff?20:12);
			std::pair<uint16_t,util::PropertyValue> ret;
			ret.first =readVal<uint16_t>();
			const uint16_t tag_type=readVal<uint16_t>();

			switch(tag_type){
				case 1:ret.second=getPropVal<uint8_t>();break;
				case 2:ret.second=getPropVal<std::string>();break;
				case 3:ret.second=getPropVal<uint16_t>();break;
				case 4:ret.second=getPropVal<uint32_t>();break;
				case 5:ret.second=getPropVal<double>();break;
				case 6:ret.second=getPropVal<int8_t>();break;
				case 7:ret.second=getPropVal<std::string>();break;//actually raw bytes
				case 8:ret.second=getPropVal<int16_t>();break;
				case 9:ret.second=getPropVal<int32_t>();break;
				case 16:ret.second=getPropVal<uint64_t>();break;
				case 17:ret.second=getPropVal<int64_t>();break;
				default:
					LOG(Runtime,warning) << "Invalid type " << tag_type << " when reading tag " << ret.first;
			}
			seek(next);
			LOG(Debug,verbose_info) << "Red tag " << ret.first << " as " << ret.second;
			return ret;
		}

	};
	
	
	class IFD{
		std::array<uint64_t,2> size,tilesize;
		std::array<float,2> pixels_per_unit;
		std::list<uint64_t> stripoffsets,stripbytecounts,tileoffsets,tilebytecounts,bitspersample;
		std::list<data::ValueArray<uint8_t>> tiles,strips;
		enum unit_types{none=1,inch,centimeter}resolution_unit=inch;
		util::PropertyMap generic_props;
		compression_type compression;
		photometric_type photometricinterpretation;
		uint16_t 
			planar_configuration=1, //http://www.awaresystems.be/imaging/tiff/tifftags/planarconfiguration.html
			samples_per_pixel;
		uint64_t rowsperstrip;
	public:
		size_t countProgress()const{return stripoffsets.size()+tiles.size();}
		void readTAG(TiffSource &source){
			std::pair<uint16_t,util::PropertyValue> tag=source.readTag();
			switch(tag.first){
				case 0x100:size[0]=tag.second.as<uint64_t>();break;//ImageWidth
				case 0x101:size[1]=tag.second.as<uint64_t>();break;//ImageLength
				case 0x102://TileOffsets
					for(const auto &off:tag.second)
						bitspersample.push_back(off.as<uint64_t>());
					break; 
				case 0x103:compression =(compression_type) tag.second.as<uint16_t>();break;//Compression
				case 0x106:photometricinterpretation = (photometric_type)tag.second.as<uint16_t>();break;//PhotometricInterpretation
				
				case 0x111://StripOffsets
					for(const auto &off:tag.second)
						stripoffsets.push_back(off.as<uint64_t>());
					break; 
				case 0x115:samples_per_pixel=tag.second.as<uint64_t>();break;
				case 0x116:rowsperstrip=tag.second.as<uint64_t>();break;
				case 0x117://StripByteCounts
					for(const auto &off:tag.second)
						stripbytecounts.push_back(off.as<uint64_t>());
					break; 

				case 0x128:resolution_unit=(unit_types)tag.second.as<uint16_t>();break;
				case 0x11A:pixels_per_unit[0]=tag.second.as<float>();break;
				case 0x11B:pixels_per_unit[1]=tag.second.as<float>();break;
					
				case 0x142:tilesize[0]=tag.second.as<uint64_t>();break;
				case 0x143:tilesize[1]=tag.second.as<uint64_t>();break;
				case 0x144://TileOffsets
					for(const auto &off:tag.second)
						tileoffsets.push_back(off.as<uint64_t>());
					break; 
				case 0x145://TileOffsets
					for(const auto &off:tag.second)
						tilebytecounts.push_back(off.as<uint64_t>());
					break;
				case 0x011C:
					planar_configuration=tag.second.as<uint16_t>();break;
				default:{
					auto found =_internal::tag_registry.find(tag.first);
					if(found!=_internal::tag_registry.end())
						generic_props.touchProperty(found->second).transfer(tag.second);
					else
						generic_props.touchProperty(std::to_string(tag.first).c_str()).transfer(tag.second);
				}break;
			}
		}

		IFD(TiffSource &source){
			uint64_t number_of_tags= source.readValAuto();
			util::DefaultMsgPrint::stopBelow(warning);
			
			for(uint16_t i=0;i<number_of_tags;i++){
				readTAG(source);
			}

			assert(tilebytecounts.size()==tileoffsets.size());
			while(!tilebytecounts.empty()){
				data::ValueArray<uint8_t> tile=source.at<uint8_t>(tileoffsets.front(),tilebytecounts.front());
				tiles.push_back(tile);
				tileoffsets.pop_front();
				tilebytecounts.pop_front();
			}
			while(!stripbytecounts.empty()){
				data::ValueArray<uint8_t> tile=source.at<uint8_t>(stripoffsets.front(),stripbytecounts.front());
				strips.push_back(tile);
				stripoffsets.pop_front();
				stripbytecounts.pop_front();
			}
		}
		size_t computeSize()const{
			return util::product(size)*3;
		}
		
		void readTiles(
			data::Chunk &dst,std::list<data::ValueArray<uint8_t>> tiles,
			std::array<uint64_t,2> tilesize,
			transfer_func transfer,
			std::shared_ptr<util::ProgressFeedback> feedback
		)const{
				for (uint64_t y = 0; y < size[1]; y += tilesize[1]){
					for (uint64_t x = 0; x < size[0]; x += tilesize[0]){
						assert(!tiles.empty());
						
						data::Chunk tile_buff=transfer(tiles.front(),tilesize);
						dst.copyFromTile(tile_buff,{x,y}); 

						if(feedback)
							feedback->progress();
						tiles.pop_front();
					}
				}
		}
		
		data::Chunk makeChunk(std::shared_ptr<util::ProgressFeedback> feedback)const{
			std::unique_ptr<data::Chunk> ret;
			
			transfer_func transfer;
			
			if(samples_per_pixel==1){ // scalar interpretation
				switch(bitspersample.front()){
					case 1:
						ret.reset(new data::MemChunk<bool>(size[0],size[1],1,1,true));
						break;
					case 8:
						ret.reset(new data::MemChunk<uint8_t>(size[0],size[1],1,1,true));
						break;
					case 16:
						ret.reset(new data::MemChunk<uint16_t>(size[0],size[1],1,1,true));
						transfer=_internal::make_direct_transfer<uint16_t>();
						break;
					default:
						LOG(Runtime,error) << "Unsupportet bit depth " << bitspersample.front();
				}
			} else if(samples_per_pixel==3){ //rgb interpretation
				ret.reset(new data::MemChunk<util::color24>(size[0],size[1],1,1,true));
				transfer=_internal::jpeg_transfer;
			} else {
				LOG(Runtime,error) << "Unsupported samples per pixel " << samples_per_pixel;
			}
			
			ret->touchBranch("TIFF")=generic_props;
			
			//todo deal with those
			auto YCbCrSubSampling = extractOrTell("530",ret->touchBranch("TIFF"),info);// http://www.awaresystems.be/imaging/tiff/tifftags/ycbcrsubsampling.html
			auto ReferenceBlackWhite = extractOrTell("532",ret->touchBranch("TIFF"),info);// http://www.awaresystems.be/imaging/tiff/tifftags/referenceblackwhite.html

			if(pixels_per_unit[0] && pixels_per_unit[1]){
				auto &voxelsize= ret->touchProperty("voxelsize");
				switch(resolution_unit){
					case inch:
						voxelsize = util::fvector3{25.4/pixels_per_unit[0],25.4/pixels_per_unit[1],1};
						break;
					case centimeter:
						voxelsize = util::fvector3{10/pixels_per_unit[0],10/pixels_per_unit[1],1};
						break;
					case none:
					default:
						LOG(Runtime,warning) << "ignoring resolution "<< pixels_per_unit << " because no resolution type is given";
						break;
				}
			}

			
			if(!strips.empty()){
				std::cout << "stripoffsets" << stripoffsets << std::endl;
				LOG(Runtime,info) 
					<< "Loading " << ret->getSizeAsString() << "-Image from " << stripoffsets.size() << " stripes (" 
					<< ret->getVolume()*ret->getBytesPerVoxel() / 1024 / 1024 << "MB)";
					
				readTiles(*ret,strips,{size[0],rowsperstrip},transfer,feedback); // stripes are essentially tiles as wide as the image
			} else if(!tiles.empty()) {
				LOG(Runtime,info) 
					<< "Reading " << ret->getSizeAsString() << "-Image from " << tiles.size() << " tiles (" 
					<< ret->getVolume()*ret->getBytesPerVoxel() / 1024 / 1024 << "MB)";
					
				readTiles(*ret,tiles,tilesize,transfer,feedback);
			}
			return *ret;
		}
	};
}

util::istring ImageFormat_TiffSa::suffixes(isis::image_io::FileFormat::io_modes /*modes*/) const{return ".tiff .tif .scn";}

std::list< data::Chunk > ImageFormat_TiffSa::load(
	data::ByteArray source, 
	std::list<util::istring> /*formatstack*/, 
	std::list<util::istring> dialects, 
	std::shared_ptr<util::ProgressFeedback> feedback
)throw( std::runtime_error & ) {
	_internal::TiffSource tiff(source);
	
	
	uint64_t next_ifd = tiff.readValAuto<uint32_t>();
	
	std::list<_internal::IFD> images;
	
	do{	
		tiff.seek(next_ifd);
		images.push_back(_internal::IFD(tiff));
	}while((next_ifd = tiff.readValAuto<uint32_t>()));
	
	std::list<data::Chunk> ret;
	int nr=0;
	for(const _internal::IFD& ifd:images){
		size_t mbsize=ifd.computeSize() / 1024 / 1024;
		if(mbsize >= 4096 && checkDialect(dialects,"lowmem"))
			LOG(Runtime,warning) << "Skipping " << mbsize << "MB image because of lowmem dialect";
		else {
			auto ch=ifd.makeChunk(feedback);
			ch.setValueAs("sequenceNumber",nr++);
			ret.push_back(ch);
		}
	}
	
	return ret;
}

std::string ImageFormat_TiffSa::getName() const{return "tiff";}

std::list<util::istring> ImageFormat_TiffSa::dialects() const {return {"lowmem"};}

void ImageFormat_TiffSa::write(const data::Image &/*image*/, const std::string &/*filename*/, std::list<util::istring> /*dialects*/, std::shared_ptr<util::ProgressFeedback> /*feedback*/){
	throwGenericError("not yet implemented");
}


}}

isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_TiffSa();
}
