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
  
	virtual data::ChunkList load ( std::string filename, std::string dialect ){
		data::MemChunk<short> test(1,1,1,5);
		data::ChunkList list;
		list.push_back(test);
		return list;//return data::ChunkList();
	}
	
	virtual bool save ( const data::ChunkList& chunks, std::string filename, std::string dialect ){
		return false;
	}
	bool tainted(){return false;}//internal plugins are not tainted
};
}}
isis::image_io::FileFormat* factory(){
  return new isis::image_io::ImageFormat_Null();
}
