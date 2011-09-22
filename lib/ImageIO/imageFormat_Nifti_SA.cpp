#include <DataStorage/io_interface.h>
#include <DataStorage/fileptr.hpp>
#include <CoreUtils/matrix.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#define NIFTI_TYPE_UINT8           2
#define NIFTI_TYPE_UINT16        512
#define NIFTI_TYPE_UINT32        768
#define NIFTI_TYPE_UINT64       1280

#define NIFTI_TYPE_INT8          256
#define NIFTI_TYPE_INT16           4
#define NIFTI_TYPE_INT32           8
#define NIFTI_TYPE_INT64        1024

#define NIFTI_TYPE_FLOAT32        16
#define NIFTI_TYPE_FLOAT64        64
#define NIFTI_TYPE_FLOAT128     1536

#define NIFTI_TYPE_COMPLEX64      32
#define NIFTI_TYPE_COMPLEX128   1792
#define NIFTI_TYPE_COMPLEX256   2048

#define NIFTI_TYPE_RGB24         128

#define NIFTI_SLICE_SEQ_INC  1
#define NIFTI_SLICE_SEQ_DEC  2
#define NIFTI_SLICE_ALT_INC  3
#define NIFTI_SLICE_ALT_DEC  4


#define NIFTI_UNITS_UNKNOWN 0
#define NIFTI_UNITS_METER   1
#define NIFTI_UNITS_MM      2
#define NIFTI_UNITS_MICRON  3
#define NIFTI_UNITS_SEC     8
#define NIFTI_UNITS_MSEC   16
#define NIFTI_UNITS_USEC   24
#define NIFTI_UNITS_HZ     32
#define NIFTI_UNITS_PPM    40
#define NIFTI_UNITS_RADS   48
							   
namespace isis
{
namespace image_io
{

namespace _internal{

//define the nifti-header (brazenly stolen from nifti1.h)
struct nifti_1_header {
	                     /* NIFTI-1 usage         */    /* ANALYZE 7.5 field(s) */
                                                        /*--- was header_key substruct ---*/
	int   sizeof_hdr;    /*!< MUST be 348           */  /* int sizeof_hdr;      */
	char  data_type[10]; /*!< ++UNUSED++            */  /* char data_type[10];  */
	char  db_name[18];   /*!< ++UNUSED++            */  /* char db_name[18];    */
	int   extents;       /*!< ++UNUSED++            */  /* int extents;         */
	short session_error; /*!< ++UNUSED++            */  /* short session_error; */
	char  regular;       /*!< ++UNUSED++            */  /* char regular;        */
	char  dim_info;      /*!< MRI slice ordering.   */  /* char hkey_un0;       */

										/*--- was image_dimension substruct ---*/
	short dim[8];        /*!< Data array dimensions.*/  /* short dim[8];        */
	float intent_p1 ;    /*!< 1st intent parameter. */  /* short unused8;       */
														/* short unused9;       */
	float intent_p2 ;    /*!< 2nd intent parameter. */  /* short unused10;      */
														/* short unused11;      */
	float intent_p3 ;    /*!< 3rd intent parameter. */  /* short unused12;      */
														/* short unused13;      */
	short intent_code ;  /*!< NIFTI_INTENT_* code.  */  /* short unused14;      */
	short datatype;      /*!< Defines data type!    */  /* short datatype;      */
	short bitpix;        /*!< Number bits/voxel.    */  /* short bitpix;        */
	short slice_start;   /*!< First slice index.    */  /* short dim_un0;       */
	float pixdim[8];     /*!< Grid spacings.        */  /* float pixdim[8];     */
	float vox_offset;    /*!< Offset into .nii file */  /* float vox_offset;    */
	float scl_slope ;    /*!< Data scaling: slope.  */  /* float funused1;      */
	float scl_inter ;    /*!< Data scaling: offset. */  /* float funused2;      */
	short slice_end;     /*!< Last slice index.     */  /* float funused3;      */
	char  slice_code ;   /*!< Slice timing order.   */
	char  xyzt_units ;   /*!< Units of pixdim[1..4] */
	float cal_max;       /*!< Max display intensity */  /* float cal_max;       */
	float cal_min;       /*!< Min display intensity */  /* float cal_min;       */
	float slice_duration;/*!< Time for 1 slice.     */  /* float compressed;    */
	float toffset;       /*!< Time axis shift.      */  /* float verified;      */
	int   glmax;         /*!< ++UNUSED++            */  /* int glmax;           */
	int   glmin;         /*!< ++UNUSED++            */  /* int glmin;           */

