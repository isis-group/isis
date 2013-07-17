/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef IMAGEFORMAT_NIFTI_SA_HPP
#define IMAGEFORMAT_NIFTI_SA_HPP

#define NIFTI_TYPE_BINARY          1
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

#include <DataStorage/io_interface.h>
#include <CoreUtils/matrix.hpp>
#include <sys/stat.h>

namespace isis
{
namespace image_io
{
namespace _internal
{

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

class WriteOp: public data::ChunkOp, protected data::_internal::NDimensional<4>
{
protected:
	std::set<data::dimensions> flip_list;
	data::FilePtr m_out;
	size_t m_voxelstart, m_bpv;
	WriteOp( const isis::data::Image &image, size_t bitsPerVoxel );
	virtual bool doCopy( data::Chunk &ch, util::vector4<size_t> posInImage ) = 0;
	void applyFlipToCoords ( util::vector4< size_t > &coords, data::dimensions blockdims );
	void applyFlipToData ( data::ValueArrayReference &dat, util::vector4< size_t > chunkSize );
	void applyFlipToData ( data::Chunk& dat );
public:
	virtual ~WriteOp() {}
	nifti_1_header *getHeader();
	virtual unsigned short getTypeId() = 0;
	virtual size_t getDataSize();

	bool operator()( data::Chunk &ch, util::vector4<size_t> posInImage );
	bool setOutput( const std::string &filename, size_t voxelstart = 352 );
	void addFlip( data::dimensions dim );
};

}


class ImageFormat_NiftiSa: public FileFormat
{
	static const util::Matrix4x4<short> nifti2isis;
	static const util::Selection formCode;

	typedef bool(*demuxer_type)(const util::PropertyValue &value,std::list<data::Chunk> &chunks,util::PropertyMap::PropPath name);

	/// get the tranformation matrix from image space to Nifti space using row-,column and sliceVec from the given PropertyMap
	static util::Matrix4x4<double> getNiftiMatrix( const util::PropertyMap &props );
	static void useSForm( util::PropertyMap &props );
	static void useQForm( util::PropertyMap &props );
	static bool storeQForm( const util::PropertyMap &props, _internal::nifti_1_header *head );
	static void storeSForm( const util::PropertyMap &props, _internal::nifti_1_header *head );
	std::map<short, unsigned short> nifti_type2isis_type;
	std::map<unsigned short, short> isis_type2nifti_type;
	std::map<unsigned short,demuxer_type> prop_demuxer;
	template<typename T, typename NEW_T> static unsigned short typeFallBack( const std::string name ) {
		LOG( Runtime, info ) << data::ValueArray<T>::staticName() <<  " is not supported by " << name << " falling back to " << data::ValueArray<NEW_T>::staticName();
		return data::ValueArray<NEW_T>::staticID;
	}
	static void guessSliceOrdering( const data::Image img, char &slice_code, float &slice_duration );
	static std::list<data::Chunk> parseSliceOrdering( const boost::shared_ptr< _internal::nifti_1_header > &head, data::Chunk current );

	static bool parseDescripForSPM( util::PropertyMap &props, const char desc[] );
	static void storeDescripForSPM( const isis::util::PropertyMap &props, char desc[] );
	static void storeHeader( const util::PropertyMap &props, _internal::nifti_1_header *head );
	static float determinant( const util::Matrix3x3<float> &m );
	static std::list<data::Chunk> parseHeader( const boost::shared_ptr< _internal::nifti_1_header > &head, data::Chunk props );
	std::auto_ptr<_internal::WriteOp> getWriteOp( const data::Image &src, util::istring dialect );
	data::ValueArray<bool> bitRead( isis::data::ValueArray< uint8_t > src, size_t length );
	bool checkSwapEndian ( boost::shared_ptr< _internal::nifti_1_header > header );
	void flipGeometry( data::Image &image, data::dimensions flipdim );
	void translateFromDcmMetaConst( util::PropertyMap& orig );
	void translateFromDcmMetaSlices( util::PropertyMap& orig );
public:
	ImageFormat_NiftiSa();
	std::string getName()const;
	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const util::istring &/*dialect*/, boost::shared_ptr<util::ProgressFeedback> progress )  throw( std::runtime_error & );
	void write( const data::Image &image, const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> progress )  throw( std::runtime_error & );
	bool tainted()const {return false;}//internal plugins are not tainted
	util::istring dialects( const std::string &/*filename*/ )const {return "fsl spm withExtProtocols";}

protected:
	util::istring suffixes( io_modes mode = both )const;
};


}
}
#endif // IMAGEFORMAT_NIFTI_SA_HPP
