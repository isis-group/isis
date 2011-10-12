#include <DataStorage/io_interface.h>
#include <DataStorage/fileptr.hpp>
#include <CoreUtils/matrix.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/type_traits/make_signed.hpp>

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

#define NIFTI_XFORM_UNKNOWN      0 /* Arbitrary coordinates (Method 1). */
#define NIFTI_XFORM_SCANNER_ANAT 1 /* Scanner-based anatomical coordinates */
#define NIFTI_XFORM_ALIGNED_ANAT 2 /* Coordinates aligned to another file's, or to anatomical "truth". */
#define NIFTI_XFORM_TALAIRACH    3 /* Coordinates aligned to Talairach-Tournoux Atlas; (0,0,0)=AC, etc. */
#define NIFTI_XFORM_MNI_152      4 /* MNI 152 normalized coordinates. */


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
		nifti_type2isis_type[NIFTI_TYPE_INT8 ]=data::ValuePtr< int8_t>::staticID;
		nifti_type2isis_type[NIFTI_TYPE_INT16]=data::ValuePtr<int16_t>::staticID;
		nifti_type2isis_type[NIFTI_TYPE_INT32]=data::ValuePtr<int32_t>::staticID;
		nifti_type2isis_type[NIFTI_TYPE_INT64]=data::ValuePtr<int64_t>::staticID;

		nifti_type2isis_type[NIFTI_TYPE_UINT8 ]=data::ValuePtr< uint8_t>::staticID;
		nifti_type2isis_type[NIFTI_TYPE_UINT16]=data::ValuePtr<uint16_t>::staticID;
		nifti_type2isis_type[NIFTI_TYPE_UINT32]=data::ValuePtr<uint32_t>::staticID;
		nifti_type2isis_type[NIFTI_TYPE_UINT64]=data::ValuePtr<uint64_t>::staticID;

		nifti_type2isis_type[NIFTI_TYPE_FLOAT32]=data::ValuePtr<float>::staticID;
		nifti_type2isis_type[NIFTI_TYPE_FLOAT64]=data::ValuePtr<double>::staticID;

		typedef std::map<short,unsigned short>::const_reference ref_type;
		BOOST_FOREACH(ref_type ref, nifti_type2isis_type){
			isis_type2nifti_type[ref.second]=ref.first;
		}

/*		nii2isis[NIFTI_TYPE_COMPLEX64 ]=data::ValuePtr<std::complex< float > >::staticID;
		nii2isis[NIFTI_TYPE_COMPLEX128]=data::ValuePtr<std::complex< double > >::staticID;*/
	}
