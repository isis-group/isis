#include <DataStorage/io_interface.h>

namespace isis{ namespace image_io{ 

class NullFormat: public isis::data::FileFormat{
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
isis::data::FileFormat* factory(){
  return new isis::image_io::NullFormat();
}