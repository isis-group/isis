#include "imageFormat_tiff_sa.hpp"
#include <stdint.h>

// https://www.itu.int/itudoc/itu-t/com16/tiff-fx/docs/tiff6.pdf
// http://www.awaresystems.be/imaging/tiff/bigtiff.html

namespace isis{
namespace image_io{
	
namespace _internal {
	class TiffSource:protected data::ByteArray{
		bool m_big_tiff=false, m_byteswap=false;
		uint64_t offset=0;
		

	public:
		void seek(uint64_t _offset){offset=_offset;}
		template<typename T> T readVal(){
			T ret=at<T>(offset,1,m_byteswap)[0];
			offset+=sizeof(T);
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
		template<typename T> util::PropertyValue readPropVal(){
			util::PropertyValue ret;
			const uint64_t number_of_values= readValAuto<uint32_t>();
			for(auto val:at<T>(offset,number_of_values,m_byteswap))
				ret.push_back(val);
			
			return ret;
		}
		std::pair<uint16_t,util::PropertyValue> readTag(){
			uint64_t next=offset+ (m_big_tiff?20:12);
			std::pair<uint16_t,util::PropertyValue> ret;
			ret.first =readVal<uint16_t>();
			const uint16_t tag_type=readVal<uint16_t>();

			switch(tag_type){
				case 1://byte
					ret.second=readPropVal<uint8_t>();
					break;
				case 2://ascii
				case 3://short
					ret.second=readPropVal<uint16_t>();
					break;
				case 4://long
					ret.second=readPropVal<uint32_t>();
					break;
				case 5://rational (Two LONGs: the first represents the numerator of a fraction; the second, the denominator.)
				default:
					LOG(Runtime,error) << "Invalid type " << tag_type << " when reading tag " << ret.first;
			}
			seek(next);
			LOG(Debug,info) << "Red tag " << ret.first << " as " << ret.second;
			return ret;
		}

	};
	
	
	class IFD:data::Image{
	public:
		size_t readTAG(TiffSource &source){
			std::pair<uint16_t,util::PropertyValue> tag=source.readTag();
			static const util::PropertyMap::PropPath tiff("tiff");
			switch(tag.first){
				default:
					touchProperty(tiff/std::to_string(tag.first).c_str()).transfer(tag.second);
					break;
			}
		}

		IFD(TiffSource &source){
			size_t number_of_tags= source.readValAuto();
			
			for(uint16_t i=0;i<number_of_tags;i++){
				readTAG(source);
			}
		}
	};
}

util::istring ImageFormat_TiffSa::suffixes(isis::image_io::FileFormat::io_modes /*modes*/) const{return ".tiff .tif";}

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
	
}

std::string ImageFormat_TiffSa::getName() const{return "tiff";}
void ImageFormat_TiffSa::write(const data::Image & image, const std::string & filename, const util::istring & dialect, std::shared_ptr<util::ProgressFeedback> feedback){}


}}

isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_TiffSa();
}
