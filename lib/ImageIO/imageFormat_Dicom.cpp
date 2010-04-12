#include "imageFormat_Dicom.hpp"
#include "common.hpp"

#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmimage/diregist.h> //for color support
#include <boost/date_time/posix_time/posix_time.hpp>

namespace isis{ namespace image_io{
namespace _internal{
class DicomChunk : public data::Chunk{
	struct Deleter{
		DcmFileFormat *m_dcfile;
		DicomImage *m_img;
		std::string m_filename;
		Deleter(DcmFileFormat *dcfile,DicomImage *img,std::string filename):m_dcfile(dcfile),m_img(img),m_filename(filename){}
		void operator ()(void *at){
			LOG_IF(not m_dcfile, Runtime,error)
				<< "Trying to close non existing dicom file";
			LOG_IF(not m_img, Runtime,error)
				<< "Trying to close non existing dicom image";
			LOG(Debug,verbose_info)	<< "Closing mapped dicom-file " << util::MSubject(m_filename) << " (pixeldata was at " << at << ")";
			delete m_img;
			delete m_dcfile;
		}
	};
	template<typename TYPE>	DicomChunk(
		TYPE* dat,Deleter del,
		size_t width,size_t height):
		data::Chunk(dat,del,width,height,1,1)
	{
		LOG(Debug,verbose_info)
			<< "Mapping greyscale pixeldata of " << del.m_filename << " at "
			<< dat << " (" << util::TypePtr<TYPE>::staticName() << ")" ;
	}
	template<typename TYPE>
	static data::Chunk* copyColor(TYPE* source,size_t width,size_t height){
		data::Chunk *ret=new data::MemChunk<util::color<TYPE> >(width,height);
		util::TypePtr<util::color<TYPE> > &dest=ret->asTypePtr<util::color<TYPE> >();
		const size_t pixels = dest.len();
		for(size_t i=0;i<pixels;i++){
			util::color<TYPE> &dvoxel = dest[i];
			dvoxel.r=source[i];
			dvoxel.g=source[i+pixels];
			dvoxel.b=source[i+2*pixels];
		}
		return ret;
	}
public:
	static boost::shared_ptr<data::Chunk> makeChunk(std::string filename,DcmFileFormat *dcfile){
		boost::shared_ptr<data::Chunk> ret;
		
		DicomImage *img=new DicomImage(dcfile,EXS_Unknown);
		if(img->getStatus()==EIS_Normal){
			const DiPixel *const  pix=img->getInterData();
			const unsigned long width=img->getWidth(),height=img->getHeight();
			const void * const data=pix->getData();
			DcmDataset* dcdata=dcfile->getDataset();
			if(pix){
				if(img->isMonochrome()){ //try to load image directly from the raw monochrome dicom-data
					Deleter del(dcfile,img,filename);
					switch(pix->getRepresentation()){
						case EPR_Uint8: ret.reset(new DicomChunk((uint8_t*) data,del,width,height));break;
						case EPR_Sint8: ret.reset(new DicomChunk((int8_t*)  data,del,width,height));break;
						case EPR_Uint16:ret.reset(new DicomChunk((uint16_t*)data,del,width,height));break;
						case EPR_Sint16:ret.reset(new DicomChunk((int16_t*) data,del,width,height));break;
						case EPR_Uint32:ret.reset(new DicomChunk((uint32_t*)data,del,width,height));break;
						case EPR_Sint32:ret.reset(new DicomChunk((int32_t*) data,del,width,height));break;
						default:
							LOG(Runtime,error)<< "Unsupported datatype for monochrome images"; //@todo tell the user which datatype it is
					}
					if(ret){
						util::PropMap &dcmMap = ret->setProperty(ImageFormat_Dicom::dicomTagTreeName,util::PropMap());
						ImageFormat_Dicom::dcmObject2PropMap(dcdata,dcmMap);
						return ret;// get out of here - the source image must not be deleted if we created a chunk linked to it
					}
				} else if(pix->getPlanes()==3){ //try to load data as color image
					switch(pix->getRepresentation()){
						case EPR_Uint8: ret.reset(copyColor((Uint8* )data,width,height));break;
						case EPR_Uint16:ret.reset(copyColor((Uint16*)data,width,height));break;
						default:
							LOG(Runtime,error)<< "Unsupported datatype for color images"; //@todo tell the user which datatype it is
					}
					if(ret){
						util::PropMap &dcmMap = ret->setProperty(ImageFormat_Dicom::dicomTagTreeName,util::PropMap());
						ImageFormat_Dicom::dcmObject2PropMap(dcdata,dcmMap);
					}
				} else {
					LOG(Runtime,error)
						<< util::MSubject(filename) << " doest not have a supported pixel type. Won't load it";
				}
			} else {
				LOG(Runtime,error)
				<< "Didn't get any pixel data from " << util::MSubject(filename);
			}
			delete img;
		} else {
			LOG(Runtime,error) << "Failed to load image from " << filename << " (" << DicomImage::getString(img->getStatus()) << ")";
		}
		delete dcfile;		
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

	/////////////////////////////////////////////////////////////////////////////////
	// Transform known DICOM-Tags into default-isis-properties
	/////////////////////////////////////////////////////////////////////////////////
	
	// compute sequenceStart and acquisitionTime (have a look at table C.10.8 in the standart)
	if(hasOrTell(prefix+"SeriesTime",object,warning) && hasOrTell(prefix+"SeriesDate",object,warning)){
		const ptime sequenceStart = genTimeStamp(object[prefix+"SeriesDate"]->as<date>(),object[prefix+"SeriesTime"]->as<ptime>());

		// compute acquisitionTime
		if(hasOrTell(prefix+"AcquisitionTime",object,warning) and hasOrTell(prefix+"AcquisitionDate",object,warning)){
			const ptime acTime = genTimeStamp(object[prefix+"AcquisitionDate"]->as<date>(),object[prefix+"AcquisitionTime"]->as<ptime>());
			const boost::posix_time::time_duration acDist=acTime-sequenceStart;
			const float fAcDist=float(acDist.ticks()) / acDist.ticks_per_second();
			LOG(Debug,verbose_info) << "Computed acquisitionTime as " <<fAcDist;
			object.setProperty("acquisitionTime",fAcDist);
			object.remove(prefix+"AcquisitionTime");
			object.remove(prefix+"AcquisitionDate");
		}
      
		LOG(Debug,verbose_info) << "Computed sequenceStart as " <<sequenceStart;
		
		object.setProperty("sequenceStart",sequenceStart);
		object.remove(prefix+"SeriesTime");
		object.remove(prefix+"SeriesDate");
	}

	transformOrTell<u_int16_t>  (prefix+"SeriesNumber",     "sequenceNumber",     object,warning);
	transformOrTell<std::string>(prefix+"SeriesDescription","sequenceDescription",object,warning);
	transformOrTell<std::string>(prefix+"PatientsName",     "subjectName",        object,warning);
	transformOrTell<date>       (prefix+"PatientsBirthDate","subjectBirth",       object,warning);
	// @todo sex is missing
	transformOrTell<u_int16_t>  (prefix+"PatientsWeight",   "subjectWeigth",      object,warning);
	
	// compute voxelSize and gap
	{
		util::fvector4 voxelSize(invalid_float,invalid_float,invalid_float,invalid_float);
		if(hasOrTell(prefix+"PixelSpacing",object,warning)){
			voxelSize = object[prefix+"PixelSpacing"]->as<util::fvector4>();
			object.remove(prefix+"PixelSpacing");
			std::swap(voxelSize[0],voxelSize[1]); // the values are row-spacing (size in phase dir) /column spacing (size in read dir)
		}
		if(hasOrTell(prefix+"SliceThickness",object,warning)){
			voxelSize[2]=object[prefix+"SliceThickness"]->as<float>();
			object.remove(prefix+"SliceThickness");
		}
		object.setProperty("voxelSize",voxelSize);

		util::fvector4 voxelGap(invalid_float,invalid_float,invalid_float,invalid_float);
		if(hasOrTell(prefix+"RepetitionTime",object,warning)){
			voxelGap[3]=object[prefix+"RepetitionTime"]->as<float>()/1000;
			object.remove(prefix+"RepetitionTime");
		}
		if(hasOrTell(prefix+"SpacingBetweenSlices",object,info)){
			voxelGap[2]=object[prefix+"SpacingBetweenSlices"]->as<float>();
			if(voxelSize[2]!=invalid_float)
				voxelGap[2]-=voxelSize[2]; //SpacingBetweenSlices is the distance between the centers of the slices - so substract the slice SliceThickness here
			object.remove(prefix+"SpacingBetweenSlices");
		}
		if(voxelGap!=util::fvector4(invalid_float,invalid_float,invalid_float,invalid_float))
		  object.setProperty("voxelGap",voxelGap);
	}
	
	transformOrTell<std::string>   (prefix+"PerformingPhysiciansName","performingPhysician",object,info);
	transformOrTell<u_int16_t>     (prefix+"NumberOfAverages",        "numberOfAverages",   object,warning);

	if(hasOrTell(prefix+"ImageOrientationPatient",object,info)){
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
			LOG(Runtime,error) << "Could not extract read- and phaseVector from " << object[prefix+"ImageOrientationPatient"];
		}
	} else {
		LOG(Runtime,warning)<< "Making up read and phase vector, because the image lacks this information";
		object.setProperty("readVec" ,util::fvector4(1,0,0));
		object.setProperty("phaseVec",util::fvector4(0,1,0));
	}

