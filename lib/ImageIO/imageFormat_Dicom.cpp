#include <DataStorage/io_interface.h>
#include <dcmtk/config/cfunix.h> //@todo add switch for windows if needed
#include <dcmtk/dcmdata/dcfilefo.h>

namespace isis{ namespace image_io{

	
class ImageFormat_Dicom: public FileFormat{
	void dcmObject2PropMap(DcmObject* master_obj,util::PropMap &map)
	{
		for(DcmObject* obj=master_obj->nextInContainer(NULL);obj;obj=master_obj->nextInContainer(obj)){
			std::string name(const_cast<DcmTag&>(obj->getTag()).getTagName());
			if(obj->isLeaf()){
				DcmElement* elem=dynamic_cast<DcmElement*>(obj);
				OFString buff;
				
				switch(obj->getVR()){
					case EVR_AS:{ 	// age string (nnnD, nnnW, nnnM, nnnY)
						elem->getOFString(buff,0);
						std::cout << "age:" << buff << std::endl;
					}break;
					case EVR_DA:{
					elem->getOFString(buff,0);
					std::cout << "Date:" << buff << std::endl;
					}break;
					case EVR_FL:{
						Float32 buff;
						elem->getFloat32(buff);
						map[name]=buff;//if Float32 is float its fine, if not we will get an linker error here
					}
					case EVR_FD:{
						Float64 buff;
						elem->getFloat64(buff);
						map[name]=buff;//if Float64 is double its fine, if not we will get an linker error here
					}
					case EVR_SL:{ //signed long
						Sint32 buff;
						elem->getSint32(buff);
						map[name]=buff;
					}break;
					case EVR_SS:{ //signed short
						Sint16 buff;
						elem->getSint16(buff);
						map[name]=buff;
					}break;
					case EVR_UL:{ //unsigned long
						Uint32 buff;
						elem->getUint32(buff);
						map[name]=buff;
					}break;
					case EVR_US:{ //unsigned short
						Uint16 buff;
						elem->getUint16(buff);
						map[name]=buff;
					}break;
					case EVR_IS:{ //integer string
						elem->getOFString(buff,0);
						map[name]=boost::lexical_cast<int32_t>(buff);
					}
					case EVR_AE:	//Application Entity (string)
					case EVR_CS:	// Code String (string)
					case EVR_LT:	//long text
					case EVR_SH:	//short string
					case EVR_ST:	//short text
					case EVR_UT:	//Unlimited Text
					case EVR_PN:{	//Person Name
						elem->getOFString(buff,0);
						map[name]=buff;
					}break;
					default:{
						elem->getOFString(buff,0);
						std::cout << "Implement me"
						<< name << "("
						<< const_cast<DcmTag&>(obj->getTag()).getVRName() << "):"
						<< buff << std::endl;
					}break;
				}
/*				std::cout
				<< const_cast<DcmTag&>(obj->getTag()).getTagName() << " " //Tag Name
				<< const_cast<DcmTag&>(obj->getTag()).getVRName()  << "=" <<  obj->getVR() //Tag VR-Name
				<< const_cast<DcmTag&>(obj->getTag()).toString() << std::endl; //String rep of the key*/
			} else{
				map[name]=util::PropMap();
				dcmObject2PropMap(obj,map[name]->cast_to_Type<util::PropMap>());
			}
		}
	}
public:
	std::string suffixes(){return std::string(".ima");}
	std::string name(){return "Dicom";}

	data::ChunkList load ( std::string filename, std::string dialect )
	{
		DcmFileFormat dcfile;
		if(dcfile.loadFile(filename.c_str()).good())
			dcfile.print(std::cout);
		DcmDataset* dcdata=dcfile.getDataset();
		util::PropMap myPropMap;
		dcmObject2PropMap(dcdata,myPropMap);
		return data::ChunkList();
	}

	bool write(const data::Image &image,std::string filename,std::string dialect )
	{
	}
	
	bool tainted(){return false;}//internal plugins are not tainted
	size_t maxDim(){return 2;}
};
}}
isis::image_io::FileFormat* factory(){
	return new isis::image_io::ImageFormat_Dicom();
}

#undef USE_STD_CXX_INCLUDES
#undef HAVE_SSTREAM