protected:
	std::string suffixes()const {
		return std::string( ".nii" );
	}
	std::map<short,unsigned short> nifti_type2isis_type;
	std::map<unsigned short,short> isis_type2nifti_type;
	template<typename T> static unsigned short typeFallBack(){
		typedef typename boost::make_signed<T>::type NEW_T;
		LOG(Runtime,info) << util::Value<T>::staticName() <<  " is not supported by fsl falling back to " << util::Value<NEW_T>::staticName();
		return util::Value<NEW_T>::staticID;
	}
	static void guessSliceOrdering(const data::Image img,char &slice_code, float &slice_duration){
		std::list< util::PropertyValue > dummy=img.getChunksProperties("acquisitionNumber");
		const std::vector< util::PropertyValue > acnums(dummy.begin(),dummy.end());

		dummy=img.getChunksProperties("acquisitionTime");
		const std::vector< util::PropertyValue > actimes(dummy.begin(),dummy.end());
		
		if(acnums.size()<=1){ // seems like there is only one chunk - slice ordering doesnt matter - just choose NIFTI_SLICE_SEQ_INC
			slice_code=NIFTI_SLICE_SEQ_INC;
		} else if(acnums[0].isEmpty()){
			LOG(Runtime,error) << "There is no acquisitionNumber for the first chunk, assuming normal slice ordering.";
			slice_code=NIFTI_SLICE_SEQ_INC;
		} else {
			const util::PropertyValue first=acnums.front();
			const util::PropertyValue second=acnums[1];
			const util::PropertyValue middle=acnums[acnums.size()/2+.5];
			
			if(first->gt(*second)){ // second slice has a lower number than the first => decrementing
				if(middle->gt(*second)){ // if the middle number is greater than the second its interleaved
					LOG(Runtime,info)
						<< "The \"middle\" acquisitionNumber (" << middle.toString() << ") is greater than the second (" << second.toString()
						<< ") assuming decrementing interleaved slice order";
					slice_code=NIFTI_SLICE_ALT_DEC;
				} else { // assume "normal" otherwise
					LOG(Runtime,info)
						<< "The first acquisitionNumber (" << first.toString() << ") is greater than the second (" << second.toString()
						<< ") assuming decrementing slice order";
					slice_code=NIFTI_SLICE_SEQ_DEC;
				}
			} else { // assume incrementing
				if(middle->lt(*second)){ // if the middle number is less than the second ist interleaved
					LOG(Runtime,info)
						<< "The \"middle\" acquisitionNumber (" << middle.toString() << ") is less than the second (" << second.toString()
						<< ") assuming incrementing interleaved slice order";
					slice_code=NIFTI_SLICE_ALT_INC;
				} else { // assume "normal" otherwise
					LOG(Runtime,info)
						<< "The first acquisitionNumber (" << first.toString() << ") is not greater than the second (" << second.toString()
						<< ") assuming incrementing slice order";
					slice_code=NIFTI_SLICE_SEQ_INC;
				}
			}
		}

		if(actimes.size()>1 && !actimes[1].isEmpty()){
			slice_duration=fabs(actimes[1]->as<float>()-actimes[0]->as<float>());
			if(slice_code==NIFTI_SLICE_SEQ_INC || slice_code==NIFTI_SLICE_SEQ_DEC){ // if its interleaved there was another slice between 0 and 1
				slice_duration/=2;
			}
		}

	}
	static bool parseDescripForSPM(util::PropertyMap &props, const char desc[]){
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
	static void storeHeader(const util::PropertyMap &props,_internal::nifti_1_header *head){
		// implicit stuff
		head->intent_code=0;
		head->slice_start=0;
		head->slice_end=head->dim[3];

		//in isis length is allways mm and time duration is allways msecs
		head->xyzt_units=NIFTI_UNITS_MM|NIFTI_UNITS_MSEC;

		//store description if there is one
		if(props.hasProperty("sequenceDescription"))
			strncpy(head->descrip,props.getPropertyAs<std::string>("sequenceDescription").c_str(),80);

		// store niftis original sform if its there
		if(props.hasProperty("nifti/sform_code")){
			head->sform_code=props.getPropertyAs<short>("nifti/sform_code");
			if(props.hasProperty("nifti/srow_x") && props.hasProperty("nifti/srow_y") && props.hasProperty("nifti/srow_z")){
				props.getPropertyAs<util::fvector4>("nifti/srow_x").copyTo(head->srow_x);
				props.getPropertyAs<util::fvector4>("nifti/srow_y").copyTo(head->srow_y);
				props.getPropertyAs<util::fvector4>("nifti/srow_z").copyTo(head->srow_z);
			}
		}
		// store niftis original qform if its there
		if(props.hasProperty("nifti/qform_code")){
			head->qform_code=props.getPropertyAs<short>("nifti/qform_code");
			if( props.hasProperty("nifti/quatern_b") && props.hasProperty("nifti/quatern_c") && props.hasProperty("nifti/quatern_d") &&
			    props.hasProperty("nifti/qoffset") && props.hasProperty("nifti/qfac")
			){
				const util::fvector4 offset=props.getPropertyAs<util::fvector4>("nifti/qoffset");
				head->quatern_b = props.getPropertyAs<float>("nifti/quatern_b");
				head->quatern_c = props.getPropertyAs<float>("nifti/quatern_c");
				head->quatern_d = props.getPropertyAs<float>("nifti/quatern_d");
				head->pixdim[0] = props.getPropertyAs<float>("nifti/qfac");
				head->qoffset_x=offset[0];head->qoffset_y=offset[1];head->qoffset_z=offset[2];
			}
		}

		//store voxel size
		props.getPropertyAs<util::fvector4>("voxelSize").copyTo(head->pixdim+1);

		//store current orientation (may override values set above)
		if(!storeQForm(props,head)) //try to encode as quaternion
			storeSForm(props,head); //fall back to normal matrix

		strcpy(head->magic,"n+1");
	}
	static void parseHeader(const _internal::nifti_1_header *head,data::Chunk &props){
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
			switch(head->slice_code){ //@todo check this
				case 0:
				case NIFTI_SLICE_SEQ_INC:
					if(head->slice_duration){ // if there is no slice duration, and the sequence is "normal" there is no use in numbering
						for(uint32_t i=0;i<head->dim[3];i++)
							props.propertyValueAt("acquisitionNumber",i)=i;
					} else {
						props.propertyValue("acquisitionNumber")=0;
					}
					break;
				case NIFTI_SLICE_SEQ_DEC:
					for(uint32_t i=0;i<head->dim[3];i++)
						props.propertyValueAt("acquisitionNumber",head->dim[3]-i-1)=i;
					break;
				case NIFTI_SLICE_ALT_INC:{
					uint32_t i=0,cnt;
					for(cnt=0;i<floor(head->dim[3]/2+.5);i++,cnt+=2)
						props.propertyValueAt("acquisitionNumber",i)=cnt;
					for(cnt=1;i<head->dim[3];i++,cnt+=2)
						props.propertyValueAt("acquisitionNumber",i)=cnt;
					}break;
				case NIFTI_SLICE_ALT_DEC:{
					uint32_t i=0,cnt;
					for(cnt=0;i<floor(head->dim[3]/2+.5);i++,cnt+=2)
						props.propertyValueAt("acquisitionNumber",head->dim[3]-i-1)=cnt;
					for(cnt=1;i<head->dim[3];i++,cnt+=2)
						props.propertyValueAt("acquisitionNumber",head->dim[3]-i-1)=cnt;
					}break;
				default:LOG(Runtime,error) << "Unknown slice code " << util::MSubject(head->slice_code);break;
			}

			if(head->slice_duration ){
				for(uint32_t i=0;i<head->dim[3];i++){
					props.propertyValueAt("acquisitionTime",  i)=props.propertyValueAt("acquisitionNumber",i)->as<float>()*head->slice_duration*time_fac;
				}
			}
		} else {
			LOG_IF(head->slice_code,Runtime,warning) << "Sorry slice_code!=0 is currently only supportet for 3D-images, ignoring it.";
		}

		// Tr
		if(dims>4)
			props.setPropertyAs<uint16_t>("repetitionTime",head->pixdim[4]*time_fac);

		// sequenceDescription
		if(!parseDescripForSPM(props, head->descrip)) // if descrip dos not hold Te,Tr and stuff (SPM dialect)
			props.setPropertyAs<std::string>("sequenceDescription",head->descrip);// use it the usual way

		// TODO: at the moment scaling not supported due to data type changes
		if ( !(head->scl_slope == 1 || head->scl_inter==0) ) {
			//          throwGenericError( std::string( "Scaling is not supported at the moment. Scale Factor: " ) + util::Value<float>( scale ).toString() );
			LOG( Runtime, error ) << "Scaling is not supported at the moment.";
		}



	}