	object.setProperty("indexOrigin",util::fvector4());
	if(hasOrTell(prefix+"ImagePositionPatient",object,info))
	{
		object["indexOrigin"]=object.getPropertyValue(prefix+"ImagePositionPatient")->as<util::fvector4>();
	} else {
		object["indexOrigin"]=util::fvector4();
		LOG(Runtime,warning)<< "Making up indexOrigin, because the image lacks this information";
	}
	transformOrTell<u_int32_t>(prefix+"InstanceNumber","acquisitionNumber",object,error);

	////////////////////////////////////////////////////////////////
	// Do some sanity checks on redundant tags
	////////////////////////////////////////////////////////////////

	if(object.hasProperty(prefix+"Unknown Tag(0019,1015)")){
		const util::fvector4 &org=object["indexOrigin"]->cast_to_Type<util::fvector4>();
		const util::PropertyValue &comp=object.getPropertyValue(prefix+"Unknown Tag(0019,1015)");
		if(comp==org) // will use the more lazy comparison because org is not a Type
			object.remove(prefix+"Unknown Tag(0019,1015)");
		else 
			LOG(Debug,warning)
			<< prefix+"Unknown Tag(0019,1015):" << comp << " differs from indexOrigin:"
			<< org << ", won't remove it";
	}
	if(object.hasProperty(prefix+"Unknown Tag(0051,100c)")){ //@todo siemens only
		std::string fov= object.getProperty<std::string>(prefix+"Unknown Tag(0051,100c)");
		float read,phase;
		if(std::sscanf(fov.c_str(),"FoV %f*%f",&read,&phase)==2){
			object.setProperty("fov",util::fvector4(read,phase,invalid_float,invalid_float));
		}
	}
}

void ImageFormat_Dicom::readMosaic(const data::Chunk& source, data::ChunkList& dest)
{
	// prepare some needed parameters
  	const std::string prefix=std::string(ImageFormat_Dicom::dicomTagTreeName)+"/";
	util::slist iType=source.getProperty<util::slist>(prefix+"ImageType");
	std::replace(iType.begin(),iType.end(),std::string("MOSAIC"),std::string("WAS_MOSAIC"));
	
	
	std::string NumberOfImagesInMosaicProp;
	if(source.hasProperty(prefix+"Unknown Tag(0019,100a)")){
		NumberOfImagesInMosaicProp=prefix+"Unknown Tag(0019,100a)";
	} else if(source.hasProperty(prefix+"CSAImageHeaderInfo/NumberOfImagesInMosaic")){
		NumberOfImagesInMosaicProp=prefix+"CSAImageHeaderInfo/NumberOfImagesInMosaic";
	} else {
		LOG(Runtime,error)	<< "Could not determine the number of images in the mosaic";
		return;
	}
	
	// All is fine, lets start
	u_int16_t images=source.getPropertyValue(NumberOfImagesInMosaicProp)->as<u_int16_t>();
	util::ivector4 size=source.sizeToVector();
	const u_int16_t matrixSize = std::ceil(std::sqrt(images));
	size[0]/=matrixSize;size[1]/=matrixSize;size[2]*=images;
	assert(size[3]==1);
	LOG(Debug,info) << "Decomposing a "<< source.sizeToVector() <<" mosaic-image into a " << size << " image";
	
	boost::shared_ptr<data::Chunk> newChunk(new data::Chunk(source.cloneToMem(size[0],size[1],size[2],size[3])));
	
	for(size_t slice=0;slice<size[2];slice++){
		for(size_t phase=0;phase<size[1];phase++){
				const size_t dpos[]={0,phase,slice,0};
				const size_t column=slice%matrixSize;
				const size_t row=slice/matrixSize;
				const size_t sstart[]={column*size[0],row*size[1]+phase,0,0};
				const size_t send[]={sstart[0]+size[0]-1,row*size[1]+phase,0,0};
				source.copyRange(sstart,send,*newChunk,dpos);
		}
	}
	
	// fix the properties
	static_cast<util::PropMap&>(*newChunk)=static_cast<const util::PropMap&>(source); //copy _only_ the Properties of source
	newChunk->remove(NumberOfImagesInMosaicProp); // we dont need that anymore
	newChunk->setProperty(prefix+"ImageType",iType);
	
	//remove the additional mosaic offset and recalc the fov if given
	//eg. if there is a 10x10 Mosaic, substract the half size of 9 Images from the indexOrigin
	util::fvector4 &origin=newChunk->getPropertyValue("indexOrigin")->cast_to_Type<util::fvector4>();
	const util::fvector4 voxelSize=newChunk->getProperty<util::fvector4>("voxelSize");
	util::fvector4 voxelGap;
	if(newChunk->hasProperty("voxelGap"))
		voxelGap=newChunk->getProperty<util::fvector4>("voxelGap");
	const util::fvector4 fovCorr=newChunk->getFoV(voxelSize,voxelGap)/2*(matrixSize-1);
	const util::fvector4 offset = (newChunk->getProperty<util::fvector4>("readVec")*fovCorr[0]) + (newChunk->getProperty<util::fvector4>("phaseVec")*fovCorr[1]);
	
	origin= origin+ offset;
	LOG(Debug,info) << "New origin: " << newChunk->getPropertyValue("indexOrigin");
	
	if(newChunk->hasProperty("fov")){
		util::fvector4 &ref=newChunk->getPropertyValue("fov")->cast_to_Type<util::fvector4>();
		ref[0]/=matrixSize;
		ref[1]/=matrixSize;
		LOG_IF(ref[2]!=invalid_float,Runtime,warning) << "Overriding defined slice FoV in mosaic image";
		ref[2]=newChunk->getFoV(voxelSize,voxelGap)[2];
		LOG(Debug,info) << "New fov: " << newChunk->getPropertyValue("fov");
	}
	dest.push_back(*newChunk);
}


int ImageFormat_Dicom::load(data::ChunkList &chunks, const std::string& filename, const std::string& dialect )
{
	boost::shared_ptr<data::Chunk> chunk;
	
	DcmFileFormat *dcfile=new DcmFileFormat;
	OFCondition loaded=dcfile->loadFile(filename.c_str());
	if(loaded.good()){
		if(chunk =_internal::DicomChunk::makeChunk(filename,dcfile)){
			//we got a chunk from the file
			sanitise(*chunk,"");
			chunk->setProperty("source",filename);
			const util::slist iType=chunk->getProperty<util::slist>(std::string(ImageFormat_Dicom::dicomTagTreeName)+"/"+"ImageType");
			if(std::find(iType.begin(),iType.end(),"MOSAIC")!=iType.end()){ // if its a mosaic
				LOG(Runtime,verbose_info) << "This seems to be an mosaic image, will decompose it";
				readMosaic(*chunk,chunks);
			} else {
				chunks.push_back(*chunk);
			}
			return 1;
		}
	} else {
		LOG(Runtime,error)
			<< "Failed to open file " << util::MSubject(filename) << ":" << loaded.text();
	}
	return 0;
}

bool ImageFormat_Dicom::write(const data::Image &image,const std::string& filename,const std::string& dialect )
{
	LOG(Runtime,error)
		<< "writing dicom files is not yet supportet";
	return false;
}
	
bool ImageFormat_Dicom::tainted(){return false;}//internal plugins are not tainted
}}

isis::image_io::FileFormat* factory(){
	if (not dcmDataDict.isDictionaryLoaded()){
		LOG(isis::image_io::Runtime,isis::error)
			<< "No data dictionary loaded, check environment variable "; //set DCMDICTPATH or fix DCM_DICT_DEFAULT_PATH in cfunix.h of dcmtk
		return NULL;
	}	
	return new isis::image_io::ImageFormat_Dicom();
}