											/*--- was data_history substruct ---*/
	char  descrip[80];   /*!< any text you like.    */  /* char descrip[80];    */
	char  aux_file[24];  /*!< auxiliary filename.   */  /* char aux_file[24];   */

	short qform_code ;   /*!< NIFTI_XFORM_* code.   */  /*-- all ANALYZE 7.5 ---*/
	short sform_code ;   /*!< NIFTI_XFORM_* code.   */  /*   fields below here  */
														/*   are replaced       */
	float quatern_b ;    /*!< Quaternion b param.   */
	float quatern_c ;    /*!< Quaternion c param.   */
	float quatern_d ;    /*!< Quaternion d param.   */
	float qoffset_x ;    /*!< Quaternion x shift.   */
	float qoffset_y ;    /*!< Quaternion y shift.   */
	float qoffset_z ;    /*!< Quaternion z shift.   */

	float srow_x[4] ;    /*!< 1st row affine transform.   */
	float srow_y[4] ;    /*!< 2nd row affine transform.   */
	float srow_z[4] ;    /*!< 3rd row affine transform.   */

	char intent_name[16];/*!< 'name' or meaning of data.  */

	char magic[4] ;      /*!< MUST be "ni1\0" or "n+1\0". */

} ;                   /**** 348 bytes total ****/

}
	
class ImageFormat_NiftiSa: public FileFormat
{
	static const util::Matrix4x4<short> nifti2isis;
public:
	ImageFormat_NiftiSa(){
		nii2isis[NIFTI_TYPE_INT8 ]=data::ValuePtr< int8_t>::staticID;
		nii2isis[NIFTI_TYPE_INT16]=data::ValuePtr<int16_t>::staticID;
		nii2isis[NIFTI_TYPE_INT32]=data::ValuePtr<int32_t>::staticID;
		nii2isis[NIFTI_TYPE_INT64]=data::ValuePtr<int64_t>::staticID;

		nii2isis[NIFTI_TYPE_UINT8 ]=data::ValuePtr< uint8_t>::staticID;
		nii2isis[NIFTI_TYPE_UINT16]=data::ValuePtr<uint16_t>::staticID;
		nii2isis[NIFTI_TYPE_UINT32]=data::ValuePtr<uint32_t>::staticID;
		nii2isis[NIFTI_TYPE_UINT64]=data::ValuePtr<uint64_t>::staticID;

		nii2isis[NIFTI_TYPE_FLOAT32]=data::ValuePtr<float>::staticID;
		nii2isis[NIFTI_TYPE_FLOAT64]=data::ValuePtr<double>::staticID;

/*		nii2isis[NIFTI_TYPE_COMPLEX64 ]=data::ValuePtr<std::complex< float > >::staticID;
		nii2isis[NIFTI_TYPE_COMPLEX128]=data::ValuePtr<std::complex< double > >::staticID;*/
	}
protected:
	std::string suffixes()const {
		return std::string( ".nii" );
	}
	std::map<short,unsigned short> nii2isis;
	static bool parseDescrip(util::PropertyMap &props, const char desc[]){
		//check description for tr, te and fa and date which is written by spm8
		boost::regex descriptionRegex(
			".*TR=([[:digit:]]{1,})ms.*TE=([[:digit:]]{1,})ms.*FA=([[:digit:]]{1,})deg\\ *([[:digit:]]{1,2}).([[:word:]]{3}).([[:digit:]]{4})\\ *([[:digit:]]{1,2}):([[:digit:]]{1,2}):([[:digit:]]{1,2}).*"
		);
		boost::cmatch results;

		if ( boost::regex_match( desc, results,  descriptionRegex ) ) {
			props.propertyValue("repetitionTime")=util::Value<uint16_t>( results.str( 1 ) );
			props.propertyValue("echoTime")=util::Value<uint16_t>( results.str( 2 ) );
			props.propertyValue("flipAngle")=util::Value<uint16_t>( results.str( 2 ) );

			const util::Value<int> day=results.str( 4 ), month=results.str( 5 ), year=results.str( 6 );
			const util::Value<uint8_t> hours=boost::lexical_cast<uint8_t>( results.str( 7 ) ), minutes=boost::lexical_cast<uint8_t>( results.str( 8 ) ), seconds=boost::lexical_cast<uint8_t>( results.str( 9 ) );

			boost::posix_time::ptime sequenceStart=boost::posix_time::ptime(
				boost::gregorian::date((int)year,(int)month,(int)day),
				boost::posix_time::time_duration( hours, minutes, seconds )
			);
			props.setPropertyAs<boost::posix_time::ptime>( "sequenceStart", sequenceStart );

			LOG( ImageIoLog, info )
				<< "Using Tr=" << props.propertyValue("repetitionTime") << ", Te=" << props.propertyValue("echoTime") 
				<< ", flipAngle=" << props.propertyValue("flipAngle") << " and sequenceStart=" << props.propertyValue("sequenceStart")
				<< " from SPM8 description.";

			return true;
		} else
			return false;
	}
	static void header2PropMap(const _internal::nifti_1_header *head,data::Chunk &props){
		unsigned short dims=head->dim[0];
		double time_fac=1;
		double size_fac=1;

		switch(head->xyzt_units & 0x07){
			case NIFTI_UNITS_METER:size_fac=1e3;break;
			case NIFTI_UNITS_MICRON:size_fac=1e-3;break;
		}

		switch(head->xyzt_units & 0x38){
			case NIFTI_UNITS_SEC:time_fac=1e3;break;
			case NIFTI_UNITS_USEC:time_fac=1e-3;break;
		}


		props.setPropertyAs<uint16_t>("sequenceNumber",0);
		props.setPropertyAs<std::string>("sequenceDescription",head->descrip);

		const util::Selection formCode("SCANNER_ANAT,ALIGNED_ANAT,TALAIRACH,MNI_152");

		if( head->sform_code ) { // get srow if sform_code>0
			props.setPropertyAs( "nifti/sform_code", formCode )->castTo<util::Selection>().set(head->sform_code);
			props.setPropertyAs( "nifti/srow_x", util::fvector4() )->castTo<util::fvector4>().copyFrom(head->srow_x,head->srow_x+4);;
			props.setPropertyAs( "nifti/srow_y", util::fvector4() )->castTo<util::fvector4>().copyFrom(head->srow_y,head->srow_y+4);;
			props.setPropertyAs( "nifti/srow_z", util::fvector4() )->castTo<util::fvector4>().copyFrom(head->srow_z,head->srow_z+4);;
		}

		if( head->qform_code ) { // get the quaternion if qform_code>0
			props.setPropertyAs( "nifti/qform_code", formCode )->castTo<util::Selection>().set(head->qform_code);
			props.setPropertyAs( "nifti/quatern_b", head->quatern_b);
			props.setPropertyAs( "nifti/quatern_c", head->quatern_c);
			props.setPropertyAs( "nifti/quatern_d", head->quatern_d);
			props.setPropertyAs( "nifti/qoffset", util::fvector4( head->qoffset_x, head->qoffset_y, head->qoffset_z, 0 ) );
			props.setPropertyAs( "nifti/qfac", (head->dim[0]==-1) ?:1 );

			// voxel size
			util::fvector4 v_size;v_size.copyFrom(head->pixdim+1,head->pixdim+std::min<unsigned short>(dims,3)+1);
			props.setPropertyAs<util::fvector4>("nifti/pixdim",v_size*size_fac);
		}

		if(head->sform_code){ // if sform_code is set, use that regardless of qform
			useSForm(props);
		} else if(head->qform_code){ // if qform_code is set, but no sform use that (thats the "normal" case)
			useQForm(props);
		} else {
			LOG( Runtime, warning ) << "Neither sform_code nor qform_code are set, using identity matrix for geometry";
			props.setPropertyAs<util::fvector4>( "rowVec",    nifti2isis.getRow(0) ); // we use the transformation from nifti to isis as unity
			props.setPropertyAs<util::fvector4>( "columnVec", nifti2isis.getRow(1) ); // because the image will very likely be in nifti space
			props.setPropertyAs<util::fvector4>( "sliceVec",  nifti2isis.getRow(2) );
			props.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4(head->pixdim[1],head->pixdim[2],head->pixdim[3]) );
		}
		// set space unit factors
		props.propertyValue( "voxelSize")->castTo<util::fvector4>()*=size_fac;
		props.propertyValue( "indexOrigin")->castTo<util::fvector4>()*=size_fac;

