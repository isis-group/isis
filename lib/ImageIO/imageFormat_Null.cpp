#include <DataStorage/io_interface.h>

namespace isis{ namespace image_io{ 

class ImageFormat_Null: public FileFormat{
public:
	std::string suffixes(){
		return std::string();
	}
	std::string dialects(){
	return std::string();
	}
	std::string name(){
		return "Null";
	}
  
	virtual isis::data::ChunkList load ( std::string filename, std::string dialect ){
		return isis::data::ChunkList();
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