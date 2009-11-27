#include <DataStorage/io_interface.h>

namespace isis{ namespace image_io{ 

class ImageFormat_Null: public FileFormat{
public:
	std::string suffixes(){
		return std::string(".null .null.gz");
	}
	std::string dialects(){
		return std::string("inverted");
	}
	std::string name(){
		return "Null";
	}
  
	virtual data::ChunkList load ( std::string filename, std::string dialect ){
		
		const size_t images=5;
		const size_t timesteps=10;

		data::ChunkList chunks;
		for(int i=0;i<timesteps;i++){
			for(int c=0;c<images;c++){
				data::MemChunk<short> ch(3,3,3);
				ch.setProperty("indexOrigin",util::fvector4(0,0,0,i));
				for(int x=0;x<3;x++)
					ch.voxel<short>(x,x,x)=c+i;
				chunks.push_back(ch);
			}
		}
		return chunks;//return data::ChunkList();
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
