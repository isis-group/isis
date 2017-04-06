#include "imageFormat_tiff_sa.hpp"
#include <stdint.h>
#include <jpeglib.h>

// https://www.itu.int/itudoc/itu-t/com16/tiff-fx/docs/tiff6.pdf
// http://www.awaresystems.be/imaging/tiff/bigtiff.html

namespace isis{
namespace image_io{
	
namespace _internal {

	class ValueArrayJPEGSource : public jpeg_source_mgr{
		data::ValueArray<uint8_t> array;
	public:
		ValueArrayJPEGSource(data::ValueArray<uint8_t> src):array(src){
			init_source=init_source_p;
			fill_input_buffer=fill_input_buffer_p;
			skip_input_data=skip_input_data_p;
			resync_to_restart=jpeg_resync_to_restart;
		}
		static void init_source_p(j_decompress_ptr cinfo){
			ValueArrayJPEGSource* sp = (ValueArrayJPEGSource*)cinfo->src;
			sp->next_input_byte = sp->array.begin();
			sp->bytes_in_buffer = sp->array.getLength();
		}
		static boolean fill_input_buffer_p(j_decompress_ptr cinfo){
// 			Normally the whole strip/tile is read and so we don't need to do
// 			a fill.  In the case of CHUNKY_STRIP_READ_SUPPORT we might not have
// 			all the data, but the rawdata is refreshed between scanlines and
// 			we push this into the io machinery in JPEGDecode(). 	 
// 			http://trac.osgeo.org/gdal/ticket/3894

			LOG(Runtime,warning) << "the jpeg decoder asked for a refill, it shouldn't. Telling it the stream ended";
			static const JOCTET dummy_EOI[2] = { 0xFF, JPEG_EOI };
			/* insert a fake EOI marker */
			cinfo->src->next_input_byte = dummy_EOI;
			cinfo->src->bytes_in_buffer = 2;
			return true;
		}
		static void skip_input_data_p(j_decompress_ptr cinfo, long num_bytes){
			if (num_bytes > 0) {
				if (num_bytes > cinfo->src->bytes_in_buffer) { // oops, buffer overrun 
					fill_input_buffer_p(cinfo);
				} else {
					cinfo->src->next_input_byte+=num_bytes;
					cinfo->src->bytes_in_buffer-=num_bytes;
				}
			}
		}
		static void term_source_p(j_decompress_ptr cinfo){}
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

	template<typename T> util::PropertyValue getPropVal_impl(data::ByteArray &source,bool byteswap,size_t offset, size_t number_of_values){
		util::PropertyValue ret;
		for(auto val:source.at<T>(offset,number_of_values,byteswap))
			ret.push_back(val);
		return ret;
	}

	template<> util::PropertyValue getPropVal_impl<std::string>(data::ByteArray &source,bool byteswap,size_t offset, size_t number_of_values){
		const char *str=(char*)source.at<uint8_t>(offset,number_of_values,byteswap).getRawAddress().get();
		return util::Value<std::string>(str);
	}
	
	//rational (Two LONGs: the first represents the numerator of a fraction; the second, the denominator.)
	template<> util::PropertyValue getPropVal_impl<double>(data::ByteArray &source,bool byteswap,size_t offset, size_t number_of_values){
		util::PropertyValue ret=getPropVal_impl<uint32_t>(source,byteswap,offset,number_of_values*2);
		const util::Value<double> frac=ret[0].as<double>()/ret[1].as<double>();
		LOG(Debug,verbose_info) << "Computed " << frac << " from rational " << ret;
		return frac;
	}

	class TiffSource:protected data::ByteArray{
		bool m_big_tiff=false, m_byteswap=false;
		uint64_t m_offset=0;
		
	public:
		template<typename T> ValueArray<T> at( size_t offset, size_t len = 0){
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
					m_byteswap=(__BYTE_ORDER==__BIG_ENDIAN);
					break; 
				case 0x4D4D: // big endian
					m_byteswap=(__BYTE_ORDER==__LITTLE_ENDIAN);
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
			const size_t projected_size=number_of_values*sizeof(T);
			
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
			LOG(Debug,info) << "Red tag " << ret.first << " as " << ret.second;
			return ret;
		}

	};
	
	
	class IFD{
		std::array<size_t,2> size,tilesize;
		std::array<float,2> pixels_per_unit;
		std::list<size_t> stripoffsets,stripbytecounts,tileoffsets,tilebytecounts,bitspersample;
		std::list<_internal::ValueArrayJPEGSource> tiles;
		enum unit_types{none=1,inch,centimeter}resolution_unit=inch;
		util::PropertyMap generic_props;
		compression_type compression;
		photometric_type photometricinterpretation;
		uint16_t 
			planar_configuration=1, //http://www.awaresystems.be/imaging/tiff/tifftags/planarconfiguration.html
			samples_per_pixel;
		size_t rowsperstrip;
	public:
		size_t readTAG(TiffSource &source){
			std::pair<uint16_t,util::PropertyValue> tag=source.readTag();
			switch(tag.first){
				case 0x100:size[0]=tag.second.as<size_t>();break;//ImageWidth
				case 0x101:size[1]=tag.second.as<size_t>();break;//ImageLength
				case 0x102://TileOffsets
					for(const auto &off:tag.second)
						bitspersample.push_back(off.as<size_t>());
					break; 
				case 0x103:compression =(compression_type) tag.second.as<uint16_t>();break;//Compression
				case 0x106:photometricinterpretation = (photometric_type)tag.second.as<uint16_t>();break;//PhotometricInterpretation
				
				case 0x111://StripOffsets
					for(const auto &off:tag.second)
						stripoffsets.push_back(off.as<size_t>());
					break; 
				case 0x115:samples_per_pixel=tag.second.as<size_t>();break;
				case 0x116:rowsperstrip=tag.second.as<size_t>();break;
				case 0x117://StripByteCounts
					for(const auto &off:tag.second)
						stripbytecounts.push_back(off.as<size_t>());
					break; 

				case 0x128:resolution_unit=(unit_types)tag.second.as<uint16_t>();break;
				case 0x11A:pixels_per_unit[0]=tag.second.as<float>();break;
				case 0x11B:pixels_per_unit[1]=tag.second.as<float>();break;
					
				case 0x142:tilesize[0]=tag.second.as<size_t>();break;
				case 0x143:tilesize[1]=tag.second.as<size_t>();break;
				case 0x144://TileOffsets
					for(const auto &off:tag.second)
						tileoffsets.push_back(off.as<size_t>());
					break; 
				case 0x145://TileOffsets
					for(const auto &off:tag.second)
						tilebytecounts.push_back(off.as<size_t>());
					break; 
				default:
					generic_props.touchProperty(std::to_string(tag.first).c_str()).transfer(tag.second);
					break;
			}
		}

