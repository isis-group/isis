#include "common.hpp"
#include <thread>


QImage isis::qt5::makeQImage(const data::ValueArrayBase &data,size_t line_length,data::scaling_pair scaling)
{
	auto transfer_function = [scaling](uchar *dst, const data::ValueArrayBase &line){
		line.copyToMem<uint8_t>(dst,line.getLength(),scaling);
	};
	return makeQImage(data,line_length,transfer_function);
}

QImage isis::qt5::makeQImage(const data::ValueArrayBase& data, size_t line_length, const std::function<void (uchar *, const data::ValueArrayBase &)> &transfer_function)
{
	LOG_IF(data.getLength()%line_length,Debug,error) << "The length of the array (" << data.getLength() << ") is not a multiple of the given line length for the QImage, something is really wrong here";

	QImage ret(line_length,data.getLength()/line_length, QImage::Format_Indexed8);
	
	ret.setColorCount(256);
	for(int i=0;i<256;i++)
		ret.setColor(i,qRgb(i,i,i));

	const auto splices=data.splice(line_length);
	const size_t thread_size=std::ceil(splices.size()/float(std::thread::hardware_concurrency()));
	std::vector<std::thread> threads;
	
	auto line_copy = [&ret,&splices,&transfer_function](size_t start, size_t end){
		for(size_t line_idx=start;line_idx<end;line_idx++) {
			uchar *dst=ret.scanLine(line_idx);
			transfer_function(dst,*splices[line_idx]);
		}
	};
	
	for(size_t i=0;i<splices.size();i+=thread_size){
		const size_t start=i,end=std::min(i+thread_size,splices.size());
		threads.push_back(std::thread(line_copy,start,end));
	}
	for(std::thread &t:threads){
		t.join();
	}
	return ret;
}