public:
	std::string getName()const {return "Nifti standalone";}

	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &/*dialect*/ )  throw( std::runtime_error & ) {
		data::FilePtr mfile(filename);

		//get the header - we use it directly from the file
		const _internal::nifti_1_header *header= reinterpret_cast<const _internal::nifti_1_header*>(&mfile[0]);

		if(header->intent_code!=0){
			throwGenericError(std::string("only intent_code==0 is supportet"));
		}

		//set up the size - copy dim[0] values from dim[1]..dim[dim[0]]
		util::FixedVector<size_t,4> size;
		size.fill(1);

		size.copyFrom(header->dim+1,header->dim+1+header->dim[0]);

		data::ValuePtrReference data=mfile.atByID(nifti_type2isis_type[header->datatype],header->vox_offset);
		LOG(Runtime,info) << "Mapping nifti image as " << data->getTypeName() << " of length " << data->getLength();
		LOG_IF((size_t)header->bitpix!=data->bytesPerElem()*8,Runtime,warning)
			<< "nifti field bitpix does not fit the bytesize of the given datatype ("
			<< data->getTypeName() << "/" << header->bitpix <<  ")";

		chunks.push_back(data::Chunk(data,size[0],size[1],size[2],size[3]));
		parseHeader(header,chunks.back());

		return 1;
	}

	void write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		class CopyOp:public data::ChunkOp{
			data::FilePtr &m_out;
			const data::Image &m_image;
			const unsigned short m_ID;
			const size_t m_bytesPerPixel;
			const data::scaling_pair m_scale;
		public:
			CopyOp(const data::Image &image,data::FilePtr &out):
				m_image(image),m_out(out),m_ID(image.getMajorTypeID()),m_bytesPerPixel(image.getBytesPerVoxel()),m_scale(image.getScalingTo( m_ID )){}
            bool operator()(data::Chunk &ch, util::FixedVector< size_t, 4 > posInImage){
				size_t offset=384+m_image.getLinearIndex(posInImage)*m_bytesPerPixel;
				data::ValuePtrReference out_data=m_out.atByID(m_ID,offset,ch.getVolume());
				ch.asValuePtrBase().copyTo(*out_data,m_scale);
			}
		};
		
		const size_t bpv=image.getBytesPerVoxel();
		unsigned short target_id=image.getMajorTypeID();

		if(util::istring(dialect.c_str())=="fsl"){
			switch(target_id){
				case util::Value<uint16_t>::staticID:target_id=typeFallBack<uint16_t>();break;
				case util::Value<uint32_t>::staticID:target_id=typeFallBack<uint32_t>();break;
			}
		}
		
		const size_t datasize=image.getVolume()*bpv;
		if(isis_type2nifti_type[target_id]){ // "normal types"

			// open/map the new file
			data::FilePtr out(filename,datasize+384,true);

			// get the first 384 bytes as header
			_internal::nifti_1_header *header= reinterpret_cast<_internal::nifti_1_header*>(&out[0]);
			memset(header,0,sizeof(_internal::nifti_1_header));
			storeHeader(image,header);

			// set the datatype
			header->sizeof_hdr=header->vox_offset=sizeof(_internal::nifti_1_header);
			header->bitpix=bpv*8;
			header->datatype=isis_type2nifti_type[target_id];

			// store the image size in dim and fill up the rest with "1" (to prevent fsl from exploding)
			header->dim[0]=image.getRelevantDims();
			image.getSizeAsVector().copyTo(header->dim+1);
			std::fill(header->dim+5,header->dim+8,1);

			guessSliceOrdering(image,header->slice_code,header->slice_duration);

			std::pair< float, float > minmax=image.getMinMaxAs<float>();
			header->cal_min=minmax.first;header->cal_max=minmax.second;

			CopyOp do_copy(image,out);
			const_cast<data::Image&>( image).foreachChunk(do_copy); // @todo we _do_ need a const version of foreachChunk/Voxel
		} else {
			LOG(Runtime,error) << "Sorry, the datatype " << util::MSubject( image.getMajorTypeName() )<< " is not supportet for nifti output";
			throwGenericError("unsupported datatype");
		}

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
	static bool storeQForm(const util::PropertyMap &props,_internal::nifti_1_header *head){
		const util::Matrix4x4<float> image2isis=util::Matrix4x4<float>(
			props.getPropertyAs<util::fvector4>("rowVec"),
			props.getPropertyAs<util::fvector4>("columnVec"),
			props.getPropertyAs<util::fvector4>("sliceVec")
		).transpose();// the columns of the transform matrix are row-, slice- and

		const util::Matrix4x4<float> image2nifti=nifti2isis.transpose().dot(image2isis);// apply inverse transform from nifti to isis

		// take values of the 3x3 matrix == analog to the nifti reference implementation
		float r11=image2nifti.elem(0,0),r21=image2nifti.elem(0,1),r31=image2nifti.elem(0,2);//first column
		float r12=image2nifti.elem(1,0),r22=image2nifti.elem(1,1),r32=image2nifti.elem(1,2);//second column
		float r13=image2nifti.elem(2,0),r23=image2nifti.elem(2,1),r33=image2nifti.elem(2,2);//third column

		// compute the determinant to determine if the transformation is proper
		if(r11*r22*r33-r11*r32*r23-r21*r12*r33+r21*r32*r13+r31*r12*r23-r31*r22*r13 > 0){
			head->pixdim[0]=1;
		} else { // improper => flip 3rd column
			r13 = -r13 ; r23 = -r23 ; r33 = -r33 ;
			head->pixdim[0]=-1;
		}
		
		head->qform_code=NIFTI_XFORM_SCANNER_ANAT;
		// the following was more or less stolen from the nifti reference implementation
		const float a_square=r11 + r22 + r33 + 1;
		if(a_square>0.5) { // simple case
			const float a = 0.5  * sqrt(a_square);
			head->quatern_b = 0.25 * (r32-r23) / a;
			head->quatern_c = 0.25 * (r13-r31) / a;
			head->quatern_d = 0.25 * (r21-r12) / a;
		} else {                       /* trickier case */
			float xd = 1.0 + r11 - (r22+r33) ;  /* 4*b*b */
			float yd = 1.0 + r22 - (r11+r33) ;  /* 4*c*c */
			float zd = 1.0 + r33 - (r11+r22) ;  /* 4*d*d */
			float a;
			if( xd > 1.0 ){
				head->quatern_b = 0.5l * sqrt(xd) ;
				head->quatern_c = 0.25l* (r12+r21) / head->quatern_b ;
				head->quatern_d = 0.25l* (r13+r31) / head->quatern_b ;
				a = 0.25l* (r32-r23) / head->quatern_b ;
			} else if( yd > 1.0 ){
				head->quatern_c = 0.5l * sqrt(yd) ;
				head->quatern_b = 0.25l* (r12+r21) / head->quatern_c ;
				head->quatern_d = 0.25l* (r23+r32) / head->quatern_c ;
				a = 0.25l* (r13-r31) / head->quatern_c ;
			} else {
				head->quatern_d = 0.5l * sqrt(zd) ;
				head->quatern_b = 0.25l* (r13+r31) / head->quatern_d ;
				head->quatern_c = 0.25l* (r23+r32) / head->quatern_d ;
				a = 0.25* (r21-r12) / head->quatern_d ;
			}
			if( a < 0.0 ){
				head->quatern_b=-head->quatern_b ;
				head->quatern_c=-head->quatern_c ;
				head->quatern_d=-head->quatern_d;
			}
		}

		const util::fvector4 nifti_offset=nifti2isis.transpose().dot(props.getPropertyAs<util::fvector4>("indexOrigin"));
		head->qoffset_x=nifti_offset[0];
		head->qoffset_y=nifti_offset[1];
		head->qoffset_z=nifti_offset[2];
		
		return true;
	}
	static void storeSForm(const util::PropertyMap &props,_internal::nifti_1_header *head){

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