		// set slice ordering
		if(dims==3){
			const int interl[]={0,1,-1};
			switch(head->slice_code){
				case 0:
				case NIFTI_SLICE_SEQ_INC:
					for(short i=0;i<head->dim[3];i++){
						props.propertyValueAt("acquisitionNumber",i)=i;
						props.propertyValueAt("acquisitionTime",  i)=i*head->slice_duration*time_fac;
					}
					break;
				case NIFTI_SLICE_SEQ_DEC:
					for(short i=0;i<head->dim[3];i++){
						props.propertyValueAt("acquisitionNumber",head->dim[3]-i-1)=i;
						props.propertyValueAt("acquisitionTime",  head->dim[3]-i-1)=i*head->slice_duration*time_fac;
					}
					break;
				case NIFTI_SLICE_ALT_INC:
					for(short i=0;i<head->dim[3];i++){
						props.propertyValueAt("acquisitionNumber",i)=i+interl[i%3];
						props.propertyValueAt("acquisitionTime",  i)=(i+interl[i%3])*head->slice_duration*time_fac;
					}
					break;
				case NIFTI_SLICE_ALT_DEC:
					for(short i=0;i<head->dim[3];i++){
						props.propertyValueAt("acquisitionNumber",head->dim[3]-i-1)=i+interl[i%3];
						props.propertyValueAt("acquisitionTime",  head->dim[3]-i-1)=(i+interl[i%3])*head->slice_duration*time_fac;
					}
					break;

				default:LOG(Runtime,error) << "Unknown slice code " << util::MSubject(head->slice_code);break;
			}
		} else {
			LOG_IF(head->slice_code,Runtime,warning) << "Sorry slice_code!=0 is currently only supportet for 3D-images, ignoring it.";
		}

