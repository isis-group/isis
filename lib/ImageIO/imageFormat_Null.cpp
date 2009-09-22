#include <DataStorage/io_interface.h>

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
  
	virtual isis::data::Chunks load ( std::string filename, std::string dialect ){
		return isis::data::Chunks();
	}
	
	virtual bool save ( const isis::data::Chunks& chunks, std::string filename, std::string dialect ){
		return false;
	}
	bool tainted(){return false;}//internal plugins are not tainted
};

isis::data::FileFormat* factory(){
  return new NullFormat();
}