		IFD(TiffSource &source){
			size_t number_of_tags= source.readValAuto();
			util::DefaultMsgPrint::stopBelow(warning);
			
			for(uint16_t i=0;i<number_of_tags;i++){
				readTAG(source);
			}
			std::cout << generic_props << std::endl;
			assert(tilebytecounts.size()==tileoffsets.size());
			while(!tilebytecounts.empty()){
				data::ValueArray<uint8_t> tile=source.at<uint8_t>(tileoffsets.front(),tilebytecounts.front());
				tiles.push_back(tile);
				tileoffsets.pop_front();
				tilebytecounts.pop_front();
			}
		}
		data::Chunk makeChunk(){
			data::MemChunk<util::color24> ret(size[0],size[1],1,1,true);
			ret.branch("TIFF")=generic_props;
			
			if(!stripoffsets.empty()){
				std::cout << "stripoffsets" << stripoffsets << std::endl;
			}
			for (size_t y = 0; y < size[1]; y += tilesize[1]){
				for (size_t x = 0; x < size[0]; x += tilesize[0]){
					jpeg_decompress_struct cinfo;
					jpeg_error_mgr jerr;

					cinfo.err = jpeg_std_error(&jerr);
					cinfo.err->trace_level=5;
					jpeg_create_decompress(&cinfo);

					cinfo.src = &tiles.front();
					cinfo.image_width  = tilesize[0];      /* image width and height, in pixels */
					cinfo.image_height = tilesize[1];
					
					if(jpeg_read_header(&cinfo, TRUE)!=JPEG_HEADER_OK)
						LOG(Debug,warning) << "jpeg_read_header didn't return OK";
					if(!jpeg_start_decompress(&cinfo))
						LOG(Debug,warning) << "jpeg_start_decompress didn't return OK";
					
					assert(cinfo.out_color_space==JCS_RGB);
					assert(cinfo.out_color_components==3);
					assert(cinfo.image_width==tilesize[0]);
					assert(cinfo.data_precision==8);
					for(size_t i=0;i<std::min(tilesize[1],size[1]-y);i++){
// 						if(size[0]-x >= tilesize[0]){
// 							JSAMPARRAY scanline=(JSAMPROW*)&ret.voxel<util::color24>(x,i);
// 							jpeg_read_scanlines(&cinfo,scanline,1);
// 						} else {
							std::unique_ptr<JSAMPROW> scanline((JSAMPROW*)malloc(sizeof(JSAMPROW)*cinfo.out_color_components*tilesize[0]));
							jpeg_read_scanlines(&cinfo,scanline.get(),1);
							memcpy(&ret.voxel<util::color24>(x,i),scanline.get(),(size[0]-x)*cinfo.out_color_components);
// 						}
					}
					jpeg_finish_decompress(&cinfo);
					tiles.pop_front();
				}
			}
			return ret;
		}
	};
}

util::istring ImageFormat_TiffSa::suffixes(isis::image_io::FileFormat::io_modes /*modes*/) const{return ".tiff .tif .scn";}

std::list< data::Chunk > ImageFormat_TiffSa::load(
	data::ByteArray source, 
	std::list<util::istring> /*formatstack*/, 
	const util::istring &dialect, 
	std::shared_ptr<util::ProgressFeedback> feedback
)throw( std::runtime_error & ) {
	bool byteswap=false,big_tiff=false;
	_internal::TiffSource tiff(source);
	
	const uint64_t ifd_start = tiff.readValAuto<uint32_t>();
	
	tiff.seek(ifd_start);
	
	_internal::IFD first(tiff);
	first.makeChunk();
	
}

std::string ImageFormat_TiffSa::getName() const{return "tiff";}
void ImageFormat_TiffSa::write(const data::Image & image, const std::string & filename, const util::istring & dialect, std::shared_ptr<util::ProgressFeedback> feedback){}


}}

isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_TiffSa();
}
