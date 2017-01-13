#include "imageFormat_Dicom.hpp"
#include <isis/data/common.hpp>
#include <isis/util/istring.hpp>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmimage/diregist.h> //for color support
#include <dcmtk/dcmdata/dcdicent.h>
#include <dcmtk/oflog/config.h>
#include <dcmtk/oflog/tstring.h>
#include <dcmtk/oflog/spi/logevent.h>

#include <dcmtk/dcmjpeg/djdecode.h>    /* for dcmjpeg decoders */
#include <dcmtk/dcmjpeg/dipijpeg.h>    /* for dcmimage JPEG plugin */

namespace isis
{
namespace image_io
{
namespace _internal
{
class DicomChunk : public data::Chunk
{
	template<typename TYPE> DicomChunk(TYPE *dat,size_t width, size_t height ):data::Chunk( data::MemChunk<TYPE>(width, height) ) {
		LOG( Debug, verbose_info )
				<< "Copying greyscale pixeldata of into " << dat << " (" << data::ValueArray<TYPE>::staticName() << ")" ;
		asValueArrayBase().copyFromMem<TYPE>(dat,width*height);
	}
	template<typename TYPE>
	static data::Chunk *copyColor( TYPE **source, size_t width, size_t height ) {
		const size_t pixels = width*height;
		data::ValueArray<util::color<TYPE> > dest(pixels);

		for ( size_t i = 0; i < pixels; i++ ) {
			util::color<TYPE> &dvoxel = dest[i];
			dvoxel.r = source[0][i];
			dvoxel.g = source[1][i];
			dvoxel.b = source[2][i];
		}
		return new data::Chunk(dest,width,height);
	}
public:
	static data::Chunk makeChunk( const ImageFormat_Dicom &loader, DcmFileFormat &dcfile, const util::istring &dialect ) {
		std::unique_ptr<data::Chunk> ret;
		DicomImage img( &dcfile, EXS_Unknown );

		if ( img.getStatus() == EIS_Normal ) {
			const DiPixel *const  pix = img.getInterData();
			const unsigned long width = img.getWidth(), height = img.getHeight();
			const void *const data = pix->getData();
			DcmDataset *dcdata = dcfile.getDataset();

			if ( pix ) {
				if ( img.isMonochrome() ) { //try to load image directly from the raw monochrome dicom-data
					switch ( pix->getRepresentation() ) {
					case EPR_Uint8:
						ret.reset( new DicomChunk( ( uint8_t * ) data, width, height ) );
						break;
					case EPR_Sint8:
						ret.reset( new DicomChunk( ( int8_t * )  data, width, height ) );
						break;
					case EPR_Uint16:
						ret.reset( new DicomChunk( ( uint16_t * )data, width, height ) );
						break;
					case EPR_Sint16:
						ret.reset( new DicomChunk( ( int16_t * ) data, width, height ) );
						break;
					case EPR_Uint32:
						ret.reset( new DicomChunk( ( uint32_t * )data, width, height ) );
						break;
					case EPR_Sint32:
						ret.reset( new DicomChunk( ( int32_t * ) data, width, height ) );
						break;
					default:
						FileFormat::throwGenericError( "Unsupported datatype for monochrome images" ); //@todo tell the user which datatype it is
					}

					if ( ret ) {
						loader.dcmObject2PropMap( dcdata, ret->touchBranch( ImageFormat_Dicom::dicomTagTreeName ), dialect );
					}
				} else if ( pix->getPlanes() == 3 ) { //try to load data as color image
					// if there are 3 planes data is actually an array of 3 pointers
					switch ( pix->getRepresentation() ) {
					case EPR_Uint8:
						ret.reset( copyColor( ( Uint8 ** )data, width, height ) );
						break;
					case EPR_Uint16:
						ret.reset( copyColor( ( Uint16 ** )data, width, height ) );
						break;
					default:
						FileFormat::throwGenericError( "Unsupported datatype for color images" ); //@todo tell the user which datatype it is
					}

					if ( ret ) {
						loader.dcmObject2PropMap( dcdata, ret->touchBranch( ImageFormat_Dicom::dicomTagTreeName ), dialect );
					}
				} else {
					FileFormat::throwGenericError( "Unsupported pixel type." );
				}
			} else {
				FileFormat::throwGenericError( "Didn't get any pixel data" );
			}
		} else {
			FileFormat::throwGenericError( std::string( "Failed to open image: " ) + DicomImage::getString( img.getStatus() ));
		}

		return *ret;
	}
};

class DcmtkLogger : public LOG4CPLUS_NAMESPACE::Appender{
	std::set<LOG4CPLUS_NAMESPACE::tstring> ignores;
public:
	DcmtkLogger(){
		ignores.insert("no pixel data found in DICOM dataset");
	}
	virtual void close(){}
protected:
	virtual void append(const LOG4CPLUS_NAMESPACE::spi::InternalLoggingEvent& event){
		const LOG4CPLUS_NAMESPACE::tstring &msg=event.getMessage();
		LOG_IF(ignores.find(msg)==ignores.end(),Runtime,warning) << "Got an error from dcmtk: \"" << event.getMessage() << "\"";
	}
};
}

const char ImageFormat_Dicom::dicomTagTreeName[] = "DICOM";
const char ImageFormat_Dicom::unknownTagName[] = "UnknownTag/";

util::istring ImageFormat_Dicom::suffixes( io_modes modes )const
{
	if( modes == write_only )
		return util::istring();
	else
		return ".ima .dcm";
}
std::string ImageFormat_Dicom::getName()const {return "Dicom";}
util::istring ImageFormat_Dicom::dialects( const std::string &/*filename*/ )const {return "siemens withExtProtocols nocsa keepmosaic forcemosaic";}



void ImageFormat_Dicom::addDicomDict( DcmDataDictionary &dict )
{
	for( DcmHashDictIterator i = dict.normalBegin(); i != dict.normalEnd(); i++ ) {
		const DcmDictEntry *entry = *i;
		const DcmTagKey key = entry->getKey();
		const char *name = entry->getTagName();

		if( util::istring( "Unknown" ) == name ) {
			dictionary[key] = util::istring( unknownTagName ) + key.toString().c_str();
		} else
			dictionary[key] = name;
	}
}


void ImageFormat_Dicom::sanitise( util::PropertyMap &object, util::istring dialect )
{
	const util::istring prefix = util::istring( ImageFormat_Dicom::dicomTagTreeName ) + "/";
	util::PropertyMap &dicomTree = object.touchBranch( dicomTagTreeName );
	/////////////////////////////////////////////////////////////////////////////////
	// Transform known DICOM-Tags into default-isis-properties
	/////////////////////////////////////////////////////////////////////////////////

	if( dicomTree.hasProperty( "SiemensNumberOfImagesInMosaic" ) ) { // if its still there image was no mosaic, so I guess it should be used according to the standard
		dicomTree.rename( "SiemensNumberOfImagesInMosaic", "SliceOrientation" );
	}

	// compute sequenceStart and acquisitionTime (have a look at table C.10.8 in the standard)
	{
		// get series start time (remember this is in UTC)
		auto o_seqStart=extractOrTell("SeriesTime",dicomTree,warning );
		if(o_seqStart) {
			auto o_acDate= extractOrTell({"SeriesDate", "AcquisitionDate", "ContentDate"},dicomTree,warning);
			if( o_acDate ) { // add days since epoch from the date
				const util::timestamp seqStart = o_seqStart->as<util::timestamp>()+o_acDate->as<util::date>().time_since_epoch();
				object.setValueAs( "sequenceStart", seqStart);
				LOG(Debug,verbose_info) 
					<< "Merging Series Time " << *o_seqStart << " and Date " << *o_acDate << " as " 
					<< std::make_pair("sequenceStart",object.property("sequenceStart"));
			}
		}
	}
	{
		// compute acquisitionTime
		auto o_acTime= extractOrTell({"AcquisitionTime","ContentTime"},dicomTree,warning);
		if ( o_acTime ) {
			auto o_acDate= extractOrTell({"AcquisitionDate", "ContentDate", "SeriesDate"},dicomTree,warning);
			if( o_acDate ) {
				const util::timestamp acTime = o_acTime->as<util::timestamp>()+o_acDate->as<util::date>().time_since_epoch();
				object.setValueAs<util::timestamp>("acquisitionTime", acTime);
				LOG(Debug,verbose_info) 
					<< "Merging Content Time " << *o_acTime << " and Date " << *o_acDate
					<< " as " << std::make_pair("acquisitionTime",object.property("acquisitionTime"));
			}
		}
	}

	// compute studyStart
	if ( hasOrTell( "StudyTime", dicomTree, warning ) && hasOrTell( "StudyDate", dicomTree, warning ) ) {
		const util::date dt=dicomTree.getValueAs<util::date>("StudyDate");
		const util::timestamp tm=dicomTree.getValueAs<util::timestamp>("StudyTime");
			object.setValueAs("studyStart",tm+dt.time_since_epoch());
			dicomTree.remove("StudyTime");
			dicomTree.remove("StudyDate");
	}
	
	transformOrTell<uint16_t>  ( prefix + "SeriesNumber",     "sequenceNumber",     object, warning );
	transformOrTell<uint16_t>  ( prefix + "PatientsAge",     "subjectAge",     object, info );
	transformOrTell<std::string>( prefix + "SeriesDescription", "sequenceDescription", object, warning );
	transformOrTell<std::string>( prefix + "PatientsName",     "subjectName",        object, info );
	transformOrTell<util::date>       ( prefix + "PatientsBirthDate", "subjectBirth",       object, info );
	transformOrTell<uint16_t>  ( prefix + "PatientsWeight",   "subjectWeigth",      object, info );
	// compute voxelSize and gap
	{
		util::fvector3 voxelSize( {invalid_float, invalid_float, invalid_float} );
		const util::istring pixelsize_params[]={"PixelSpacing","ImagePlanePixelSpacing","ImagerPixelSpacing"};
		for(const util::istring &name:pixelsize_params){
			if ( hasOrTell( prefix + name, object, warning ) ) {
				voxelSize = dicomTree.getValueAs<util::fvector3>( name );
				dicomTree.remove( name );
				std::swap( voxelSize[0], voxelSize[1] ); // the values are row-spacing (size in column dir) /column spacing (size in row dir)
				break;
			}
			
		}

		if ( hasOrTell( prefix + "SliceThickness", object, warning ) ) {
			voxelSize[2] = dicomTree.getValueAs<float>( "SliceThickness" );
			dicomTree.remove( "SliceThickness" );
		} else {
			voxelSize[2] = 1 / object.getValueAs<float>( "DICOM/CSASeriesHeaderInfo/SliceResolution" );
		}
		
		object.setValueAs( "voxelSize", voxelSize );
		transformOrTell<uint16_t>( prefix + "RepetitionTime", "repetitionTime", object, warning );
		transformOrTell<float>( prefix + "EchoTime", "echoTime", object, warning );
		transformOrTell<int16_t>( prefix + "FlipAngle", "flipAngle", object, warning );

		if ( hasOrTell( prefix + "SpacingBetweenSlices", object, info ) ) {
			if ( voxelSize[2] != invalid_float ) {
				object.setValueAs( "voxelGap", util::fvector3( {0, 0, dicomTree.getValueAs<float>( "SpacingBetweenSlices" ) - voxelSize[2]} ) );
				dicomTree.remove( "SpacingBetweenSlices" );
			} else
				LOG( Runtime, warning )
						<< "Cannot compute the voxel gap from the slice spacing ("
						<< object.property( prefix + "SpacingBetweenSlices" )
						<< "), because the slice thickness is not known";
		}
	}
	transformOrTell<std::string>   ( prefix + "PerformingPhysiciansName", "performingPhysician", object, info );
	transformOrTell<uint16_t>     ( prefix + "NumberOfAverages",        "numberOfAverages",   object, warning );

	if ( hasOrTell( prefix + "ImageOrientationPatient", object, info ) ) {
		util::dlist buff = dicomTree.getValueAs<util::dlist>( "ImageOrientationPatient" );

		if ( buff.size() == 6 ) {
			util::fvector3 row, column;
			util::dlist::iterator b = buff.begin();

			for ( int i = 0; i < 3; i++ )row[i] = *b++;

			for ( int i = 0; i < 3; i++ )column[i] = *b++;

			object.setValueAs( "rowVec" , row );
			object.setValueAs( "columnVec", column );
			dicomTree.remove( "ImageOrientationPatient" );
		} else {
			LOG( Runtime, error ) << "Could not extract row- and columnVector from " << dicomTree.property( "ImageOrientationPatient" );
		}

		if( object.hasProperty( prefix + "CSAImageHeaderInfo/SliceNormalVector" ) && !object.hasProperty( "sliceVec" ) ) {
			LOG( Debug, info ) << "Extracting sliceVec from CSAImageHeaderInfo/SliceNormalVector " << dicomTree.property( "CSAImageHeaderInfo/SliceNormalVector" );
			util::dlist list = dicomTree.getValueAs<util::dlist >( "CSAImageHeaderInfo/SliceNormalVector" );
			util::fvector3 vec;
			std::copy(list.begin(), list.end(), std::begin(vec) );
			object.setValueAs( "sliceVec", vec );
			dicomTree.remove( "CSAImageHeaderInfo/SliceNormalVector" );
		}
	} else {
		LOG( Runtime, warning ) << "Making up row and column vector, because the image lacks this information";
		object.setValueAs( "rowVec" , util::fvector3( {1, 0, 0} ) );
		object.setValueAs( "columnVec", util::fvector3( {0, 1, 0} ) );
	}

	if ( hasOrTell( prefix + "ImagePositionPatient", object, info ) ) {
		object.setValueAs( "indexOrigin", dicomTree.getValueAs<util::fvector3>( "ImagePositionPatient" ) );
	} else if( object.hasProperty( prefix + "CSAImageHeaderInfo/ProtocolSliceNumber" ) ) {
		util::fvector3 orig( {0, 0, object.getValueAs<float>( prefix + "CSAImageHeaderInfo/ProtocolSliceNumber" ) / object.getValueAs<float>( "DICOM/CSASeriesHeaderInfo/SliceResolution" )} );
		LOG( Runtime, info ) << "Synthesize missing indexOrigin from CSAImageHeaderInfo/ProtocolSliceNumber as " << orig;
		object.setValueAs( "indexOrigin", orig );
	} else {
		object.setValueAs( "indexOrigin", util::fvector3() );
		LOG( Runtime, warning ) << "Making up indexOrigin, because the image lacks this information";
	}

	transformOrTell<uint32_t>( prefix + "InstanceNumber", "acquisitionNumber", object, error );

	if( dicomTree.hasProperty( "AcquisitionNumber" ) && object.property( "acquisitionNumber" ) == dicomTree.property( "AcquisitionNumber" ) )
		dicomTree.remove( "AcquisitionNumber" );

	if ( hasOrTell( prefix + "PatientsSex", object, info ) ) {
		util::Selection isisGender( "male,female,other" );
		bool set = false;

		switch ( dicomTree.getValueAs<std::string>( "PatientsSex" )[0] ) {
		case 'M':
			isisGender.set( "male" );
			set = true;
			break;
		case 'F':
			isisGender.set( "female" );
			set = true;
			break;
		case 'O':
			isisGender.set( "other" );
			set = true;
			break;
		default:
			LOG( Runtime, warning ) << "Dicom gender code " << util::MSubject( object.property( prefix + "PatientsSex" ) ) <<  " not known";
		}

		if( set ) {
			object.setValueAs( "subjectGender", isisGender);
			dicomTree.remove( "PatientsSex" );
		}
	}

	transformOrTell<uint32_t>( prefix + "CSAImageHeaderInfo/UsedChannelMask", "coilChannelMask", object, info );
	////////////////////////////////////////////////////////////////
	// interpret DWI data
	////////////////////////////////////////////////////////////////
	int32_t bValue;
	bool foundDiff = true;

	// find the B-Value
	if ( dicomTree.hasProperty( "DiffusionBValue" ) ) { //in case someone actually used the right Tag
		bValue = dicomTree.getValueAs<int32_t>( "DiffusionBValue" );
		dicomTree.remove( "DiffusionBValue" );
	} else if ( dicomTree.hasProperty( "SiemensDiffusionBValue" ) ) { //fallback for siemens
		bValue = dicomTree.getValueAs<int32_t>( "SiemensDiffusionBValue" );
		dicomTree.remove( "SiemensDiffusionBValue" );
	} else foundDiff = false;

	// If we do have DWI here, create a property diffusionGradient (which defaults to 0,0,0)
	if( foundDiff ) {
		if( dialect == "siemens" ) {
			LOG( Runtime, warning ) << "Removing acquisitionTime=" << util::MSubject( object.property( "acquisitionTime" ).toString( false ) ) << " from siemens DWI data as it is probably broken";
			object.remove( "acquisitionTime" );
		}

		bool foundGrad=false;
		if( dicomTree.hasProperty( "DiffusionGradientOrientation" ) ) {
			foundGrad= object.transform<util::fvector3>(prefix+"DiffusionGradientOrientation","diffusionGradient");
		} else if( dicomTree.hasProperty( "SiemensDiffusionGradientOrientation" ) ) {
			foundGrad= object.transform<util::fvector3>(prefix+"SiemensDiffusionGradientOrientation","diffusionGradient");
		} else {
			if(bValue)
			LOG( Runtime, error ) << "Found no diffusion direction for DiffusionBValue " << util::MSubject( bValue );
			else
				LOG(Runtime, notice ) << "Ignoring DiffusionBValue 0 as there is no diffusionGradient";
		}

		if( bValue && foundGrad ) // if bValue is not zero multiply the diffusionGradient by it
			object.refValueAs<util::fvector3>("diffusionGradient")*=bValue;
	}


	//@todo fallback for GE/Philips
	////////////////////////////////////////////////////////////////
	// Do some sanity checks on redundant tags
	////////////////////////////////////////////////////////////////
	if ( dicomTree.hasProperty( util::istring( unknownTagName ) + "(0019,1015)" ) ) {
		const util::fvector3 org = object.getValueAs<util::fvector3>( "indexOrigin" );
		const util::fvector3 comp = dicomTree.getValueAs<util::fvector3>( util::istring( unknownTagName ) + "(0019,1015)" );

		if ( util::fuzzyEqualV(comp, org ) )
			dicomTree.remove( util::istring( unknownTagName ) + "(0019,1015)" );
		else
			LOG( Debug, warning )
					<< prefix + util::istring( unknownTagName ) + "(0019,1015):" << dicomTree.property( util::istring( unknownTagName ) + "(0019,1015)" )
					<< " differs from indexOrigin:" << object.property( "indexOrigin" ) << ", won't remove it";
	}

	if(
		dicomTree.hasProperty( "CSAImageHeaderInfo/MosaicRefAcqTimes" ) &&
		dicomTree.hasProperty( util::istring( unknownTagName ) + "(0019,1029)" ) &&
		dicomTree.property( util::istring( unknownTagName ) + "(0019,1029)" ) == dicomTree.property( "CSAImageHeaderInfo/MosaicRefAcqTimes" )
	) {
		dicomTree.remove( util::istring( unknownTagName ) + "(0019,1029)" );
	}

	if ( dicomTree.hasProperty( util::istring( unknownTagName ) + "(0051,100c)" ) ) { //@todo siemens only ?
		std::string fov = dicomTree.getValueAs<std::string>( util::istring( unknownTagName ) + "(0051,100c)" );
		float row, column;

		if ( std::sscanf( fov.c_str(), "FoV %f*%f", &column, &row ) == 2 ) {
			object.setValueAs( "fov", util::fvector3( {row, column, invalid_float} ) );
		}
	}
	
	auto windowCenterQuery=dicomTree.queryProperty("WindowCenter");
	auto windowWidthQuery=dicomTree.queryProperty("WindowCenter");
	if( windowCenterQuery && windowWidthQuery){
		const double windowCenter=windowCenterQuery->as<double>(), windowWidth= windowWidthQuery->as<double>();
		windowCenterQuery.reset();windowWidthQuery.reset();
		dicomTree.remove("WindowCenter");dicomTree.remove("WindowWidth");
		object.setValueAs("window/min",windowCenter-windowWidth/2);
		object.setValueAs("window/max",windowCenter+windowWidth/2);
	}
}

data::Chunk ImageFormat_Dicom::readMosaic( data::Chunk source )
{
	// prepare some needed parameters
	const util::istring prefix = util::istring( ImageFormat_Dicom::dicomTagTreeName ) + "/";
	util::slist iType = source.getValueAs<util::slist>( prefix + "ImageType" );
	std::replace( iType.begin(), iType.end(), std::string( "MOSAIC" ), std::string( "WAS_MOSAIC" ) );
	util::istring NumberOfImagesInMosaicProp;

	if ( source.hasProperty( prefix + "SiemensNumberOfImagesInMosaic" ) ) {
		NumberOfImagesInMosaicProp = prefix + "SiemensNumberOfImagesInMosaic";
	} else if ( source.hasProperty( prefix + "CSAImageHeaderInfo/NumberOfImagesInMosaic" ) ) {
		NumberOfImagesInMosaicProp = prefix + "CSAImageHeaderInfo/NumberOfImagesInMosaic";
	}

	// All is fine, lets start
	uint16_t images;
	if(NumberOfImagesInMosaicProp.empty()){
		images = source.getSizeAsVector()[0]/ source.getValueAs<util::ilist>( prefix+"AcquisitionMatrix" ).front();
		images*=images;
		LOG(Debug,warning) << "Guessing number of slices in the mosaic as " << images << ". This might be to many";
	} else
		images = source.getValueAs<uint16_t>( NumberOfImagesInMosaicProp );
	
	const util::vector4<size_t> tSize = source.getSizeAsVector();
	const uint16_t matrixSize = std::ceil( std::sqrt( images ) );
	const util::vector3<size_t> size( {tSize[0] / matrixSize, tSize[1] / matrixSize, images} );

	LOG( Debug, info ) << "Decomposing a " << source.getSizeAsString() << " mosaic-image into a " << size << " volume";
	// fix the properties of the source (we 'll need them later)
	const util::fvector3 voxelGap = source.getValueAsOr("voxelGap",util::fvector3());
	const util::fvector3 voxelSize = source.getValueAs<util::fvector3>( "voxelSize" );
	const util::fvector3 rowVec = source.getValueAs<util::fvector3>( "rowVec" );
	const util::fvector3 columnVec = source.getValueAs<util::fvector3>( "columnVec" );
	//remove the additional mosaic offset
	//eg. if there is a 10x10 Mosaic, substract the half size of 9 Images from the offset
	const util::fvector3 fovCorr = ( voxelSize + voxelGap ) * size * ( matrixSize - 1 ) / 2; // @todo this will not include the voxelGap between the slices
	util::fvector3 &origin = source.refValueAs<util::fvector3>( "indexOrigin" );
	origin = origin + ( rowVec * fovCorr[0] ) + ( columnVec * fovCorr[1] );
	source.remove( NumberOfImagesInMosaicProp ); // we dont need that anymore
	source.setValueAs( prefix + "ImageType", iType );

	//store and remove acquisitionTime
	std::list<double> acqTimeList;
	std::list<double>::const_iterator acqTimeIt;

	bool haveAcqTimeList = source.hasProperty( prefix + "CSAImageHeaderInfo/MosaicRefAcqTimes" );
	isis::util::timestamp acqTime;

	if( haveAcqTimeList ) {
		acqTimeList = source.getValueAs<std::list<double> >( prefix + "CSAImageHeaderInfo/MosaicRefAcqTimes" );
		source.remove( prefix + "CSAImageHeaderInfo/MosaicRefAcqTimes" );
		acqTimeIt = acqTimeList.begin();
		LOG( Debug, info ) << "The acquisition time offsets of the slices in the mosaic where " << acqTimeList;
	}

	if( source.hasProperty( "acquisitionTime" ) )acqTime = source.getValueAs<isis::util::timestamp>( "acquisitionTime" );
	else {
		LOG_IF( haveAcqTimeList, Runtime, info ) << "Ignoring CSAImageHeaderInfo/MosaicRefAcqTimes because there is no acquisitionTime";
		haveAcqTimeList = false;
	}

	data::Chunk dest = source.cloneToNew( size[0], size[1], size[2] ); //create new 3D chunk of the same type
	static_cast<util::PropertyMap &>( dest ) = static_cast<const util::PropertyMap &>( source ); //copy _only_ the Properties of source
	// update origin
	dest.setValueAs( "indexOrigin", origin );

	// update fov
	if ( dest.hasProperty( "fov" ) ) {
		util::fvector3 &ref = dest.refValueAs<util::fvector3>( "fov" );
		ref[0] /= matrixSize;
		ref[1] /= matrixSize;
		ref[2] = voxelSize[2] * images + voxelGap[2] * ( images - 1 );
	}

	// for every slice add acqTime to Multivalue

	auto acqTimeQuery= dest.queryProperty( "acquisitionTime"); 
	if(acqTimeQuery && haveAcqTimeList) 
		*acqTimeQuery=util::PropertyValue(); //reset the selected ordering property to empty

	for ( size_t slice = 0; slice < images; slice++ ) {
		// copy the lines into the corresponding slice in the chunk
		for ( size_t line = 0; line < size[1]; line++ ) {
			const std::array<size_t,4> dpos = {0, line, slice, 0}; //begin of the target line
			const size_t column = slice % matrixSize; //column of the mosaic
			const size_t row = slice / matrixSize; //row of the mosaic
			const std::array<size_t,4> sstart{column *size[0], row *size[1] + line, 0, 0}; //begin of the source line
			const std::array<size_t,4> send{sstart[0] + size[0] - 1, row *size[1] + line, 0, 0}; //end of the source line
			source.copyRange( sstart, send, dest, dpos );
		}

		if(acqTimeQuery && haveAcqTimeList){
			auto newtime=acqTime +  std::chrono::milliseconds((std::chrono::milliseconds::rep)* ( acqTimeIt ) );
			acqTimeQuery->push_back(newtime);
			LOG(Debug,verbose_info) 
				<< "Computed acquisitionTime for slice " << slice << " as " << newtime
				<< "(" << acqTime << "+" <<  std::chrono::milliseconds((std::chrono::milliseconds::rep)* ( acqTimeIt ) );
			++acqTimeIt;
		}
	}

	return dest;
}


std::list< data::Chunk > ImageFormat_Dicom::load( const std::string& filename, const util::istring& dialect, std::shared_ptr< util::ProgressFeedback > progress /*progress*/ )throw( std::runtime_error & )
{
	DcmFileFormat dcfile;
	OFCondition loaded = dcfile.loadFile( filename.c_str() );

	if ( loaded.good() ) {
		std::list< data::Chunk > ret;
		data::Chunk chunk = _internal::DicomChunk::makeChunk( *this, dcfile, dialect );
		//we got a chunk from the file
		sanitise( chunk, dialect );
		const util::slist iType = chunk.getValueAs<util::slist>( util::istring( ImageFormat_Dicom::dicomTagTreeName ) + "/" + "ImageType" );

		if ( std::find( iType.begin(), iType.end(), "MOSAIC" ) != iType.end() ) { // if its a mosaic
			if( dialect == "keepmosaic" ) {
				LOG( Runtime, info ) << "This seems to be an mosaic image, but dialect \"keepmosaic\" was selected";
				ret.push_back( chunk );
			} else {
				ret.push_back( readMosaic( chunk ) );
			}
		} else if( dialect == "forcemosaic" ) 
			ret.push_back( readMosaic( chunk ) );
		else 
			ret.push_back( chunk );

		return ret;
	} else {
		FileFormat::throwGenericError( std::string( "Failed to open file: " ) + loaded.text() );
		return std::list< data::Chunk >();
	}

}

void ImageFormat_Dicom::write( const data::Image &/*image*/, const std::string &/*filename*/, const util::istring &/*dialect*/, std::shared_ptr<util::ProgressFeedback> /*progress*/ ) throw( std::runtime_error & )
{
	throw( std::runtime_error( "writing dicom files is not yet supportet" ) );
}

bool ImageFormat_Dicom::tainted()const {return false;}//internal plugins are not tainted

ImageFormat_Dicom::ImageFormat_Dicom()
{
	//first read external dictionary if available
	if ( dcmDataDict.isDictionaryLoaded() ) {
		DcmDataDictionary &dict = dcmDataDict.wrlock();
		addDicomDict( dict );
		dcmDataDict.unlock(); 
		DJDecoderRegistration::registerCodecs();
	} else {
		// check /usr/share/doc/dcmtk/datadict.txt.gz and/or
		// set DCMDICTPATH or fix DCM_DICT_DEFAULT_PATH in cfunix.h of dcmtk
		LOG( Runtime, warning ) << "No official data dictionary loaded, will only use known attributes";
	}

	// than override known entries
	dictionary[DcmTag( 0x0010, 0x0010 )] = "PatientsName";
	dictionary[DcmTag( 0x0010, 0x0030 )] = "PatientsBirthDate";
	dictionary[DcmTag( 0x0010, 0x0040 )] = "PatientsSex";
	dictionary[DcmTag( 0x0010, 0x1010 )] = "PatientsAge";
	dictionary[DcmTag( 0x0010, 0x1030 )] = "PatientsWeight";

	dictionary[DcmTag( 0x0008, 0x1050 )] = "PerformingPhysiciansName";

	// override some Siemens specific stuff because it is SliceOrientation in the standard and mosaic-size for siemens - we will figure out while sanitizing
	dictionary[DcmTag( 0x0019, 0x100a )] = "SiemensNumberOfImagesInMosaic";
	dictionary[DcmTag( 0x0019, 0x100c )] = "SiemensDiffusionBValue";
	dictionary[DcmTag( 0x0019, 0x100e )] = "SiemensDiffusionGradientOrientation";
	dictionary.erase(DcmTag( 0x0021, 0x1010 )); // dcmtk says its ImageType but it isn't (at least not on Siemens)

	for( unsigned short i = 0x0010; i <= 0x00FF; i++ ) {
		dictionary[DcmTag( 0x0029, i )] = ( std::string( "Private Code for " ) + DcmTag( 0x0029, i << 8 ).toString().c_str() + "-" + DcmTag( 0x0029, ( i << 8 ) + 0xFF ).toString().c_str() ).c_str();
	}
	
	//http://www.healthcare.siemens.com/siemens_hwem-hwem_ssxa_websites-context-root/wcm/idc/groups/public/@global/@services/documents/download/mdaw/mtiy/~edisp/2008b_ct_dicomconformancestatement-00073795.pdf
	for( unsigned short i = 0x0; i <= 0x02FF; i++ ) {
		char buff[7];
		std::snprintf(buff,7,"0x%.4X",i);
		dictionary[DcmTag( 0x6000, i )] = util::PropertyMap::PropPath("DICOM overlay info") / util::PropertyMap::PropPath(buff);
	}
	dictionary[DcmTag( 0x6000, 0x3000 )] = util::PropertyMap::PropPath("DICOM overlay data");
	

	//hack to steal logging from dcmtk and redirect it to our own
	LOG4CPLUS_NAMESPACE::Logger logger = LOG4CPLUS_NAMESPACE::Logger::getRoot();
	// there shall be no logging besides me
	logger.removeAllAppenders();
	logger.addAppender(LOG4CPLUS_NAMESPACE::SharedAppenderPtr(new _internal::DcmtkLogger));
}

util::PropertyMap::PropPath ImageFormat_Dicom::tag2Name( const DcmTagKey &tag )const
{
	std::map< DcmTagKey, util::PropertyMap::PropPath >::const_iterator entry = dictionary.find( tag );
	return ( entry != dictionary.end() ) ? entry->second : util::PropertyMap::PropPath( util::istring( unknownTagName ) + tag.toString().c_str() );
}


}
}

isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Dicom;
}
