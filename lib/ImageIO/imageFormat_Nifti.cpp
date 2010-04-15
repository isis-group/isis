/****************************************************************
 *
 * <Copyright information>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Lydia Hellrung, hellrung@cbs.mpg.de, 2009
 *
 * ImageFormatNii.cpp
 *
 * Description:
 *
 *  Created on: Aug 12, 2009
 *      Author: hellrung
 ******************************************************************/

//LOCAL INCLUDES
#include <DataStorage/io_interface.h>
#include <CoreUtils/type.hpp>
#include "common.hpp"
#include "CoreUtils/vector.hpp"

//SYSTEM INCLUDES
#include <nifti1_io.h>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace isis
{
namespace image_io
{

/*************************
 * creating an own chunk for better Nifti handling (whole file handling)
 **************************/
namespace _internal
{

class NiftiChunk : public data::Chunk
{
public:
	template<typename T, typename D> NiftiChunk(T* src, D del, size_t width, size_t height, size_t slices, size_t timesteps) :
		data::Chunk(src, del, width, height, slices, timesteps)
	{
		LOG(ImageIoDebug, info) << "create NiftiChunk";
	}

private:
	NiftiChunk(const NiftiChunk&); // no standard copy constructor
	NiftiChunk& operator=(const NiftiChunk&); // no copy operator
};// class NiftiChunk
}//namespace _internal


/*************************
 * now, the real plugin for Nifti conversion
 **************************/
class ImageFormat_Nifti : public FileFormat
{
	// own chunk creations, like NiftiChunk, usually having it's own deleter
	struct Deleter
	{
		//members
		nifti_image *m_pNiImage;
		std::string m_filename;
		//constructor
		Deleter(nifti_image *ni, const std::string& filename) :
			m_pNiImage(ni), m_filename(filename){}

		//the most important operator
		void operator ()(void *at)
		{
			LOG_IF(NULL == m_pNiImage, ImageIoLog, error)
					<<	"Trying to close non-existing nifti file: " << util::MSubject(m_filename);
			LOG(ImageIoDebug, info) << "Closing Nifti-Chunk file" << util::MSubject(m_filename);
			//clean up with the function from nifti1_io.h
			nifti_image_free(m_pNiImage);
		}
	};

public:
	enum vectordirection {readDir=0,phaseDir,sliceDir,centerVec,voxelSizeVec};

	std::string suffixes() {
		return std::string(".nii.gz .nii .hdr");}

	std::string dialects() {
		return std::string("fsl");}

	std::string name() {
		//TODO: wahrscheinlich sollten die Namen irgendwie so aussehen "mpg.cbs.nii"?
		return "Nifti";}

	bool tainted() {
		return false;}//internal plugins are not tainted

	/***********************
	 * load file
	 ************************/
	int load(data::ChunkList &retList,const std::string& filename, const std::string& dialect)
	{
		//read the file with the function from nifti1_io.h
		nifti_image* ni = nifti_image_read(filename.c_str(), true);
		if (NULL == ni) {
			return 0;
		}
		// 0.0 would mean "not in use" - so for better handling use a 1.0
		float scale = ni->scl_slope ? ni->scl_slope : 1.0;
		// TODO: at the moment scaling not supported due to data type changes
		if (1.0 != scale) {
			LOG(ImageIoDebug, isis::error) << "Scaling is not supported at the moment. Scale Factor:  " << scale;
			return 0;
		}
		LOG(ImageIoDebug, isis::info) << "datatype to load from nifti " << ni->datatype;

		boost::shared_ptr<data::Chunk> retChunk;
		Deleter del(ni, filename);
		switch(ni->datatype)
		{
			case DT_UINT8:
				retChunk.reset(new _internal::NiftiChunk(static_cast<uint8_t*>(ni->data),del,ni->dim[1],ni->dim[2], ni->dim[3], ni->dim[4]));
				break;
			case DT_INT8:
				retChunk.reset(new _internal::NiftiChunk(static_cast<int8_t*>(ni->data),del,ni->dim[1],ni->dim[2], ni->dim[3], ni->dim[4]));
				break;
			case DT_INT16:
				retChunk.reset(new _internal::NiftiChunk(static_cast<int16_t*>(ni->data),del,ni->dim[1],ni->dim[2], ni->dim[3], ni->dim[4]));
				break;
			case DT_UINT16:
				retChunk.reset(new _internal::NiftiChunk(static_cast<uint16_t*>(ni->data),del,ni->dim[1],ni->dim[2], ni->dim[3], ni->dim[4]));
				break;
			case DT_UINT32:
				retChunk.reset(new _internal::NiftiChunk(static_cast<uint32_t*>(ni->data),del,ni->dim[1],ni->dim[2], ni->dim[3], ni->dim[4]));
				break;
			case DT_INT32:
				retChunk.reset(new _internal::NiftiChunk(static_cast<int32_t*>(ni->data),del,ni->dim[1],ni->dim[2], ni->dim[3], ni->dim[4]));
				break;
			case DT_FLOAT32:
				retChunk.reset(new _internal::NiftiChunk(static_cast<float*>(ni->data),del,ni->dim[1],ni->dim[2], ni->dim[3], ni->dim[4]));
				break;
			case DT_FLOAT64:
				retChunk.reset(new _internal::NiftiChunk(static_cast<double*>(ni->data),del,ni->dim[1],ni->dim[2], ni->dim[3], ni->dim[4]));
				break;
			default:
				LOG(ImageIoDebug, isis::error) << "Unsupported datatype " << ni->datatype;
				return 0;
		}

		// don't forget to take the properties with
		copyHeaderFromNifti(*retChunk, *ni);
		// push the completed NiftiChunk into the list
		retList.push_back(*retChunk);
		return retList.size();
	}


	/***********************
	 * write file
	 ************************/
	bool write(const data::Image &image, const std::string& filename, const std::string& dialect)
	{
		LOG(ImageIoDebug, isis::info) << "Write Nifti.";
		boost::filesystem::path boostFilename(filename);
		//default init for nifti image
		nifti_image ni;
		memset(&ni,0, sizeof(nifti_image)); //set everything to zero - default value for "not used"
		ni.nu=ni.nv=ni.nw=1;
		ni.datatype = DT_UNKNOWN;
		ni.data = NULL;
		ni.fname=const_cast<char*>(filename.c_str());
		// get dim info from image
		util::fvector4 dimensions = image.sizeToVector();
		//set the props from the image to the nifti file
		copyHeaderToNifti(image, ni);
		// set filename for resulting image(s) due to Analyze vs. Nifti
		ni.fname=const_cast<char*>(filename.c_str()); // header name
		boost::filesystem::path imgname;
		if ("hdr" == extension(boostFilename))
		{
			ni.nifti_type = 0; // that's ANALYZE ID
			imgname  =  change_extension(boostFilename, ".img");
			ni.iname=const_cast<char*>(imgname.string().c_str());
		}
		else {
			ni.nifti_type = 1; // that's NIFTI ID
			ni.iname=const_cast<char*>(filename.c_str());
		}

		// FSL compatibility
		if(strcasecmp(dialect.c_str(), "fsl")) {
		//	stuffFslCompatibility(image, ni);
		}

		// copy the data to the nifti image
		LOG(ImageIoLog, isis::info) << image.getChunk(0,0,0,0).typeName();
		switch(image.getChunk(0,0,0,0).typeID()){
		case util::TypePtr<int8_t>::staticID:   ni.datatype = DT_INT8;   copyDataToNifti<int8_t>(image, ni);   break;
		case util::TypePtr<u_int8_t>::staticID: ni.datatype = DT_UINT8;  copyDataToNifti<u_int8_t>(image, ni); break;

		case util::TypePtr<int16_t>::staticID:  ni.datatype = DT_INT16;  copyDataToNifti<int16_t>(image, ni);  break;
		case util::TypePtr<u_int16_t>::staticID:ni.datatype = DT_UINT16; copyDataToNifti<u_int16_t>(image, ni);break;
		
		case util::TypePtr<int32_t>::staticID:  ni.datatype = DT_INT32;  copyDataToNifti<int32_t>(image, ni);  break;
		case util::TypePtr<u_int32_t>::staticID:ni.datatype = DT_UINT32; copyDataToNifti<u_int16_t>(image, ni);break;
		
		case util::TypePtr<float>::staticID:    ni.datatype = DT_FLOAT32;copyDataToNifti<float>(image, ni);    break;
		case util::TypePtr<double>::staticID:   ni.datatype = DT_FLOAT64;copyDataToNifti<double>(image, ni);   break;
		default:
			LOG(ImageIoLog, error) << "Datatype " << util::TypePtr<float>::staticName() << " cannot be written!";
		}
		//now really write the nifti file with the function from nifti1_io.h
		errno=0; //reset errno
		nifti_image_write(&ni); //write the image - in case of a failure errno should be set
		if(errno) {
			//not succesfully written
			LOG(ImageIoLog, error) << "Could not write to "<< filename << "(" << strerror(errno) << ")";
			boost::filesystem::remove(boostFilename);
			return false;
		}

		return true; // everything seems to be fine
	}

	/****************************************
	 * PRIVATE
	 ****************************************/
private:

	void copyHeaderFromNifti(data::Chunk& retChunk, const nifti_image& ni)
	{
		util::fvector4 dimensions(ni.dim[1], ni.ndim >= 2 ? ni.dim[2] : 1,
				ni.ndim >= 3 ? ni.dim[3] : 1, ni.ndim >= 4 ? ni.dim[4] : 1);
		LOG(ImageIoLog, info)<< "size of chunk " << dimensions << "/" << ni.ndim;

		for (int t = 0; t < dimensions[3]; t++)
		{
			util::fvector4 offsets(ni.qoffset_x, ni.qoffset_y, ni.qoffset_z, 0);
			//retChunk.setProperty("acquisitionTime", );
			retChunk.setProperty("indexOrigin",offsets);
			retChunk.setProperty("acquisitionNumber", t);
			retChunk.setProperty("sequenceNumber", 1);
			retChunk.setProperty("readVec", getVector(ni, readDir));
			retChunk.setProperty("phaseVec", getVector(ni, phaseDir));
			retChunk.setProperty("sliceVec", getVector(ni, sliceDir));
			retChunk.setProperty("voxelSize",getVector(ni, voxelSizeVec));
			retChunk.setProperty("centerVec",getVector(ni, centerVec));
			retChunk.setProperty("study/description", std::string(ni.descrip) );
			retChunk.setProperty("series/description", std::string(ni.intent_name) );

			//just some LOGS
			LOG(ImageIoLog, info) << "dims at all " << dimensions;
			LOG(ImageIoLog, info) << "Offset values from nifti" << offsets;
			LOG(ImageIoLog,info) << "FOV read/phase/slice/voxelsize:"
			<< getVector(ni, readDir)
			<< " / " << getVector(ni, phaseDir)
			<< " / " << getVector(ni, sliceDir)
			<< getVector(ni, voxelSizeVec);
		}
	}

	util::fvector4 getVector(const nifti_image& ni, const enum vectordirection& dir)
	{
		util::fvector4 retVec(0,0,0,0);
		float div = 1.0; // factor for read (dx) /phase (dy) /slice (dz) vectors
		switch (dir)
		{
			case readDir:
			div = ni.dx;
			break;
			case phaseDir:
			div = ni.dy;
			break;
			case sliceDir:
			div = ni.dz;
			break;
			case voxelSizeVec://read size of voxels pixdim+1 is dx ... pixdim+4=dt
			return util::FixedVector<float,4>(ni.pixdim+1);
			break;
			case centerVec:
				switch(ni.xyz_units){
					case NIFTI_UNITS_METER:div = 1.0/1.0e3;break;
					case NIFTI_UNITS_MICRON:div = 1.0/1.0e-3;break;
				}
			break;
			default:
			break;
		}

		if(ni.nifti_type>0) { // only for non-ANALYZE
			//      RotMatrix scaleMat;//method 2
			util::fvector4 qto,sto;
			//get qto 
			for(int i=0;i<3;i++) {
				qto[i] =ni.qto_xyz.m[i][dir]/div;}
			LOG(ImageIoDebug,info) << "Orientation red from qto_xyz:" << dir+1 << " is " << qto;
			//get sto
			for(int i=0;i<3;i++) {
				sto[i] =ni.sto_xyz.m[i][dir]/div;}
			LOG(ImageIoDebug,info) << "Orientation red from sto_xyz:" << dir+1 << " is " << sto;
			
			//use one of them
			if(ni.qform_code>0){// just tranform to the nominal space of the scanner
				retVec=qto;
				LOG_IF(qto!=sto and ni.sform_code>0,ImageIoLog,warning)
				<< "sto_xyz:" << dir+1 << " (" << sto << ") differs from qto_xyz:" 
				<< dir+1 << " ("<< qto << "). But I'll ignore it anyway because qform_code>0.";
			} else if(ni.sform_code>0){ // method 3
				retVec=sto;
			} else if(ni.sform_code==0 and ni.qform_code==0) {
				retVec=qto;
				LOG(ImageIoLog, info) << "sform_code and qform_code are 0. Trying to use qto_xyz info!";
			} else {
				LOG(ImageIoLog,error)
					<< "can't read orientation Vector for direction: " << dir+1;
			}
		}
		return retVec;
	}

	template<typename T>
	bool copyDataToNifti(const data::Image& image, nifti_image& ni)
	{
		ni.data = malloc(image.bytes_per_voxel()*image.volume());

		data::Image::ChunkIterator it = image.chunksBegin();
		data::ChunkList list(image.chunksBegin(),image.chunksEnd());
			unsigned short i=0;
			BOOST_FOREACH(const data::Chunk &ref,list){

			}

		T *refNii = (T*) ni.data;
		for (int z = 0; z < image.sizeToVector()[2]; z++) {
			for (int y = 0; y < image.sizeToVector()[1]; y++) {
				for (int x = 0; x < image.sizeToVector()[0]; x++) {
					*refNii = image.voxel<T>(x,y,z);
					refNii++;
				}
			}
		}
		ni.nbyper = image.bytes_per_voxel();
		//min / max due to T in image but for nifti everything is float
		T min,max;
		image.getMinMax(min,max);
		ni.cal_min = min;
		ni.cal_max = max;
	}

	bool copyHeaderToNifti(const data::Image& image, nifti_image& ni)
	{
		//all the other information for the nifti header
		ni.scl_slope=1.0;
		ni.scl_inter=0.0;// TODO: ? http://209.85.135.104/search?q=cache:AxBp5gn9GzoJ:nifti.nimh.nih.gov/board/read.php%3Ff%3D1%26i%3D57%26t%3D57+nifti-1+scl_slope&hl=en&ct=clnk&cd=1&client=iceweasel-a
		ni.freq_dim=1;
		ni.phase_dim=2;
		ni.slice_dim=3;
		ni.xyz_units=NIFTI_UNITS_MM;
		ni.time_units=NIFTI_UNITS_MSEC;

		util::fvector4 dimensions = image.sizeToVector();
		LOG(ImageIoLog, info) << dimensions;
		ni.ndim = ni.dim[0] = dimensions[3]>1 ? 4 : 3;
		ni.nx = ni.dim[1] = dimensions[0];
		ni.ny = ni.dim[2] = dimensions[1];
		ni.nz = ni.dim[3] = dimensions[2];
		ni.nt = ni.dim[4] = dimensions[3];

		ni.nvox=image.volume();

		util::fvector4 readVec = image.getProperty<util::fvector4>("readVec");

		util::fvector4 phaseVec = image.getProperty<util::fvector4>("phaseVec");
		util::fvector4 sliceVec = image.getProperty<util::fvector4>("sliceVec");
//		util::fvector4 centerVec = image.getProperty<util::fvector4>("centerVec");
//		LOG(ImageIoLog, info) << centerVec;
		util::fvector4 voxelSizeVector = image.getProperty<util::fvector4>("voxelSize");
		ni.dx = ni.pixdim[1] = voxelSizeVector[0];
		ni.dy = ni.pixdim[2] = voxelSizeVector[1];
		ni.dz = ni.pixdim[3] = voxelSizeVector[2];
		ni.dt = ni.pixdim[4] = voxelSizeVector[3];



		if (true == image.hasProperty("study/description")) {
			std::string descrip = (image.getProperty<std::string>("study/description"));
			snprintf(ni.descrip, 80, "%s", descrip.c_str());}
		if (true == image.hasProperty("series/description")) {
			std::string descrip = (image.getProperty<std::string>("series/description"));
			snprintf(ni.intent_name, 16, "%s", descrip.c_str());}

		//		//special case centervec
		//		util::fvector4 centerVec(0,0,0,1);
		//		util::fvector4 FOV(192, 192, 110, 0);
		//		//diagonale trougth the image in "normal" space
		//		util::fvector4 ivector = readVec*(FOV[readDir]-ni.dx)+
		//				phaseVec*(FOV[phaseDir]-ni.dy)+
		//				sliceVec*(FOV[timeDir]-ni.dz);
		//		centerVec = centerVec + ivector/2;

		//the rotation matrix
		//create space tranformation matrices - transforms the space when reading _NOT_ the data
		//TODO something is going wrong with the rotation matrix, maybe scaling - not clear
		ni.sform_code=ni.qform_code=NIFTI_XFORM_SCANNER_ANAT;//set scanner aligned space from nifti1.h
		LOG(ImageIoLog, info) << "ReadVec " << readVec << "  phaseVec" <<phaseVec << "sliceVec" << sliceVec;
		//util::fvector4 offsets = image.getProperty<util::fvector4>("offsetVec");
		//		ni.qoffset_x = offsets[0];
		//		ni.qoffset_y = offsets[1];
		//		ni.qoffset_z = offsets[2];
		for(int y =0;y<3;y++) {
			ni.qto_xyz.m[0][y]=readVec[y];//rot[0][y];
			ni.qto_xyz.m[1][y]=phaseVec[y];//rot[1][y];
			ni.qto_xyz.m[2][y]=sliceVec[y];//rot[2][y];
//			ni.qto_xyz.m[y][3]=centerVec[y];
		}

		memcpy(ni.sto_xyz.m,ni.qto_xyz.m,sizeof(ni.sto_xyz.m));

		//add scaling to the sform
		for(int y =0;y<3;y++) {
			ni.sto_xyz.m[0][y]*=ni.pixdim[1+y];
			ni.sto_xyz.m[1][y]*=ni.pixdim[1+y];
			ni.sto_xyz.m[2][y]*=ni.pixdim[1+y];
		}

		//generate matching quaternions
		nifti_mat44_to_quatern(
				ni.qto_xyz,
				&ni.quatern_b,&ni.quatern_c,&ni.quatern_d,
				&ni.qoffset_x,&ni.qoffset_y,&ni.qoffset_z,
				NULL,NULL,NULL,
				&ni.qfac);

	}



};//end class definition
}}//namespace image_io isis


isis::image_io::FileFormat* factory()
{
return new isis::image_io::ImageFormat_Nifti();
}
