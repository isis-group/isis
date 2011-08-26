#include <DataStorage/io_interface.h>
#include <DataStorage/fileptr.hpp>

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
	util::PropertyMap header2PropMap(const _internal::nifti_1_header *head){
		util::PropertyMap props;
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

		// voxel size
		util::fvector4 v_size;v_size.copyFrom(head->pixdim,head->pixdim+std::min<unsigned short>(dims,3));
		props.setPropertyAs<util::fvector4>("voxelSize",v_size*size_fac);

		props.setPropertyAs<uint16_t>("sequenceNumber",0);
		props.setPropertyAs<uint16_t>("acquisitionNumber",0);
		props.setPropertyAs<std::string>("sequenceDescription",head->descrip);
		props.setPropertyAs( "indexOrigin", util::fvector4( head->qoffset_x, head->qoffset_y, head->qoffset_z, 0 ) );
		
		
		return props;
	}
public:
	std::string getName()const {return "Nifti standalone";}

	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		data::FilePtr mfile(filename);

		//get the header - we use it directly from the file
		_internal::nifti_1_header *header= reinterpret_cast<_internal::nifti_1_header*>(&mfile[0]);
		util::PropertyMap props=header2PropMap(header);

		//set up the size - copy dim[0] values from dim[1]..dim[dim[0]]
		util::FixedVector<size_t,4> size;
		size.fill(1);

		size.copyFrom(header->dim+1,header->dim+1+header->dim[0]);

		data::ValuePtrReference data=mfile.atByID(nii2isis[header->datatype],header->vox_offset);
		LOG(Runtime,info) << "Mapping nifti image as " << data->getTypeName() << " of length " << data->getLength();
		return 0;
	}

	void write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_NiftiSa();
}
