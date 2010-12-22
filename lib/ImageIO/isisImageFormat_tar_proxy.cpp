
#include "DataStorage/io_interface.h"
#include <DataStorage/io_factory.hpp>
#include <CoreUtils/tmpfile.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>

#include <tar.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <fstream>

namespace isis
{
namespace image_io
{

/**
 * IO-Proxy handling tar files.
 * This might not work for OS with broken file handling (eg. Windows).
 */
class ImageFormat_TarProxy: public FileFormat
{
private:
	struct
	{
		char name[100];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char typeflag;
		char linkname[100];
		char magic[6];
		char version[2];
		char uname[32];
		char gname[32];
		char devmajor[8];
		char devminor[8];
		char prefix[155];
		char padding[12];
	}tar_header;

protected:
	std::string suffixes()const {
		return std::string( "tar tar.gz tgz tar.bz2 tbz tar.Z taz" );
	}
public:
	std::string dialects( const std::string &filename )const {
/*		if( filename.empty() ) {*/
			return std::string();
/*		} else {
			std::set<std::string> ret;
			data::IOFactory::FileFormatList formats = data::IOFactory::get().getFormatInterface( FileFormat::makeBasename( filename ).first );
			BOOST_FOREACH( data::IOFactory::FileFormatList::const_reference ref, formats ) {
				const std::list<std::string> dias = util::string2list<std::string>( ref->dialects( filename ) );
				ret.insert( dias.begin(), dias.end() );
			}
			return util::list2string( ret.begin(), ret.end(), ",", "", "" );
		}*/
	}
	std::string name()const {return "tar decompression proxy for other formats";}

	int load ( data::ChunkList &chunks, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & ) {
		int ret=0;

		const util::istring suffix=makeBasename(filename).second.c_str();
		

		// set up the input stream
		std::ifstream input(filename.c_str(), std::ios_base::binary);
		input.exceptions( std::ios::badbit );
		boost::iostreams::filtering_istream in;
		if(suffix==".tar.gz" || suffix==".tgz")
			in.push(boost::iostreams::gzip_decompressor());
		else if(suffix==".tar.bz2" || suffix==".tbz")
			in.push(boost::iostreams::bzip2_decompressor());
		else if(suffix==".tar.Z" || suffix==".taz")
			in.push(boost::iostreams::zlib_decompressor());
		in.push(input);

		while(in.good() && boost::iostreams::read(in,reinterpret_cast<char*>(&tar_header),512)==512){ //read the header block

			//get the size
			std::stringstream buff(tar_header.size);
			size_t size,next_header_in;
			if(tar_header.size[10]!=0) {
				buff >> std::oct >> size;
				next_header_in=(size/512)*512 + (size%512 ? 512:0);
			} else
				continue; // if the size entry is zero there wont be anything usefull to us

			//get the original filename (use substr, because these fields are not )
			boost::filesystem::path org_file(std::string(tar_header.prefix).substr(0,155)+std::string(tar_header.name).substr(0,100));

			if(tar_header.typeflag == AREGTYPE || tar_header.typeflag == REGTYPE){
				
				data::IOFactory::FileFormatList formats = data::IOFactory::get().getFormatInterface( org_file.string() ); // and get the reading pluging for that

				if(formats.empty()){
					LOG(Runtime,warning) << "Skipping " << org_file << " from " << filename << " because no plugin was found to read it"; // skip if we found none
				} else {
					LOG(Debug,info) << "Got " << org_file << " from " << filename << " there are " << formats.size() << " plugins which should be able to read it";

					const std::pair<std::string, std::string> base = formats.front()->makeBasename( org_file.string() );//ask any of the plugins for the suffix
					util::TmpFile tmpfile( "", base.second );//create a temporary file with this suffix
					int mfile=open(tmpfile.file_string().c_str(),O_CREAT|O_RDWR);
					if(mfile==-1){
						throwSystemError( errno, std::string("Failed to open temporary ") + tmpfile.file_string());
					}
					posix_fallocate(mfile,0,size); // set it to the given size - otherwise mmap will be very sad

					char *mmem=(char*)mmap(NULL,size,PROT_WRITE,MAP_SHARED,mfile,0);//map it into memory
					if(mmem==MAP_FAILED){
						throwSystemError( errno, std::string("Failed to map temporary ") + tmpfile.file_string() + " into memory");
					}
					LOG(Debug,info) << "Mapped " << size << " bytes of " << tmpfile.file_string() << " at " << mmem;
					
					size_t red=boost::iostreams::read(in,mmem,size); // read data from the stream into the mapped memory
					next_header_in-=red;
					if(red!=size) // read the data from the stream
						LOG(Runtime,warning) << "Could not read all " << size << " bytes for " << tmpfile.file_string();

					//unmap and close the file
					munmap(mmem,size);
					close(mfile);

					// read the temporary file
					data::ChunkList chunksT;
					ret += data::IOFactory::get().loadFile( chunksT, tmpfile, "", dialect );
					BOOST_FOREACH( data::ChunkList::reference ref, chunksT ) { // set the source property of the red chunks to something more usefull
						ref->setProperty( "source", (boost::filesystem::path(filename) / org_file).file_string() );
					}
					chunks.insert(chunks.end(),chunksT.begin(),chunksT.end()); // copy the red chunks into the output-list
				}
			} else {
				LOG(Debug,verbose_info) << "Skipping " << org_file.file_string() << " because its no regular file (type is "<< tar_header.typeflag << ")" ;
			}
			in.ignore(next_header_in); // skip the remaining input until the next header
		}
		return ret;
	}

	void write( const data::Image &image, const std::string &filename, const std::string &dialect )throw( std::runtime_error & ) {
		throw( std::runtime_error( "Writing to tar is not (yet) implemented" ) );
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_TarProxy();
}
