#include <DataStorage/io_interface.h>

namespace isis{ namespace image_io{ 

class ImageFormat_Null: public FileFormat{
public:
	std::string suffixes(){
		return std::string(".null .null.gz");
	}
	std::string dialects(){
	return std::string("dia1");
	}
	std::string name(){
		return "Null";
	}
  
	virtual isis::data::ChunkList load ( std::string filename, std::string dialect ){
		isis::data::MemChunk<short> test(1,1,1,5);
		isis::data::ChunkList list;
		list.add(test);
		return list;//return isis::data::ChunkList();
	}
	
	virtual bool save ( const isis::data::ChunkList& chunks, std::string filename, std::string dialect ){
		return false;
	}
	bool tainted(){return false;}//internal plugins are not tainted
};
}}
isis::image_io::FileFormat* factory(){
  return new isis::image_io::ImageFormat_Null();
}
