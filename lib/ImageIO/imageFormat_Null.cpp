#include <DataStorage/io_interface.h>

class NullFormat: public isis::data::FileFormat{
public:
  std::list<format> formats(){
    return std::list<format>();
  }
  std::string name(){
    return "Null";
  }
};

isis::data::FileFormat* factory(){
  return new NullFormat();
}