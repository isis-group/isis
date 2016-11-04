#include "common.hpp"



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

	int line_idx=0;
	for(auto line:data.splice(line_length)) {
		uchar *dst=ret.scanLine(line_idx);
		transfer_function(dst,*line);
		++line_idx;
	}
	return ret;
}
