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
const char ImageFormat_Dicom::unknownTagName[]="Unknown Tag";

std::string ImageFormat_Dicom::suffixes(){return std::string(".ima");}
std::string ImageFormat_Dicom::name(){return "Dicom";}


ptime ImageFormat_Dicom::genTimeStamp(const date& date, const ptime& time) {
	return ptime(date,time.time_of_day());
}


void ImageFormat_Dicom::sanitise(isis::util::PropMap& object, string dialect) {
	const std::string prefix=std::string(ImageFormat_Dicom::dicomTagTreeName)+"/";
	
	// compute sequenceStart and acquisitionTime
	if(hasOrTell(prefix+"SeriesTime",object,util::warning) && hasOrTell(prefix+"SeriesDate",object,util::warning)){
		const ptime sequenceStart = genTimeStamp(object[prefix+"SeriesDate"]->as<date>(),object[prefix+"SeriesTime"]->as<ptime>());

		// compute acquisitionTime
		if(hasOrTell(prefix+"AcquisitionTime",object,util::warning) and hasOrTell(prefix+"AcquisitionDate",object,util::warning)){
			const ptime acTime = genTimeStamp(object[prefix+"AcquisitionDate"]->as<date>(),object[prefix+"AcquisitionTime"]->as<ptime>());
			const boost::posix_time::time_duration acDist=acTime-sequenceStart;
			const float fAcDist=float(acDist.ticks()) / acDist.ticks_per_second();
			LOG(ImageIoDebug,util::verbose_info) << "Computed acquisitionTime as " <<fAcDist;
			object.setProperty("acquisitionTime",fAcDist);
			object.remove(prefix+"AcquisitionTime");
			object.remove(prefix+"AcquisitionDate");
		}
      
		LOG(ImageIoDebug,util::verbose_info) << "Computed sequenceStart as " <<sequenceStart;
		
		object.setProperty("sequenceStart",sequenceStart);
		object.remove(prefix+"SeriesTime");
		object.remove(prefix+"SeriesDate");
	}

	transformOrTell<u_int16_t>  (prefix+"SeriesNumber",     "sequenceNumber",     object,util::warning);
	transformOrTell<std::string>(prefix+"SeriesDescription","sequenceDescription",object,util::warning);
	transformOrTell<std::string>(prefix+"PatientsName",     "subjectName",        object,util::warning);
	transformOrTell<date>       (prefix+"PatientsBirthDate","subjectBirth",       object,util::warning);
	// @todo sex is missing
	transformOrTell<u_int16_t>  (prefix+"PatientsWeight",   "subjectWeigth",      object,util::warning);
	
	// compute voxelSize
	util::fvector4 voxelSize;
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

	// Compute voxel gap
	const float nan=std::numeric_limits<float>::quiet_NaN();
	util::fvector4 voxelGap(nan,nan,nan,nan);
	if(hasOrTell(prefix+"RepetitionTime",object,util::warning)){
		voxelGap[4]=object[prefix+"RepetitionTime"]->as<float>();
		object.remove(prefix+"RepetitionTime");
	}
	object.setProperty("voxelGap",voxelGap);

	transformOrTell<float>         (prefix+"EchoTime",                "echoTime",           object,util::warning);
	transformOrTell<std::string>   (prefix+"PerformingPhysiciansName","performingPhysician",object,util::warning);
	transformOrTell<u_int16_t>     (prefix+"NumberOfAverages",        "numberOfAverages",   object,util::warning);

	if(hasOrTell(prefix+"ImageOrientationPatient",object,util::error)){
		util::dlist buff=object[prefix+"ImageOrientationPatient"]->as<util::dlist>();
		if(buff.size()==6){
			util::fvector4 read,phase;
			util::dlist::iterator b=buff.begin();
			for(int i=0;i<3;i++)read[i]=*b++;
			for(int i=0;i<3;i++)phase[i]=*b++;
			object.setProperty("readVec" ,read);
			object.setProperty("phaseVec",phase);
			object.remove(prefix+"ImageOrientationPatient");
		} else {
			LOG(ImageIoLog,util::error) << "Could not extract read- and phaseVector from " << object[prefix+"ImageOrientationPatient"];
		}
	}

	transformOrTell<util::fvector4>(prefix+"ImagePositionPatient",    "indexOrigin",        object,util::warning);
	transformOrTell<u_int32_t>     (prefix+"InstanceNumber",          "acquisitionNumber",  object,util::error);
}

int ImageFormat_Dicom::load(data::ChunkList &chunks, const std::string& filename, const std::string& dialect )
{
	boost::shared_ptr<data::Chunk> chunk;
	
	DcmFileFormat *dcfile=new DcmFileFormat;
	if(dcfile->loadFile(filename.c_str()).good() and (chunk =_internal::DicomChunk::makeSingleMonochrome(filename,dcfile))){
		//we got a chunk from the file
		sanitise(*chunk,"");
		chunks.push_back(*chunk);
		return 1;
	} else {
		delete dcfile;//no chunk was created, so we have to deal with the dcfile on our own
		LOG(ImageIoLog,util::error)
		<< "Failed to create a chunk from " << util::MSubject(filename);
	}
	return 0;
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
			<< "No data dictionary loaded, check environment variable "; //set DCMDICTPATH or fix DCM_DICT_DEFAULT_PATH in cfunix.h of dcmtk
		return NULL;
	}	
	return new isis::image_io::ImageFormat_Dicom();
}
