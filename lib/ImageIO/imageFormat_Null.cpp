#include <DataStorage/io_interface.h>

class NullFormat: public isis::data::FileReader{
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
  bool tainted(){return false;}//internal plugins are not tainted
};

isis::data::FileReader* factory(){
  return new NullFormat();
}