#include "imageFormat_Dicom.hpp"
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "common.hpp"

namespace isis{ namespace image_io{
namespace _internal{
class DicomChunk : public data::Chunk{
	struct Deleter{
		DcmFileFormat *m_dcfile;
		DicomImage *m_img;
		std::string m_filename;
		Deleter(DcmFileFormat *dcfile,DicomImage *img,std::string filename):m_dcfile(dcfile),m_img(img),m_filename(filename){}
		void operator ()(void *at){
			LOG_IF(not m_dcfile, ImageIoLog,util::error)
				<< "Trying to close non existing dicom file";
			LOG_IF(not m_img, ImageIoLog,util::error)
				<< "Trying to close non existing dicom image";
			LOG(ImageIoDebug,util::info)	<< "Closing mapped dicom-file " << util::MSubject(m_filename) << " (pixeldata was at " << at << ")";
			delete m_img;
			delete m_dcfile;
		}
	};
	template<typename TYPE>	DicomChunk(
		TYPE* dat,Deleter del,
		size_t width,size_t height):
		data::Chunk(dat,del,width,height,1,1)
	{
		LOG(ImageIoDebug,util::info)
		<< "Mapping greyscale pixeldata of " << del.m_filename << " at "
		<< dat << " (" << util::TypePtr<TYPE>::staticName() << ")" ;
		DcmDataset* dcdata=del.m_dcfile->getDataset();
		util::PropMap &dcmMap = setProperty(ImageFormat_Dicom::dicomTagTreeName,util::PropMap());
		ImageFormat_Dicom::dcmObject2PropMap(dcdata,dcmMap);
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

using boost::posix_time::ptime;
using boost::gregorian::date;

const char ImageFormat_Dicom::dicomTagTreeName[]="DICOM";

std::string ImageFormat_Dicom::suffixes(){return std::string(".ima");}
std::string ImageFormat_Dicom::name(){return "Dicom";}


void ImageFormat_Dicom::sanitise(isis::util::PropMap& object, string dialect) {
	util::fvector4 voxelSize;
	const std::string prefix=std::string(ImageFormat_Dicom::dicomTagTreeName)+"/";

	// Fix voxelSize
	if(hasOrTell(prefix+"PixelSpacing",object,util::info)){
		voxelSize = object[prefix+"PixelSpacing"]->as<util::fvector4>();
		object.remove(prefix+"PixelSpacing");
	} else {
		LOG(ImageIoLog,util::warning) << "PixelSpacing not found, assuming <1,1>";
		voxelSize[0]=1;voxelSize[1]=1;
	}
	if(hasOrTell(prefix+"SliceThickness",object,util::warning)){
		voxelSize[2]=object[prefix+"SliceThickness"]->as<float>();
		object.remove(prefix+"SliceThickness");
	} else {
		LOG(ImageIoLog,util::warning) << "SliceThickness not found, assuming 1";
		voxelSize[2]=1;
	}		
	object.setProperty("voxelSize",voxelSize);
	
	// Fix indexOrigin/ImagePositionPatient
	transformOrTell<util::fvector4>(prefix+"ImagePositionPatient","indexOrigin",object,util::warning);

	// compute the "sequenceStart"
	if(hasOrTell(prefix+"SeriesTime",object,util::warning) && hasOrTell(prefix+"SeriesDate",object,util::warning)){
		const ptime seriesTime=object[prefix+"SeriesTime"]->as<ptime>();
		const date seriesDate=object[prefix+"SeriesDate"]->as<date>();
		const ptime sequenceStart(seriesDate,seriesTime.time_of_day());
		LOG(ImageIoDebug,util::verbose_info) << "Computed sequenceStart as " <<sequenceStart;
		
		object.setProperty("sequenceStart",sequenceStart);
		object.remove(prefix+"SeriesTime");
		object.remove(prefix+"SeriesDate");
	}

	// make sure "seriesNumber" is there
	hasOrTell(prefix+"seriesNumber",object,util::warning);
	
}

data::ChunkList ImageFormat_Dicom::load( const std::string& filename, const std::string& dialect )
{
	boost::shared_ptr<data::Chunk> chunk;
	
	DcmFileFormat *dcfile=new DcmFileFormat;
	if(dcfile->loadFile(filename.c_str()).good() and (chunk =_internal::DicomChunk::makeSingleMonochrome(filename,dcfile))){
		//we got a chunk from the file
		sanitise(*chunk,"");
		return data::ChunkList(1,*chunk);
	} else {
		delete dcfile;//no chunk was created, so we have to deal with the dcfile on our own
		LOG(ImageIoLog,util::error)
		<< "Failed to create a chunk from " << util::MSubject(filename);
	}
	return data::ChunkList();
}

bool ImageFormat_Dicom::write(const data::Image &image,const std::string& filename,const std::string& dialect )
{
	LOG(ImageIoLog,util::error)
		<< "writing dicom files is not yet supportet";
	return false;
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