		// Tr
		if(dims>4)
			props.setPropertyAs<uint16_t>("repetitionTime",head->pixdim[4]*time_fac);

		// sequenceDescription
		if(!parseDescrip(props, head->descrip)) // if descrip dos not hold Te,Tr and stuff (SPM dialect)
			props.setPropertyAs<std::string>("sequenceDescription",head->descrip);// use it the usual way

		// TODO: at the moment scaling not supported due to data type changes
		if ( head->scl_slope != 1 || head->scl_inter>0 ) {
			//          throwGenericError( std::string( "Scaling is not supported at the moment. Scale Factor: " ) + util::Value<float>( scale ).toString() );
			LOG( Runtime, error ) << "Scaling is not supported at the moment.";
		}



	}

public:
	std::string getName()const {return "Nifti standalone";}

	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		data::FilePtr mfile(filename);

		//get the header - we use it directly from the file
		_internal::nifti_1_header *header= reinterpret_cast<_internal::nifti_1_header*>(&mfile[0]);

		if(header->intent_code!=0){
			throwGenericError(std::string("only intent_code==0 is supportet"));
		}

		//set up the size - copy dim[0] values from dim[1]..dim[dim[0]]
		util::FixedVector<size_t,4> size;
		size.fill(1);

		size.copyFrom(header->dim+1,header->dim+1+header->dim[0]);

