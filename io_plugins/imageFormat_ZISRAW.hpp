#ifndef ZISRAW_HPP
#define ZISRAW_HPP

#include <isis/core/io_factory.hpp>
#include <stdint.h>
#include <stdio.h>
#include <memory>
#include <future>

namespace isis{
namespace image_io{
	
namespace _internal {
	struct DimensionEntry{
		std::string Dimension;
		int32_t start,size,StoredSize;
		float StartCoordinate;
	};
	struct DirectoryEntryDV{
		int32_t PixelType;
		int64_t FilePosition;
		int32_t Compression;
		uint8_t PyramidType;
		int32_t DimensionCount;
		std::vector<DimensionEntry> dims;
		size_t size()const{return 32+DimensionCount*20;}
	};

	template<typename T> void getScalar(data::ByteArray &data,T &variable,size_t offset){
        data.at<T>(offset,1,__BYTE_ORDER__==__ORDER_BIG_ENDIAN__).copyToMem(&variable,1);
	}
	DirectoryEntryDV getDVEntry(data::ByteArray &data,size_t offset);
	
	util::PropertyMap getXML(data::ByteArray &data,size_t offset,size_t length, std::shared_ptr<std::ofstream> dump_stream=nullptr);
	
	template<typename D> data::ValueArray<util::color<D>> color_reshuffle(const data::ValueArray<D> &data){
		assert(data.getLength()%3==0);
		data::ValueArray<util::color<D>> ret(data.getLength()/3);
		auto src_it=data.begin();
		for(util::color<D> &dst:ret){
			dst.b=*src_it;++src_it;
			dst.g=*src_it;++src_it;
			dst.r=*src_it;++src_it;
		}
		assert(src_it==data.end());
		return ret;
	}
	
	data::ValueArrayReference reinterpretData(const data::ByteArray &data,int32_t PixelType);
}

class ImageFormat_ZISRAW : public FileFormat{
	static const std::map<uint32_t,uint16_t> PixelTypeMap,PixelSizeMap;
	static const std::map<std::string,uint16_t> PixelTypeMapStr;
	class Segment{
		uint64_t allocated_size,used_size;
	protected:
		data::ByteArray data;
		template<typename T> void getScalar(T &variable,size_t offset){
			return _internal::getScalar(data,variable,offset);
		}
	public:
		std::string id;
		size_t getSegmentSize();
		/**
		 * Create a segment from the source
		 * \param source the ByteArray from the container (file)
		 * \param offset the offset of the segment inside the container (this will be moved forward to the next segment)
		 */
		Segment(data::ByteArray &source, const size_t offset);
	};
	class FileHeader:public Segment{
	public:
		struct {int32_t major,minor;}version;
		uint8_t PrimaryFileGuid[16],FileGuid[16];
		int32_t FilePart;
		int64_t DirectoryPosition,MetadataPosition,AttachmentDirectoryPosition;
		bool UpdatePending;

		FileHeader(data::ByteArray &source, const size_t offset);
	};
	class MetaData:public Segment{
		int32_t XMLSize,AttachmentSize;
		util::PropertyMap xml_data;
	public:
		MetaData(data::ByteArray &source, const size_t offset, std::shared_ptr<std::ofstream> dump_stream);
		util::PropertyMap get()const;
	};
	class SubBlock:public Segment{
		int32_t MetadataSize,AttachmentSize;
		int64_t DataSize;
		data::ByteArray image_data;
		_internal::DirectoryEntryDV DirectoryEntry;
		static data::Chunk jxrRead(size_t xsize,size_t ysize,isis::data::ByteArray image_data,unsigned short isis_type,unsigned short pixel_size);
	public:
		SubBlock(data::ByteArray &source, const size_t offset, std::shared_ptr<std::ofstream> dump_stream);
		std::function<data::Chunk()> getChunkGenerator()const;
		util::PropertyMap xml_data;
		bool isNormalImage()const;
		std::string getPlaneID()const;
		std::map<std::string,_internal::DimensionEntry> getDimsInfo()const;
		std::array<size_t,4> getSize()const;
	};
	class Directory:public Segment{
	public:
		std::vector<_internal::DirectoryEntryDV> entries;
		Directory(data::ByteArray &source, const size_t offset);
	};
	void storeProperties(data::Chunk &dst,std::string plane_id);
	void transferFromMosaic(std::list<SubBlock> segments,data::Chunk &dst, int32_t xoffset, int32_t yoffset,std::shared_ptr<util::ProgressFeedback> feedback);
public:
	util::istring suffixes(isis::image_io::FileFormat::io_modes /*modes*/) const override {return ".czi";}

	std::list< data::Chunk > load(
		data::ByteArray source,
		std::list<util::istring> /*formatstack*/,
		std::list<util::istring> dialects,
		std::shared_ptr<util::ProgressFeedback> feedback
	)throw( std::runtime_error & ) override;

	std::string getName() const override {return "Zeiss Integrated Software RAW";}

	std::list<util::istring> dialects() const override {return {"dump_xml","nopyramid"};}

	void write(const data::Image &/*image*/, const std::string &/*filename*/, std::list<util::istring> /*dialects*/, std::shared_ptr<util::ProgressFeedback> /*feedback*/) override{
		throwGenericError("not yet implemented");
	}
};

	
}}

#endif //ZISRAW_HPP
