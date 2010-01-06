#include "imageFormat_Dicom.hpp"

namespace isis{ namespace image_io{

std::string ImageFormat_Dicom::suffixes(){return std::string(".ima");}
std::string ImageFormat_Dicom::name(){return "Dicom";}

data::ChunkList ImageFormat_Dicom::load( std::string filename, std::string dialect )
{
	DcmFileFormat dcfile;
	if(dcfile.loadFile(filename.c_str()).good()){
// 			dcfile.print(std::cout);
		DcmDataset* dcdata=dcfile.getDataset();
		util::PropMap myPropMap;
		dcmObject2PropMap(dcdata,myPropMap);
	}
	return data::ChunkList();
}

bool ImageFormat_Dicom::write(const data::Image &image,std::string filename,std::string dialect )
{
}
	
bool ImageFormat_Dicom::tainted(){return false;}//internal plugins are not tainted
size_t ImageFormat_Dicom::maxDim(){return 2;}
}}

isis::image_io::FileFormat* factory(){
	return new isis::image_io::ImageFormat_Dicom();
}
