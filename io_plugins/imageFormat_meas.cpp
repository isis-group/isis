//
//  imageFormat_meas.cpp
//  isis
//
//  Created by Enrico Reimer on 23.08.17.
//  Copyright © 2017 Enrico Reimer. All rights reserved.
//

#include <isis/data/io_interface.h>
#include <boost/fusion/container/vector.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace isis{
namespace image_io{

namespace _internal{

class ByteStream{
	std::streambuf *buf;
public:
	ByteStream(std::streambuf *src):buf(src){}
	template<typename T> T extract(){
		T ret;
		buf->sgetn(reinterpret_cast<std::streambuf::char_type*>(&ret),sizeof(T));
		return ret;
	}
	std::string extractString(){
		char ret[255];
		std::istream stream(buf);
		stream.getline(ret,255,0);
		return ret;
	}
	void makeHeader(){
		
		//spec says "name" is null terminated in the file, so we use that to get the make the string and scan for its end
		std::string name=extractString();
		
		uint32_t len=extract<uint32_t>();
		
		std::vector<char> data(len);
		buf->sgetn(data.data(),len);
		
		std::cout << "== " << name << " =====" << std::endl << std::string(data.begin(),data.end()) << std::endl;
	}
};

struct MDH_header{
	uint32_t ulFlagsAndDMALength;
	int32_t lMeasUID;
	uint32_t ulScanCounter, ulTimeStamp, ulPMUTimeStamp,aulEvalInfoMask[2];
	uint16_t ushSamplesInScan, ushUsedChannels;
	int8_t sLoopCounter[28];
	int32_t sCutOff;
	uint16_t ushKSpaceCentreColumn,ushCoilSelect;
	float fReadOutOffcentre;
	uint32_t ulTimeSinceLastRF;
	uint16_t ushKSpaceCentreLineNo, ushKSpaceCentrePartitionNo,aushIceProgramPara[4],aushFreePara[4];
	uint8_t sSliceData [28];
	uint16_t ushChannelId,ushPTABPosNeg;
};
class MDH:public data::ValueArray<std::complex<float>>{
	std::streambuf *buf;
public:
	MDH_header header;
	MDH(std::streambuf *src):buf(src){
		static_assert(sizeof(header)==128, "size of MDH header block is not 128 bytes");
		
		//read the header
		buf->sgetn(reinterpret_cast<std::streambuf::char_type*>(&header),sizeof(header));
		
		//initialize with proper size
		data::ValueArray<std::complex<float>>::operator=(data::ValueArray<std::complex<float>>(header.ushSamplesInScan));

		//read the samples
		buf->sgetn(reinterpret_cast<std::streambuf::char_type*>(getRawAddress(0).get()),header.ushSamplesInScan*bytesPerElem());
	}
};
class MDHChunk : public data::MemChunk<std::complex<float>>{
	size_t scanline;
public:
	MDHChunk(std::list<_internal::MDH> &scans, size_t partitions)
	:data::MemChunk<std::complex<float>>(
		scans.front().getLength(),
		scans.size()/partitions, partitions,1, true),scanline(0){
		while(!scans.empty()){
			insert(scans.front());
			scans.pop_front();
		}
	}
	void insert(const MDH &scanln){
		scanln.copyRange(0,scanln.getLength(),this->asValueArrayBase(),scanline*getDimSize(0));
		scanline++;
	}
};

}

class ImageFormat_meas: public FileFormat
{
public:
	virtual std::list<data::Chunk> 
	load(
		std::streambuf *source, 
		std::list<util::istring> formatstack, 
		std::list<util::istring> dialects, 
		std::shared_ptr<util::ProgressFeedback> feedback 
	)throw( std::runtime_error & ) override{
		std::list< data::Chunk > ret;
		_internal::ByteStream hdr_stream(source);

		uint32_t hdr_len=hdr_stream.extract<uint32_t>();
		uint32_t buff_cnt=hdr_stream.extract<uint32_t>();
		
		while(buff_cnt--)
			hdr_stream.makeHeader();
		
		//forward to first hdm
		auto cur_pos=source->pubseekoff(0, std::ios_base::cur);
		assert(cur_pos<=hdr_len);
		cur_pos=source->pubseekoff(hdr_len-cur_pos,std::ios_base::cur);
		LOG_IF(cur_pos!=hdr_len,Runtime,error) << "Failed to jump to end of the header, reading will most likely fail";
		
		std::map<uint32_t, std::list<_internal::MDH>> scans;
		while(source->sgetc()!=std::streambuf::traits_type::eof()){
			_internal::MDH mdh(source);
			scans[mdh.header.ushChannelId].push_back(std::move(mdh));
		}
		
		while(!scans.empty()){
			auto scan=*scans.begin();
			scans.erase(scans.begin());
			
			std::cout << "Channel " << scan.first << ":" << scan.second.size() << " scanlines" << std::endl;
			_internal::MDHChunk ch(scan.second,44);
			ch.setValueAs("coilChannelMask",scan.first);
			ret.push_back(ch);
		}

		return ret;
	}
	std::string getName() const override{return "Siemens Raw data";};
	void write(const data::Image & image, const std::string & filename, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback) override{
		throwGenericError("not yet implemented");
	}
protected:
	util::istring suffixes(isis::image_io::FileFormat::io_modes modes) const override{return ".dat";}
};

}}

isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_meas();
}

