#include "DataStorage/io_interface.h"
#include "DataStorage/io_factory.hpp"
#include <stdio.h>
#include <fstream>
#include <zlib.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

namespace isis{ namespace image_io{
	
class ImageFormat_CompProxy: public FileFormat{
private:
	static std::string tempfile() {
		std::string result;
		const char* ptr=tmpnam(NULL);
		if(ptr) result=ptr;
		//@todo find a solution for windows here
		return result;
	}
	static std::string tempfilename(std::string filename) {
		boost::filesystem::path fname(filename);
		const std::string unzipped_suffix=boost::filesystem::extension(boost::filesystem::basename(fname));
		return tempfile()+unzipped_suffix;
	}
	
	bool gz_compress(std::ifstream &in, gzFile out) {
		char *buf = new char[2048*1024];
		int len;
		int err;
		bool ret=true;
		
		try
		{
			for (
				in.read(buf, 2048*1024);
				(len = in.gcount());
				in.read(buf, 2048*1024)
			) {
				if (gzwrite(out, buf, len) != len) {
					LOG(ImageIoLog,util::error) << "Failed to compress using gzip " << gzerror(out, &err);
					return false;
				}
			}
			if (in.bad()) {
				LOG(ImageIoLog,util::error) << "Failed to read input ";
				return false;
			}
		}
		catch(...) {
			LOG(ImageIoLog,util::error) << "Failed to read input ";
			delete[]  buf;
			throw;
		}
		delete buf;
		return ret;
	}
	
	bool file_compress(std::string infile, std::string outfile) {
		std::ifstream in(infile.c_str(), std::ios::binary);
		if (in == NULL) {
			LOG(ImageIoLog,util::error) << "Failed to open " << infile.c_str();
			return false;
		}
		gzFile out = gzopen(outfile.c_str(), "wb");
		if (out == NULL) {
			LOG(ImageIoLog,util::error) << "gzopen " << outfile << " failed";
			return false;
		}
		bool ret=gz_compress(in, out);
		if (gzclose(out) != Z_OK) {
			LOG(ImageIoLog,util::warning) << "gzclose " << outfile << " failed";
			return false;
		}
		else return ret;
	}
	
	bool gz_uncompress(gzFile in, std::ofstream &out) {
		char *buf=new char[2048*1024];
		int len;
		int err;
		
		try
		{
			for (
				len = gzread(in, buf, 2048*1024);
			len;
			len = gzread(in, buf, 2048*1024)
			) {
				if (len < 0)
				{
					LOG(ImageIoLog,util::error) << "Failed to read compressed data " << gzerror(in, &err);
					return false;
				}
				else {
					out.write(buf,len);
					if (out.bad()) {
						LOG(ImageIoLog,util::error) << "Failed to write uncompressed data ";
						return false;
					}
				}
			}
		}
		catch(...) {
			LOG(ImageIoLog,util::error) << "Uncompress failed";
			delete[] buf;
			throw;
		}
		delete[] buf;
		return true;
	}
	
	bool file_uncompress(std::string infile,std::string outfile) {
		gzFile in = gzopen(infile.c_str(), "rb");
		if (in == NULL) {
			LOG(ImageIoLog,util::error) << "gzopen " << infile << " failed";
			return false;
		}
		std::ofstream out(outfile.c_str(), std::ios::binary);
		if (out.bad()) {
			LOG(ImageIoLog,util::error) << "Failed to read data from " << infile;
			return false;
		}
		bool ret=gz_uncompress(in, out);
		if (gzclose(in) != Z_OK) {
			LOG(ImageIoLog,util::error) << "gclose " << outfile << " failed";
			return false;
		}
		return ret;
	}
	

public:
	std::string suffixes(){
		return std::string(".gz");
	}
/*	std::string dialects(){
		return std::string("inverted");
	}*/
	std::string name(){return "compression proxy for other formats";}

	int load (data::ChunkList &chunks,const std::string& filename,const std::string& dialect ){
		const std::string tmpfile=tempfilename(filename);
		LOG(ImageIoDebug,util::info) <<  "tmpfile=" << tmpfile;
		
		if(file_uncompress(filename,tmpfile)) {
			boost::filesystem::path fname(tmpfile);
			int result=data::IOFactory::get().loadFile(chunks, fname, dialect);
			boost::filesystem::remove(fname);
			return result;
		}
		else {
			return -1;
		}
	}

	bool write(const data::Image &image,const std::string& filename,const std::string& dialect ){
		const std::string tmpfile=tempfilename(filename);
		LOG(ImageIoLog,util::error) <<  "Compressed write is not yet implemented";
		
		return -1;
		
	}
	bool tainted(){return false;}//internal plugins are not tainted
	size_t maxDim(){return 4;}
};
}}
isis::image_io::FileFormat* factory(){
	return new isis::image_io::ImageFormat_CompProxy();
}
	