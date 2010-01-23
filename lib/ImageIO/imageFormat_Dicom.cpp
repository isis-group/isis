#include "imageFormat_Dicom.hpp"
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmimgle/dcmimage.h>

namespace isis{ namespace image_io{

namespace _internal{

class DicomChunk : public data::Chunk{
	struct Deleter{
		DcmFileFormat *m_dcfile;
		DicomImage *m_img;
		std::string m_filename;
		Deleter(DcmFileFormat *dcfile,DicomImage *img,std::string filename):m_dcfile(dcfile),m_img(img),m_filename(filename){}
		void operator ()(void *){
			LOG_IF(not m_dcfile, ImageIoLog,util::error)
				<< "Trying to close non existing dicom file";
			LOG_IF(not m_img, ImageIoLog,util::error)
				<< "Trying to close non existing dicom image";
			LOG(ImageIoDebug,util::info)	<< "Closing dicom-file " << util::MSubject(m_filename);
			delete m_img;
			delete m_dcfile;
		}
	};
	template<typename TYPE>	DicomChunk(
		TYPE* dat,
		Deleter del,
		size_t width,size_t height):
		data::Chunk(dat,del,width,height,1,1)
	{
		DcmDataset* dcdata=del.m_dcfile->getDataset();
		ImageFormat_Dicom::dcmObject2PropMap(dcdata,properties);
	}
public:
	static boost::shared_ptr<data::Chunk> makeSingleMonochrome(std::string filename,DcmFileFormat *dcfile){
		boost::shared_ptr<data::Chunk> ret;
		
		DicomImage *img=new DicomImage(dcfile,EXS_Unknown);
		if(img->isMonochrome()){
			Deleter del(dcfile,img,filename);
			const DiPixel *pix=img->getInterData();
			const unsigned long width=img->getWidth(),height=img->getHeight();
			if(pix)switch(pix->getRepresentation()){
				case EPR_Uint8: ret.reset(new DicomChunk((uint8_t*) pix->getData(),del,width,height));break;
				case EPR_Sint8: ret.reset(new DicomChunk((int8_t*)  pix->getData(),del,width,height));break;
				case EPR_Uint16:ret.reset(new DicomChunk((uint16_t*)pix->getData(),del,width,height));break;
				case EPR_Sint16:ret.reset(new DicomChunk((int16_t*) pix->getData(),del,width,height));break;
				case EPR_Uint32:ret.reset(new DicomChunk((uint32_t*)pix->getData(),del,width,height));break;
				case EPR_Sint32:ret.reset(new DicomChunk((int32_t*) pix->getData(),del,width,height));break;
			} else {
				LOG(ImageIoLog,util::error)
					<< "Didn't get any pixel data from " << util::MSubject(filename);
			}
		} else {
			LOG(ImageIoLog,util::error)
			<< util::MSubject(filename) << " is not an monochrome image. Won't load it";
			delete img;
		}
		return ret;
	}
};

}

std::string ImageFormat_Dicom::suffixes(){return std::string(".ima");}
std::string ImageFormat_Dicom::name(){return "Dicom";}



data::ChunkList ImageFormat_Dicom::load( const std::string& filename, const std::string& dialect )
{
	boost::shared_ptr<data::Chunk> chunk;
	
	DcmFileFormat *dcfile=new DcmFileFormat;
	if(dcfile->loadFile(filename.c_str()).good() and (chunk =_internal::DicomChunk::makeSingleMonochrome(filename,dcfile))){
		//we got a chunk from the file
		return data::ChunkList(1,*chunk);
	} else {
		delete dcfile;//not chunk was created, so we have to deal with the dcfile on our own
		LOG(ImageIoLog,util::error)
		<< "Failed to create a chunk from " << util::MSubject(filename);
	}
	return data::ChunkList();
}

bool ImageFormat_Dicom::write(const data::Image &image,const std::string& filename,const std::string& dialect )
{
	LOG(ImageIoLog,util::error)
		<< "writing dicom files is not yet supportet";
}
	
bool ImageFormat_Dicom::tainted(){return false;}//internal plugins are not tainted
size_t ImageFormat_Dicom::maxDim(){return 2;}
}}

isis::image_io::FileFormat* factory(){
	if (not dcmDataDict.isDictionaryLoaded()){
		LOG(isis::image_io::ImageIoLog,isis::util::error)
			<< "No data dictionary loaded, check environment variable ";
		return NULL;
	}	
	return new isis::image_io::ImageFormat_Dicom();
}
