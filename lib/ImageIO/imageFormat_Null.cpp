#include <DataStorage/io_interface.h>

class NullFormat: public isis::data::FileFormat{
public:
  std::string formats(){
    return std::string();
  }
  std::string dialects(){
    return std::string();
  }
  std::string name(){
    return "Null";
  }
};

isis::data::FileFormat* factory(){
  return new NullFormat();
}