		data::ValuePtrReference data=mfile.atByID(nii2isis[header->datatype],header->vox_offset);
		LOG(Runtime,info) << "Mapping nifti image as " << data->getTypeName() << " of length " << data->getLength();
		LOG_IF((size_t)header->bitpix!=data->bytesPerElem()*8,Runtime,warning)
			<< "nifti field bitpix does not fit the bytesize of the given datatype ("
			<< data->getTypeName() << "/" << header->bitpix <<  ")";

		chunks.push_back(data::Chunk(data,size[0],size[1],size[2],size[3]));
		header2PropMap(header,chunks.back());

		return 1;
	}

	void write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
	}
	bool tainted()const {return false;}//internal plugins are not tainted
private:
	static void useSForm(util::PropertyMap &props){
		// srow_? is the linear map from image space to nifti space (not isis space)
		// [x] [ nifti/srow_x ]   [i]
		// [y]=[ nifti/srow_y ] * [j]
		// [z] [ nifti/srow_z ]   [k]

		LOG(Debug,info) << "Using sform (" << props.propertyValue( "nifti/sform_code").toString() << ") " << util::MSubject(
			props.propertyValue("nifti/srow_x").toString()+"-"+
			props.propertyValue("nifti/srow_y").toString()+"-"+
			props.propertyValue("nifti/srow_z").toString()
		) << " to calc orientation";


		// transform from image space to nifti space
		const util::Matrix4x4<float> image2nifti(
			props.getPropertyAs<util::fvector4>("nifti/srow_x"),
			props.getPropertyAs<util::fvector4>("nifti/srow_y"),
			props.getPropertyAs<util::fvector4>("nifti/srow_z")
		);
		util::Matrix4x4<float> image2isis=nifti2isis.dot(image2nifti); // add transform to isis-space

		//get position of image-voxel 0,0,0,0 in isis space
		const util::fvector4 origin=image2isis.dot(util::fvector4(0,0,0,1));
		props.setPropertyAs<util::fvector4>("indexOrigin",origin)->castTo<util::fvector4>()[data::timeDim]=0; // timedim is 1 from the matrix calc
		LOG(Debug,info) << "Computed indexOrigin=" << props.getPropertyAs<util::fvector4>("indexOrigin") << " from sform";

		//remove offset from image2isis
		image2isis=util::Matrix4x4<float>(
			util::fvector4(1,0,0,-origin[0]),
			util::fvector4(0,1,0,-origin[1]),
			util::fvector4(0,0,1,-origin[2])
		).dot(image2isis);
		
		const util::fvector4 voxelSize( // get voxel sizes by transforming othogonal vectors of one voxel from image to isis
			image2isis.dot(util::fvector4(1,0,0)).len(),
			image2isis.dot(util::fvector4(0,1,0)).len(),
			image2isis.dot(util::fvector4(0,0,1)).len()
		);
		props.setPropertyAs<util::fvector4>("voxelSize",voxelSize)->castTo<util::fvector4>()[data::timeDim]=0; // timedim is 1 from the matrix calc
		LOG(Debug,info)	<< "Computed voxelSize=" << props.getPropertyAs<util::fvector4>("voxelSize") << " from sform";


		//remove scaling from image2isis
		image2isis=image2isis.dot(util::Matrix4x4<float>(
			util::fvector4(1/voxelSize[0],0,0),
			util::fvector4(0,1/voxelSize[1],0),
			util::fvector4(0,0,1/voxelSize[2])
		));

		props.setPropertyAs<util::fvector4>("rowVec",image2isis.transpose().getRow(0));
		props.setPropertyAs<util::fvector4>("columnVec",image2isis.transpose().getRow(1));
		props.setPropertyAs<util::fvector4>("sliceVec",image2isis.transpose().getRow(2));

		LOG(Debug,info)
			<< "Computed rowVec=" << props.getPropertyAs<util::fvector4>("rowVec") << ", "
			<< "columnVec=" << props.getPropertyAs<util::fvector4>("columnVec") << " and "
			<< "sliceVec=" << props.getPropertyAs<util::fvector4>("sliceVec") << " from sform";

		props.remove("nifti/srow_x");
		props.remove("nifti/srow_y");
		props.remove("nifti/srow_z");
	}
	static void useQForm(util::PropertyMap &props){
		
		// orientation //////////////////////////////////////////////////////////////////////////////////
		//see http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/quatern.html
		//and http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/qformExt.jpg
		double b=props.getPropertyAs<double>( "nifti/quatern_b");
		double c=props.getPropertyAs<double>( "nifti/quatern_c");
		double d=props.getPropertyAs<double>( "nifti/quatern_d");
		double a=sqrt(1.0-(b*b+c*c+d*d));

		LOG(Debug,info)
			<< "Using qform (" << props.propertyValue( "nifti/qform_code").toString()
			<< ") quaternion=" << util::fvector4(a,b,c,d) << " with qfac=" << props.propertyValue("nifti/qfac").toString()
			<< ", pixdim=" << props.propertyValue("nifti/pixdim").toString()
			<< " and qoffset= " << props.propertyValue("nifti/qoffset").toString();

		const util::Matrix4x4<double> M(
			util::fvector4(a*a+b*b-c*c-d*d,2*b*c-2*a*d,2*b*d+2*a*c),
			util::fvector4(2*b*c+2*a*d,a*a+c*c-b*b-d*d,2*c*d-2*a*b),
			util::fvector4(2*b*d-2*a*c,2*c*d+2*a*b,a*a+d*d-c*c-b*b)
		);
		const util::Matrix4x4<double> image2isis=nifti2isis.dot(M);

		props.setPropertyAs<util::fvector4>("rowVec",image2isis.transpose().getRow(0));
		props.setPropertyAs<util::fvector4>("columnVec",image2isis.transpose().getRow(1));
		props.setPropertyAs<util::fvector4>("sliceVec",image2isis.transpose().getRow(2));

		LOG(Debug,info)
			<< "Computed rowVec=" << props.getPropertyAs<util::fvector4>("rowVec") << ", "
			<< "columnVec=" << props.getPropertyAs<util::fvector4>("columnVec") << " and "
			<< "sliceVec=" << props.getPropertyAs<util::fvector4>("sliceVec") << " from qform";

		props.remove("nifti/quatern_b");
		props.remove("nifti/quatern_c");
		props.remove("nifti/quatern_d");

		// indexOrigin //////////////////////////////////////////////////////////////////////////////////
		props.setPropertyAs<util::fvector4>("indexOrigin",nifti2isis.dot(props.getPropertyAs<util::fvector4>( "nifti/qoffset")));
		LOG(Debug,info) << "Computed indexOrigin=" << props.getPropertyAs<util::fvector4>("indexOrigin") << " from qform";
		props.remove("nifti/qoffset");

		// voxelSize //////////////////////////////////////////////////////////////////////////////////
		props.transform<util::fvector4>("nifti/pixdim","voxelSize");
		LOG(Debug,info)	<< "Computed voxelSize=" << props.getPropertyAs<util::fvector4>("voxelSize") << " from qform";
	}
};

// The nifti coord system:
// The (x,y,z) coordinates refer to the CENTER of a voxel.
// In methods 2 and 3, the (x,y,z) axes refer to a subject-based coordinate system, with +x = Right  +y = Anterior  +z = Superior.
// So, the transform from nifti to isis is:
const util::Matrix4x4<short> ImageFormat_NiftiSa::nifti2isis(
	util::vector4<short>(-1, 0, 0, 0),
	util::vector4<short>( 0,-1, 0, 0),
	util::vector4<short>( 0, 0, 1, 0),
	util::vector4<short>( 0, 0, 0, 1)
);

}
}



isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_NiftiSa();
}
