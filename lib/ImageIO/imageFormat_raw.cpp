#include <DataStorage/io_interface.h>
#include <fstream>
#include <boost/foreach.hpp>
#include <sys/mman.h>
#include <fcntl.h>
#include <boost/filesystem.hpp>

namespace isis
{
namespace image_io
{

class ImageFormat_raw: public FileFormat
{
	typedef std::map<std::string,unsigned short> typemap;
protected:
	std::string suffixes()const {
		return std::string( "raw" );
	}
	struct Deleter {
		//members
		int m_file;
		std::string m_filename;
		Deleter( int file, const std::string &filename ):m_file(file),m_filename(filename){}

		void operator ()( void *at ) {
			LOG(Debug,info) << "Freeing memory at " << at << " mapped from " << m_filename;
			munmap( at, boost::filesystem::file_size(m_filename) );//@todo will fail when the file was replaced in the meantime
			close( m_file );
		}
	};

	class RawMemChunk:public data::Chunk
	{
	public:
		template<typename T> RawMemChunk(T *src,const Deleter &del, size_t width, size_t height, size_t slices=1, size_t timesteps=1)
		:data::Chunk( src, del, width, height, slices, timesteps ){}
	};
public:
	std::string getName()const {
		return "raw data output";
	}
	virtual std::string dialects(const std::string& ) const{
		std::string ret;
		typemap types=util::getTransposedTypeMap(false,true);
		types.erase("boolean*");
		types.erase("date*");
		types.erase("timestamp*");
		types.erase("selection*");
		BOOST_FOREACH(typemap::const_reference pair, types ){
			ret+=pair.first.substr(0,pair.first.length()-1);
			ret+=" ";
		}
		return ret;
	}


	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {


		const size_t fsize=boost::filesystem::file_size(filename);
		const unsigned short type=util::getTransposedTypeMap(false,true)[dialect+"*"];
		if(type==0){
			LOG(Runtime,error) << "No known datatype given, you have to give the type ofe the raw data as rdialect (eg. \"-rdialect u16bit\")";
			throwGenericError("No known datatype");
		}
		const size_t elemSize=data::_internal::ValuePtrBase::createById(type,0)->bytesPerElem();
		const size_t ssize=sqrt(fsize/elemSize);

		if(ssize*ssize*elemSize == fsize){
			LOG(Runtime,info) << "Guessing size of read and phase to be " << ssize;

			int mfile = open( filename.c_str(), O_RDONLY);
			if( mfile == -1 ) {
				throwSystemError( errno, std::string( "Failed to open " ) + filename );
			}


			char *mmem = ( char * )mmap( NULL, fsize, PROT_READ, MAP_SHARED, mfile, 0 ); //map it into memory
			if( mmem == MAP_FAILED ) {
				throwSystemError( errno, std::string( "Failed to map " ) + filename + " into memory" );
			}


			Deleter del(mfile,filename);
			chunks.push_back(RawMemChunk((uint16_t*)mmem,del,ssize,ssize));
			data::Chunk &ch=chunks.back();
			ch.setPropertyAs<uint16_t>("sequenceNumber",0);
			ch.setPropertyAs<uint32_t>("acquisitionNumber",0);
			ch.setPropertyAs("voxelSize",util::fvector4(1,1,1));
			ch.setPropertyAs("rowVec",util::fvector4(1,0));
			ch.setPropertyAs("columnVec",util::fvector4(0,1));
			ch.setPropertyAs("indexOrigin",util::fvector4(0,0));
			return 1;
		} else {
			LOG(Runtime,error) << "Could not guess image size for " << fsize << " bytes of data";
		}
	}

	void write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		class WriteOp: public data::Image::ChunkOp{
			std::ofstream out;
			unsigned short typeID;
		public:
			WriteOp(std::string fname,unsigned short id):out( fname.c_str() ),typeID(id){
				out.exceptions( std::ios::failbit | std::ios::badbit );
			}
			bool operator()(data::Chunk& ref, util::FixedVector<size_t, 4 > /*posInImage*/){
				const boost::shared_ptr<void> data( ref.getValuePtrBase().getRawAddress() );
				const size_t data_size = ref.bytesPerVoxel() * ref.getVolume();
				out.write( static_cast<const char *>( data.get() ), data_size );
			}
		};
		const std::pair<std::string, std::string> splitted = makeBasename( filename );
		unsigned short type = image.getMajorTypeID();

		std::string typeStr = util::getTypeMap(false)[type];
		typeStr.erase( typeStr.find_last_not_of( '*' ) + 1 );

		const std::string outName = splitted.first + "_" + image.getSizeAsString() + "_" + typeStr + splitted.second ;

		LOG( ImageIoLog, info ) << "Writing image of size " << image.getSizeAsVector() << " and type " << typeStr << " to " << outName;
		WriteOp writer(outName,type);
		const_cast<data::Image &>(image).foreachChunk(writer);
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_raw();
}
