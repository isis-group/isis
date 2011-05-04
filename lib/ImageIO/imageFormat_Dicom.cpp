#include "imageFormat_Dicom.hpp"
#include <DataStorage/common.hpp>
#include <CoreUtils/istring.hpp>
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmimage/diregist.h> //for color support
#include <boost/date_time/posix_time/posix_time.hpp>

namespace isis
{
namespace image_io
{
namespace _internal
{
class DicomChunk : public data::Chunk
{
	struct Deleter {
		DcmFileFormat *m_dcfile;
		DicomImage *m_img;
		std::string m_filename;
		Deleter( DcmFileFormat *dcfile, DicomImage *img, std::string filename ): m_dcfile( dcfile ), m_img( img ), m_filename( filename ) {}
		void operator ()( void *at ) {
			LOG_IF( not m_dcfile, Runtime, error )
					<< "Trying to close non existing dicom file";
			LOG_IF( not m_img, Runtime, error )
					<< "Trying to close non existing dicom image";
			LOG( Debug, verbose_info ) << "Closing mapped dicom-file " << util::MSubject( m_filename ) << " (pixeldata was at " << at << ")";
			delete m_img;
			delete m_dcfile;
		}
	};
	template<typename TYPE> DicomChunk(
		TYPE *dat, Deleter del,
		size_t width, size_t height ):
		data::Chunk( dat, del, width, height, 1, 1 ) {
		LOG( Debug, verbose_info )
				<< "Mapping greyscale pixeldata of " << del.m_filename << " at "
				<< dat << " (" << data::ValuePtr<TYPE>::staticName() << ")" ;
	}
	template<typename TYPE>
	static data::Chunk *copyColor( TYPE **source, size_t width, size_t height ) {
		data::Chunk *ret = new data::MemChunk<util::color<TYPE> >( width, height );
		data::ValuePtr<util::color<TYPE> > &dest = ret->asValuePtr<util::color<TYPE> >();
		const size_t pixels = dest.getLength();

		for ( size_t i = 0; i < pixels; i++ ) {
			util::color<TYPE> &dvoxel = dest[i];
			dvoxel.r = source[0][i];
			dvoxel.g = source[1][i];
			dvoxel.b = source[2][i];
		}

		return ret;
	}
public:
	//this uses auto_ptr by intention
	//the ownership of the DcmFileFormat-pointer shall be transfered to this function, because it has to decide if it should be deleted
	static data::Chunk makeChunk( std::string filename, std::auto_ptr<DcmFileFormat> dcfile, const std::string &dialect ) {
		std::auto_ptr<data::Chunk> ret;
		std::auto_ptr<DicomImage> img( new DicomImage( dcfile.get(), EXS_Unknown ) );

		if ( img->getStatus() == EIS_Normal ) {
			const DiPixel *const  pix = img->getInterData();
			const unsigned long width = img->getWidth(), height = img->getHeight();
			const void *const data = pix->getData();
			DcmDataset *dcdata = dcfile->getDataset();

			if ( pix ) {
				if ( img->isMonochrome() ) { //try to load image directly from the raw monochrome dicom-data
					Deleter del( dcfile.get(), img.get(), filename );

					switch ( pix->getRepresentation() ) {
					case EPR_Uint8:
						ret.reset( new DicomChunk( ( uint8_t * ) data, del, width, height ) );
						break;
					case EPR_Sint8:
						ret.reset( new DicomChunk( ( int8_t * )  data, del, width, height ) );
						break;
					case EPR_Uint16:
						ret.reset( new DicomChunk( ( uint16_t * )data, del, width, height ) );
						break;
					case EPR_Sint16:
						ret.reset( new DicomChunk( ( int16_t * ) data, del, width, height ) );
						break;
					case EPR_Uint32:
						ret.reset( new DicomChunk( ( uint32_t * )data, del, width, height ) );
						break;
					case EPR_Sint32:
						ret.reset( new DicomChunk( ( int32_t * ) data, del, width, height ) );
						break;
					default:
						FileFormat::throwGenericError( "Unsupported datatype for monochrome images" ); //@todo tell the user which datatype it is
					}

					if ( ret.get() ) {
						//OK, the source image and file pointer are managed by the chunk, we must release them
						img.release();
						dcfile.release();
						ImageFormat_Dicom::dcmObject2PropMap( dcdata, ret->branch( ImageFormat_Dicom::dicomTagTreeName ), dialect );
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

					if ( ret.get() ) {
						ImageFormat_Dicom::dcmObject2PropMap( dcdata, ret->branch( ImageFormat_Dicom::dicomTagTreeName ), dialect );
					}
				} else {
					FileFormat::throwGenericError( "Unsupported pixel type." );
				}
			} else {
				FileFormat::throwGenericError( "Didn't get any pixel data" );
			}
		} else {
			FileFormat::throwGenericError( std::string( "Failed to open image: " ) + DicomImage::getString( img->getStatus() )  + ")" );
		}

		return *ret;
	}
};
}

using boost::posix_time::ptime;
using boost::gregorian::date;

const char ImageFormat_Dicom::dicomTagTreeName[] = "DICOM";
const char ImageFormat_Dicom::unknownTagName[] = "Unknown Tag";

std::string ImageFormat_Dicom::suffixes()const {return std::string( ".ima .dcm" );}
std::string ImageFormat_Dicom::getName()const {return "Dicom";}
std::string ImageFormat_Dicom::dialects( const std::string &/*filename*/ )const {return "withExtProtocols nomosaic";}



ptime ImageFormat_Dicom::genTimeStamp( const date &date, const ptime &time )
{
	return ptime( date, time.time_of_day() );
}


void ImageFormat_Dicom::sanitise( util::PropertyMap &object, std::string /*dialect*/ )
{
	const util::istring prefix = util::istring( ImageFormat_Dicom::dicomTagTreeName ) + "/";
	/////////////////////////////////////////////////////////////////////////////////
	// Transform known DICOM-Tags into default-isis-properties
	/////////////////////////////////////////////////////////////////////////////////

	// compute sequenceStart and acquisitionTime (have a look at table C.10.8 in the standart)
	if ( hasOrTell( prefix + "SeriesTime", object, warning ) && hasOrTell( prefix + "SeriesDate", object, warning ) ) {
		const ptime sequenceStart = genTimeStamp( object.getPropertyAs<date>( prefix + "SeriesDate" ), object.getPropertyAs<ptime>( prefix + "SeriesTime" ) );

		// compute acquisitionTime
		if ( hasOrTell( prefix + "AcquisitionTime", object, warning ) and hasOrTell( prefix + "AcquisitionDate", object, warning ) ) {
			const ptime acTime = genTimeStamp( object.getPropertyAs<date>( prefix + "AcquisitionDate" ), object.getPropertyAs<ptime>( prefix + "AcquisitionTime" ) );
			const boost::posix_time::time_duration acDist = acTime - sequenceStart;
			const float fAcDist = float( acDist.ticks() ) / acDist.ticks_per_second() * 1000;
			LOG( Debug, verbose_info ) << "Computed acquisitionTime as " << fAcDist;
			object.setPropertyAs( "acquisitionTime", fAcDist );
			object.remove( prefix + "AcquisitionTime" );
			object.remove( prefix + "AcquisitionDate" );
		}

		LOG( Debug, verbose_info ) << "Computed sequenceStart as " << sequenceStart;
		object.setPropertyAs( "sequenceStart", sequenceStart );
		object.remove( prefix + "SeriesTime" );
		object.remove( prefix + "SeriesDate" );
	}

	transformOrTell<uint16_t>  ( prefix + "SeriesNumber",     "sequenceNumber",     object, warning );
	transformOrTell<uint16_t>  ( prefix + "PatientsAge",     "subjectAge",     object, warning );
	transformOrTell<std::string>( prefix + "SeriesDescription", "sequenceDescription", object, warning );
	transformOrTell<std::string>( prefix + "PatientsName",     "subjectName",        object, warning );
	transformOrTell<date>       ( prefix + "PatientsBirthDate", "subjectBirth",       object, info );
	transformOrTell<uint16_t>  ( prefix + "PatientsWeight",   "subjectWeigth",      object, warning );
	// compute voxelSize and gap
	{
		util::fvector4 voxelSize( invalid_float, invalid_float, invalid_float, 0 );

		if ( hasOrTell( prefix + "PixelSpacing", object, warning ) ) {
			voxelSize = object.getPropertyAs<util::fvector4>( prefix + "PixelSpacing" );
			object.remove( prefix + "PixelSpacing" );
			std::swap( voxelSize[0], voxelSize[1] ); // the values are row-spacing (size in column dir) /column spacing (size in row dir)
		}

		if ( hasOrTell( prefix + "SliceThickness", object, warning ) ) {
			voxelSize[2] = object.getPropertyAs<float>( prefix + "SliceThickness" );
			object.remove( prefix + "SliceThickness" );
		}

		object.setPropertyAs( "voxelSize", voxelSize );
		transformOrTell<uint16_t>( prefix + "RepetitionTime", "repetitionTime", object, warning );
		transformOrTell<float>( prefix + "EchoTime", "echoTime", object, warning );
		transformOrTell<std::string>( prefix + "TransmitCoilName", "transmitCoil", object, warning );
		transformOrTell<int16_t>( prefix + "FlipAngle", "flipAngle", object, warning );

		if ( hasOrTell( prefix + "SpacingBetweenSlices", object, info ) ) {
			if ( voxelSize[2] != invalid_float ) {
				object.setPropertyAs( "voxelGap", util::fvector4( 0, 0, object.getPropertyAs<float>( prefix + "SpacingBetweenSlices" ) - voxelSize[2] ) );
				object.remove( prefix + "SpacingBetweenSlices" );
			} else
				LOG( Runtime, warning )
						<< "Cannot compute the voxel gap from the slice spacing ("
						<< object.propertyValue( prefix + "SpacingBetweenSlices" )
						<< "), because the slice thickness is not known";
		}
	}
	transformOrTell<std::string>   ( prefix + "PerformingPhysiciansName", "performingPhysician", object, info );
	transformOrTell<uint16_t>     ( prefix + "NumberOfAverages",        "numberOfAverages",   object, warning );

	if ( hasOrTell( prefix + "ImageOrientationPatient", object, info ) ) {
		util::dlist buff = object.getPropertyAs<util::dlist>( prefix + "ImageOrientationPatient" );

		if ( buff.size() == 6 ) {
			util::fvector4 row, column;
			util::dlist::iterator b = buff.begin();

			for ( int i = 0; i < 3; i++ )row[i] = *b++;

			for ( int i = 0; i < 3; i++ )column[i] = *b++;

			object.setPropertyAs( "rowVec" , row );
			object.setPropertyAs( "columnVec", column );
			object.remove( prefix + "ImageOrientationPatient" );
		} else {
			LOG( Runtime, error ) << "Could not extract row- and columnVector from " << object.propertyValue( prefix + "ImageOrientationPatient" );
		}

		if( object.hasProperty( prefix + "CSAImageHeaderInfo/SliceNormalVector" ) && !object.hasProperty( "sliceVec" ) ) {
			LOG( Debug, info ) << "Extracting sliceVec from CSAImageHeaderInfo/SliceNormalVector " << object.propertyValue( prefix + "CSAImageHeaderInfo/SliceNormalVector" );
			util::dlist list = object.getPropertyAs<util::dlist >( prefix + "CSAImageHeaderInfo/SliceNormalVector" );
			util::fvector4 vec;
			vec.copyFrom( list.begin(), list.end() );
			object.setPropertyAs( "sliceVec", vec );
			object.remove( prefix + "CSAImageHeaderInfo/SliceNormalVector" );
		}
	} else {
		LOG( Runtime, warning ) << "Making up row and column vector, because the image lacks this information";
		object.setPropertyAs( "rowVec" , util::fvector4( 1, 0, 0 ) );
		object.setPropertyAs( "columnVec", util::fvector4( 0, 1, 0 ) );
	}

	if ( hasOrTell( prefix + "ImagePositionPatient", object, info ) ) {
		object.setPropertyAs( "indexOrigin", object.getPropertyAs<util::fvector4>( prefix + "ImagePositionPatient" ) );
	} else {
		object.setPropertyAs( "indexOrigin", util::fvector4() );
		LOG( Runtime, warning ) << "Making up indexOrigin, because the image lacks this information";
	}

	transformOrTell<uint32_t>( prefix + "InstanceNumber", "acquisitionNumber", object, error );

	if ( hasOrTell( prefix + "PatientsSex", object, warning ) ) {
		util::Selection isisGender( "male,female,other" );

		switch ( object.getPropertyAs<std::string>( prefix + "PatientsSex" )[0] ) {
		case 'M':
			isisGender.set( "male" );
			break;
		case 'F':
			isisGender.set( "female" );
			break;
		case 'O':
			isisGender.set( "other" );
			break;
		default:
			LOG( Runtime, error ) << "Dicom gender code " << util::MSubject( object.propertyValue( prefix + "ImageOrientationPatient" ) ) <<  " not known";
		}

		object.propertyValue( "subjectGender" ) = isisGender;
		object.remove( prefix + "PatientsSex" );
	}

	transformOrTell<uint32_t>( prefix + "CSAImageHeaderInfo/UsedChannelMask", "coilChannelMask", object, info );
	////////////////////////////////////////////////////////////////
	// interpret DWI data
	////////////////////////////////////////////////////////////////
	int32_t bValue;
	bool foundDiff = true;

	// find the B-Value
	if ( object.hasProperty( prefix + "DiffusionBValue" ) ) { //in case someone actually used the right Tag
		bValue = object.getPropertyAs<int32_t>( prefix + "DiffusionBValue" );
		object.remove( prefix + "DiffusionBValue" );
	} else if ( object.hasProperty( prefix + "Unknown Tag(0019,100c)" ) ) { //fallback for siemens
		bValue = object.getPropertyAs<int32_t>( prefix + "Unknown Tag(0019,100c)" );
		object.remove( prefix + "Unknown Tag(0019,100c)" );
	} else foundDiff = false;

	// If we do have DWI here, create a property diffusionGradient (which defaults to 0,0,0,0)
	if( foundDiff ) {
		util::fvector4 &diff = object.setPropertyAs( "diffusionGradient", util::fvector4() )->castTo<util::fvector4>();

		if( bValue ) { // if bValue is not zero multiply the diffusionGradient by it
			if( object.hasProperty( prefix + "DiffusionGradientOrientation" ) ) {
				diff = object.getPropertyAs<util::fvector4>( prefix + "DiffusionGradientOrientation" ) * bValue;
				object.remove( prefix + "DiffusionGradientOrientation" );
			} else if( object.hasProperty( prefix + "Unknown Tag(0019,100e)" ) ) {
				diff = object.getPropertyAs<util::fvector4>( prefix + "Unknown Tag(0019,100e)" ) * bValue;
				object.remove( prefix + "Unknown Tag(0019,100e)" );
			} else {
				LOG( Runtime, error ) << "Found no diffusion direction for DiffusionBValue " << util::MSubject( bValue );
			}
		}
	}


	//@todo fallback for GE/Philips
	////////////////////////////////////////////////////////////////
	// Do some sanity checks on redundant tags
	////////////////////////////////////////////////////////////////
	if ( object.hasProperty( prefix + "Unknown Tag(0019,1015)" ) ) {
		const util::fvector4 org = object.getPropertyAs<util::fvector4>( "indexOrigin" );
		const util::fvector4 comp = object.getPropertyAs<util::fvector4>( prefix + "Unknown Tag(0019,1015)" );

		if ( comp.fuzzyEqual( org ) )
			object.remove( prefix + "Unknown Tag(0019,1015)" );
		else
			LOG( Debug, warning )
					<< prefix + "Unknown Tag(0019,1015):" << object.propertyValue( prefix + "Unknown Tag(0019,1015)" )
					<< " differs from indexOrigin:" << object.propertyValue( "indexOrigin" ) << ", won't remove it";
	}

	if(
		object.hasProperty( prefix + "CSAImageHeaderInfo/MosaicRefAcqTimes" ) &&
		object.hasProperty( prefix + "Unknown Tag(0019,1029)" ) &&
		object.propertyValue( prefix + "Unknown Tag(0019,1029)" ) == object.propertyValue( prefix + "CSAImageHeaderInfo/MosaicRefAcqTimes" )
	) {
		object.remove( prefix + "Unknown Tag(0019,1029)" );
	}

	if ( object.hasProperty( prefix + "Unknown Tag(0051,100c)" ) ) { //@todo siemens only ?
		std::string fov = object.getPropertyAs<std::string>( prefix + "Unknown Tag(0051,100c)" );
		float row, column;

		if ( std::sscanf( fov.c_str(), "FoV %f*%f", &column, &row ) == 2 ) {
			object.setPropertyAs( "fov", util::fvector4( row, column, invalid_float, invalid_float ) );
		}
	}
}

int ImageFormat_Dicom::readMosaic( isis::data::Chunk source, std::list< isis::data::Chunk >& dest )
{
	// prepare some needed parameters
	const util::istring prefix = util::istring( ImageFormat_Dicom::dicomTagTreeName ) + "/";
	util::slist iType = source.getPropertyAs<util::slist>( prefix + "ImageType" );
	std::replace( iType.begin(), iType.end(), std::string( "MOSAIC" ), std::string( "WAS_MOSAIC" ) );
	util::istring NumberOfImagesInMosaicProp;

	const unsigned short oldSize = dest.size();

	if ( source.hasProperty( prefix + "Unknown Tag(0019,100a)" ) ) {
		NumberOfImagesInMosaicProp = prefix + "Unknown Tag(0019,100a)";
	} else if ( source.hasProperty( prefix + "CSAImageHeaderInfo/NumberOfImagesInMosaic" ) ) {
		NumberOfImagesInMosaicProp = prefix + "CSAImageHeaderInfo/NumberOfImagesInMosaic";
	} else {
		FileFormat::throwGenericError( "Could not determine the number of images in the mosaic" );
	}

	// All is fine, lets start
	uint16_t images = source.getPropertyAs<uint16_t>( NumberOfImagesInMosaicProp );
	util::FixedVector<size_t, 4> size = source.getSizeAsVector();
	const uint16_t matrixSize = std::ceil( std::sqrt( images ) );
	size[0] /= matrixSize;
	size[1] /= matrixSize;
	assert( size[3] == 1 );
	LOG( Debug, info ) << "Decomposing a " << source.getSizeAsString() << " mosaic-image into " << images << " " << size << " slices";
	// fix the properties of the source (we 'll need them later)
	util::fvector4 voxelGap;

	if ( source.hasProperty( "voxelGap" ) )
		voxelGap = source.getPropertyAs<util::fvector4>( "voxelGap" );

	const util::fvector4 voxelSize = source.getPropertyAs<util::fvector4>( "voxelSize" );
	const util::fvector4 &rowVec = source.getPropertyAs<util::fvector4>( "rowVec" );
	const util::fvector4 &columnVec = source.getPropertyAs<util::fvector4>( "columnVec" );
	//remove the additional mosaic offset
	//eg. if there is a 10x10 Mosaic, substract the half size of 9 Images from the offset
	const util::fvector4 fovCorr = ( voxelSize + voxelGap ) * size * ( matrixSize - 1 ) / 2; // @todo this will not include the voxelGap between the slices
	util::fvector4 &origin = source.propertyValue( "indexOrigin" )->castTo<util::fvector4>();
	origin = origin + ( rowVec * fovCorr[0] ) + ( columnVec * fovCorr[1] );
	source.remove( NumberOfImagesInMosaicProp ); // we dont need that anymore
	source.setPropertyAs( prefix + "ImageType", iType );

	// if we dont have a sliceVec - compute it
	if( !source.hasProperty( "sliceVec" ) ) {
		const util::fvector4 crossVec = util::fvector4(
											rowVec[1] * columnVec[2] - rowVec[2] * columnVec[1],
											rowVec[2] * columnVec[0] - rowVec[0] * columnVec[2],
											rowVec[0] * columnVec[1] - rowVec[1] * columnVec[0]
										);
		source.setPropertyAs( "sliceVec", crossVec );
		LOG( Debug, info ) << "Computed sliceVec as " << source.propertyValue( "sliceVec" );
	}

	const util::fvector4 sliceVec = source.getPropertyAs<util::fvector4>( "sliceVec" ).norm() * ( voxelSize[2] + voxelGap[2] );

	//store and remove acquisitionTime
	std::list<double> acqTimeList;

	std::list<double>::const_iterator acqTimeIt;

	bool haveAcqTimeList = source.hasProperty( prefix + "CSAImageHeaderInfo/MosaicRefAcqTimes" );

	float acqTime = 0;

	if( haveAcqTimeList ) {
		acqTimeList = source.getPropertyAs<std::list<double> >( prefix + "CSAImageHeaderInfo/MosaicRefAcqTimes" );
		source.remove( prefix + "CSAImageHeaderInfo/MosaicRefAcqTimes" );
		acqTimeIt = acqTimeList.begin();
		LOG( Debug, info ) << "The acquisition time offsets of the slices in the mosaic where " << acqTimeList;
	}

	if( source.hasProperty( "acquisitionTime" ) ) {
		acqTime = source.propertyValue( "acquisitionTime" )->castTo<float>();
	}

	// for every slice
	for ( size_t slice = 0; slice < images; slice++ ) {
		dest.push_back( source.cloneToNew( size[0], size[1] ) );//create a new chunk at the end of the output
		data::Chunk &working = dest.back(); // use this as working slice

		// copy the lines into the corresponding slice-chunk
		for ( size_t line = 0; line < size[1]; line++ ) {
			const size_t dpos[] = {0, line, 0, 0}; //begin of the target line
			const size_t column = slice % matrixSize; //column of the mosaic
			const size_t row = slice / matrixSize; //row of the mosaic
			const size_t sstart[] = {column *size[0], row *size[1] + line, 0, 0}; //begin of the source line
			const size_t send[] = {sstart[0] + size[0] - 1, row *size[1] + line, 0, 0}; //end of the source line
			source.copyRange( sstart, send, working, dpos );
		}

		// and "fix" its properties
		static_cast<util::PropertyMap &>( working ) = static_cast<const util::PropertyMap &>( source ); //copy _only_ the Properties of source
		// update origin
		util::fvector4 &origin = working.propertyValue( "indexOrigin" )->castTo<util::fvector4>();
		origin = origin + ( sliceVec * slice );

		// update fov
		if ( working.hasProperty( "fov" ) ) {
			util::fvector4 &ref = working.propertyValue( "fov" )->castTo<util::fvector4>();
			ref[0] /= matrixSize;
			ref[1] /= matrixSize;
		}

		// fix/set acquisitionNumber and acquisitionTime
		working.propertyValue( "acquisitionNumber" )->castTo<uint32_t>() += slice;

		if( haveAcqTimeList ) {
			working.setPropertyAs<float>( "acquisitionTime", acqTime +  * ( acqTimeIt++ ) );
		}

		LOG( Debug, verbose_info )
				<< "New slice " << slice << " at " << working.propertyValue( "indexOrigin" ).toString( false )
				<< " with acquisitionNumber " << working.propertyValue( "acquisitionNumber" ).toString( false )
				<< ( haveAcqTimeList ? std::string( " and acquisitionTime " ) + working.propertyValue( "acquisitionTime" ).toString( false ) : "" );
	}

	return dest.size() - oldSize;
}


int ImageFormat_Dicom::load( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect )throw( std::runtime_error & )
{

	std::auto_ptr<DcmFileFormat> dcfile( new DcmFileFormat );
	OFCondition loaded = dcfile->loadFile( filename.c_str() );

	if ( loaded.good() ) {
		data::Chunk chunk = _internal::DicomChunk::makeChunk( filename, dcfile, dialect );
		//we got a chunk from the file
		sanitise( chunk, "" );
		chunk.setPropertyAs( "source", filename );
		const util::slist iType = chunk.getPropertyAs<util::slist>( util::istring( ImageFormat_Dicom::dicomTagTreeName ) + "/" + "ImageType" );

		if ( std::find( iType.begin(), iType.end(), "MOSAIC" ) != iType.end() ) { // if its a mosaic
			if( dialect == "nomosaic" ) {
				LOG( Runtime, info ) << "This seems to be an mosaic image, but dialect \"nomosaic\" was selected";
				chunks.push_back( chunk );
				return 1;
			} else {
				LOG( Runtime, verbose_info ) << "This seems to be an mosaic image, will decompose it";
				return readMosaic( chunk, chunks );
			}
		} else {
			chunks.push_back( chunk );
			return 1;
		}
	} else {
		FileFormat::throwGenericError( std::string( "Failed to open file: " ) + loaded.text() );
	}

	return 0;
}

void ImageFormat_Dicom::write( const data::Image &/*image*/, const std::string &/*filename*/, const std::string &/*dialect*/ ) throw( std::runtime_error & )
{
	throw( std::runtime_error( "writing dicom files is not yet supportet" ) );
}

bool ImageFormat_Dicom::tainted()const {return false;}//internal plugins are not tainted
}
}

isis::image_io::FileFormat *factory()
{
	if ( not dcmDataDict.isDictionaryLoaded() ) {
		LOG( isis::image_io::Runtime, isis::error )
				<< "No data dictionary loaded, check environment variable DCMDICTPATH"; //set DCMDICTPATH or fix DCM_DICT_DEFAULT_PATH in cfunix.h of dcmtk
		return NULL;
	}

	return new isis::image_io::ImageFormat_Dicom();
}
