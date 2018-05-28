#include "common.hpp"
#include <thread>

QImage isis::qt5::makeQImage(const std::vector<data::ValueArrayBase::Reference> &lines, data::scaling_pair scaling)
{
	QImage ret;
	size_t line_length = lines.front()->getLength();
	if(lines.front()->is<util::color24>()){
		ret = QImage(line_length,lines.size(), QImage::Format_RGB32);
	} else {
		ret = QImage(line_length,lines.size(), QImage::Format_Indexed8);

		ret.setColorCount(256);
		for(int i=0;i<256;i++)
			ret.setColor(i,qRgb(i,i,i));
	}
	fillQImage(ret,lines,scaling);
	return ret;
}

QImage isis::qt5::makeQImage(const data::ValueArrayBase &slice,size_t line_length,data::scaling_pair scaling)
{
	return makeQImage(slice.splice(line_length),scaling);
}


QImage isis::qt5::makeQImage(const std::vector<data::ValueArrayBase::Reference> &lines, const std::function<void (uchar *, const data::ValueArrayBase &)>& transfer_function)
{
	QImage ret;
	size_t line_length = lines.front()->getLength();
	if(lines.front()->is<util::color24>() || lines.front()->is<util::color48>()){
		ret = QImage(line_length,lines.size(), QImage::Format_RGB32);
	} else {
		ret=QImage(line_length,lines.size(), QImage::Format_Indexed8);

		ret.setColorCount(256);
		for(int i=0;i<256;i++)
			ret.setColor(i,qRgb(i,i,i));
	}
	fillQImage(ret,lines,transfer_function);
	return ret;
}
QImage isis::qt5::makeQImage(const data::ValueArrayBase& slice, size_t line_length, const std::function<void (uchar *, const data::ValueArrayBase &)>& transfer_function)
{
	return makeQImage(slice.splice(line_length),transfer_function);
}


void isis::qt5::fillQImage(QImage& dst, const std::vector<data::ValueArrayBase::Reference> &lines, data::scaling_pair scaling)
{
	std::function<void (uchar *, const data::ValueArrayBase &)> transfer_function;
	switch(dst.format()){
	case QImage::Format_Indexed8:
		transfer_function = [scaling](uchar *dst, const data::ValueArrayBase &line){
			line.copyToMem<uint8_t>(dst,line.getLength(),scaling);
		};
		break;
	case QImage::Format_RGB32:
		transfer_function = [scaling](uchar *dst, const data::ValueArrayBase &line){
			QRgb *rgbdst=(QRgb*)dst;
			for(const util::color24 &c:const_cast<data::ValueArrayBase &>(line).as<util::color24>(scaling)){
				*(rgbdst++)=qRgb(c.r,c.g,c.b);
			}
		};
		break;
	default:
		LOG(Runtime,error) << "Unsupported image color format";
	}
	fillQImage(dst, lines, transfer_function);
}
void isis::qt5::fillQImage(QImage& dst, const data::ValueArrayBase& data, size_t line_length, data::scaling_pair scaling)
{
	LOG_IF(data.getLength()%line_length,Debug,error) << "The length of the array (" << data.getLength() << ") is not a multiple of the given line length for the QImage, something is really wrong here";
	fillQImage(dst, data.splice(line_length), scaling);
}

void isis::qt5::fillQImage(QImage& dst, const std::vector<data::ValueArrayBase::Reference> &lines, const std::function<void (uchar *, const data::ValueArrayBase &)> &transfer_function)
{
	const size_t thread_size=std::ceil(lines.size()/float(std::thread::hardware_concurrency()));
	std::vector<std::thread> threads;
	
	auto line_copy = [&dst,&lines,&transfer_function](size_t start, size_t end){
		for(size_t line_idx=start;line_idx<end;line_idx++) {
			uchar *dstline=dst.scanLine(line_idx);
			transfer_function(dstline,*lines[line_idx]);
		}
	};
	
	for(size_t i=0;i<lines.size();i+=thread_size){
		const size_t start=i,end=std::min(i+thread_size,lines.size());
		threads.push_back(std::thread(line_copy,start,end));
	}
	for(std::thread &t:threads){
		t.join();
	}
}
void isis::qt5::fillQImage(QImage& dst, const data::ValueArrayBase& data, size_t line_length, const std::function<void (uchar *, const data::ValueArrayBase &)> &transfer_function)
{
	LOG_IF(data.getLength()%line_length,Debug,error) << "The length of the array (" << data.getLength() << ") is not a multiple of the given line length for the QImage, something is really wrong here";
	fillQImage(dst, data.splice(line_length), transfer_function);
}

