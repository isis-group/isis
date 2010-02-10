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


namespace isis
{
namespace image_io
{

class ImageFormat_Nifti : public FileFormat
{
public:

	enum vectordirection{readDir=0,phaseDir,timeDir,centerVec,voxelSizeVec};

	std::string suffixes()
	{
		return std::string(".nii.gz .nii .hdr");
	}
	std::string dialects()
	{
		return std::string("fsl");
	}
	std::string name()
	{
		//TODO: wahrscheinlich sollten die Namen irgendwie so aussehen "mpg.cbs.nii"?
		return "Nifti";
	}

	bool tainted()
	{
		return false;//internal plugins are not tainted
	}

//	isis::data::ChunkList load(std::string filename,std::string dialect)
	int load(data::ChunkList &ret,const std::string& filename, const std::string& dialect)
	{
		//read the file with the function from nifti1_io.h
		nifti_image* ni = nifti_image_read(filename.c_str(), true);
		if (NULL == ni){
			return 0;
		}

		//initialize chunkList and read size from ni
		data::ChunkList chunkListFromNifti;

		//copy ni->data to ChunkList
		LOG(ImageIoDebug, isis::util::info)	<< "datatype load from nifti " << ni->datatype;
		//a nifti special scale factor meaning if mm = 1/1000

		// 0.0 would mean "not in use" - so for better handling use a 1.0
		float scale = ni->scl_slope ? ni->scl_slope : 1.0;
		// TODO: at the moment scaling not supported due to data type changes
		if (1.0 != scale){
			LOG(ImageIoDebug, isis::util::error)	<< "Scaling is not supported at the moment. Scale Factor:  " << scale;
			return 0;
		}

		switch(ni->datatype)
		{
		case DT_UINT8:
			break;
		case DT_INT8:
			break;
		case DT_INT16:
			chunkListFromNifti = copyDataFromNii<short>((*ni));
			break;
		case DT_UINT16:
			chunkListFromNifti = copyDataFromNii<unsigned short>((*ni));
			break;
		case DT_UINT32:
			break;
		case DT_INT32:
			break;
		case DT_FLOAT32:
			break;
		case DT_FLOAT64:
			break;
		default:
			LOG(ImageIoDebug, isis::util::error) << "Unsupported datatype " << ni->datatype;
			return 0;
			break;
		}

		LOG(ImageIoDebug, isis::util::info) << "datatype load from nifti " << ni->datatype;
		//clean up the function from nifti1_io.h
		nifti_image_free(ni);
		std::copy(chunkListFromNifti.begin(),chunkListFromNifti.end(),ret.end());
		return chunkListFromNifti.size();
	}

	size_t maxDim() {
		return 4; //TODO
	}

	bool write(const data::Image &image, const std::string& filename, const std::string& dialect)

	{

		LOG(ImageIoLog, isis::util::info) << "Write Nifti.";

		//default init for nifti image
		nifti_image ni;
		memset(&ni,0, sizeof(nifti_image)); //set everything to zero - default value for "not used"
		ni.nu=ni.nv=ni.nw=1;
		ni.datatype = DT_UNKNOWN;
		ni.data = NULL;
		ni.fname=const_cast<char*>(filename.c_str());

		// get info from image
		util::fvector4 dimensions = image.size();

		copyHeaderToNifti(image, ni);
//		copyDataToNifti(image, ni);
		ni.iname=(char*)filename.c_str();
		ni.fname=(char*)filename.c_str();

		LOG(ImageIoLog, isis::util::info) << image.getChunk(0,0,0,0)->typeName();

		if ("s8bit*" == image.getChunk(0,0,0,0)->typeName()){
			ni.datatype = DT_INT8;}
		else if ("u8bit*" == image.getChunk(0,0,0,0)->typeName()){
			ni.datatype = DT_UINT8;}
		else if ("s16bit*" == image.getChunk(0,0,0,0)->typeName()){
			ni.datatype = DT_INT16;
			copyDataToNifti<signed short>(image, ni);}
		else if ("u16bit*" == image.getChunk(0,0,0,0)->typeName()){
			ni.datatype = DT_UINT16;
			copyDataToNifti<unsigned short>(image, ni);}
		else if ("s32bit*" == image.getChunk(0,0,0,0)->typeName()){
			ni.datatype = DT_INT32;}
		else if ("u32bit*" == image.getChunk(0,0,0,0)->typeName()){
			ni.datatype = DT_UINT32;}
		else if ("float*" == image.getChunk(0,0,0,0)->typeName()){
			ni.datatype = DT_FLOAT32;}
		else if ("double*" == image.getChunk(0,0,0,0)->typeName()){
			ni.datatype = DT_FLOAT64;}
		else{
			LOG(ImageIoLog, isis::util::error) << "Nifti write: Datatype cannot be written!";}


		if(strcasecmp(dialect.c_str(), "fsl")) {
			//			bool do_transform=false;
			//			std::string sppart="s,p";
			//			if(geo.get_orientation()==sagittal || geo.get_orientation()==coronal) {
			//
			//				ODINLOG(odinlog,infoLog) << "Rotating around read axis for fsl dialect" << STD_endl;
			//				sppart="p-,s";
			//				do_transform=true;
			//			}
			//			STD_string rsign;
			//			double heightAng, azimutAng, inplaneAng;
			//			bool revSlice;
			//			geo.get_orientation(heightAng, azimutAng, inplaneAng, revSlice);
			//			if(!revSlice) { // FSL needs 'radiological' handness
			//				ODINLOG(odinlog,infoLog) << "Reversing handness for fsl dialect" << STD_endl;
			//				rsign="-";
			//				do_transform=true;
		}



	    //now really write the nifti file with the function from nifti1_io.h
	    errno=0; //reset errno
		nifti_image_write(&ni); //write the image - in case of a failure errno should be set
		if(errno) {
			//not succesfully written
			LOG(ImageIoLog, util::error) << "Could not write to "<< filename << "(" <<strerror(errno) << ")";
			boost::filesystem::remove(boost::filesystem::path(filename));
			return false;
		}

		return true; // everything seems to be fine
	}

	//============================= ACCESS     ===================================
	//============================= INQUIRY    ===================================
	/////////////////////////////// PROTECTED  ///////////////////////////////////

	/////////////////////////////// PRIVATE    ///////////////////////////////////
private:

	template<typename T>
	 data::ChunkList copyDataFromNii(const nifti_image& ni)
	{
		util::fvector4 dimensions(ni.dim[1], ni.ndim >= 2 ? ni.dim[2] : 1,
				ni.ndim >= 3 ? ni.dim[3] : 1, ni.ndim >= 4 ? ni.dim[4] : 1);
		LOG(ImageIoLog, isis::util::info)<< "size of chunk " << dimensions << "/" << ni.ndim;

		T minValue = 0;
    	T maxValue = 0;

		data::ChunkList chunkList;
		for (int t = 0; t < dimensions[3]; t++){
			//init MemChunk of known type and size
			//TODO datatype, better say not copying voxelwise
			data::MemChunk<unsigned short> ch(dimensions[0], dimensions[1], dimensions[2], dimensions[3]);
			util::fvector4 offsets(ni.qoffset_x, ni.qoffset_y, ni.qoffset_z, 0);

			//ch.setProperty("acquisitionTime", );
			ch.setProperty("indexOrigin", util::fvector4(ni.qoffset_x, ni.qoffset_y, ni.qoffset_z, 0));
			ch.setProperty("acquisitionNumber", t);
			ch.setProperty("sequenceNumber", 1);
			ch.setProperty("readVec", getVector(ni, readDir));
			ch.setProperty("phaseVec", getVector(ni, phaseDir));
			ch.setProperty("sliceVec", getVector(ni, timeDir));
			ch.setProperty("voxelSize",getVector(ni, voxelSizeVec));
			ch.setProperty("centerVec",getVector(ni, centerVec));
			ch.setProperty("offsetVec", offsets);
			ch.setProperty<std::string>("study/description", std::string(ni.descrip) );
			ch.setProperty<std::string>("series/description", std::string(ni.intent_name) );

			//just some LOGS
			LOG(ImageIoLog, util::info) << "dims at all " << dimensions;
			LOG(ImageIoLog, util::info) << "Offset values from nifti" << offsets;
			LOG(ImageIoLog,isis::util::info) << "FOV read/phase/slice/voxelsize:"
			<< getVector(ni, readDir)
			<< " / " << getVector(ni, phaseDir)
			<< " / " << getVector(ni, timeDir)
			<< getVector(ni, voxelSizeVec);

			T *pNiftiData = static_cast<T*> (ni.data);


			for(int z = 0; z < dimensions[2]; z++) {
				for(int y = 0; y < dimensions[1]; y++) {
					for(int x = 0; x < dimensions[0]; x++) {
						T val = ((*pNiftiData)); // attention to eventually scaling support
						ch.voxel<T>(x, y, z, t) = val;
						pNiftiData++;
						if (val < minValue){
							minValue = val;}
						if (val > maxValue){
							maxValue = val;}
					}
				}
			}

			std::cout << static_cast<util::PropMap>(ch)<< std::endl;
			chunkList.push_back(ch);
		}
		return chunkList;
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
		case timeDir:
			div = ni.dz;
			break;
		case voxelSizeVec://read size of voxels
			return isis::util::fvector4(ni.pixdim[1], ni.pixdim[2], ni.pixdim[3], ni.pixdim[4]);
			break;
		case centerVec:
			if(ni.xyz_units==NIFTI_UNITS_METER){
				div = 1.0/1.0e3;}
			if(ni.xyz_units==NIFTI_UNITS_MICRON){
				div = 1.0/1.0e-3;}
			break;
		default:
			break;
		}

		if(ni.nifti_type>0) { // only for non-ANALYZE
			//      RotMatrix scaleMat;//method 2
			if(ni.qform_code>0) {// just tranform to the nominal space of the scanner
				LOG(ImageIoLog,isis::util::info) << "Reading orientation from qform";
				for(int i=0;i<3;i++) {
					retVec[i]  =ni.qto_xyz.m[i][dir]/div;}
				} else if(ni.sform_code>0) { // method 3
					LOG(ImageIoLog,isis::util::info) << "Reading orientation from sform";
					for(int i=0;i<3;i++) {
						retVec[i]  =ni.sto_xyz.m[i][dir]/div;}
					} else {
						LOG(ImageIoLog,isis::util::error)
								<< "can't read orientation Vector from direction: " << dir ;
						return util::fvector4();}
			}
		return retVec;
	}



	template<typename T>
	bool copyDataToNifti(const data::Image& image, nifti_image& ni)
	{
		ni.data = malloc(sizeof(T)*image.size()[0]*image.size()[1]*image.size()[2]*image.size()[3]);
		T *refNii = (T*) ni.data;
		for (int z = 0; z < image.size()[2]; z++){
			for (int y = 0; y < image.size()[1]; y++){
				for (int x = 0; x < image.size()[0]; x++){
					*refNii = image.voxel<T>(x,y,z);
					refNii++;
				}
			}
		}
		ni.nbyper = sizeof(T);
		//min / max due to T in image but for nifti everything is float
		ni.cal_min = 0.0;
		ni.cal_max = 0.0;
		if ( true == image.hasProperty("minValue") ){
			ni.cal_min = static_cast<float> (image.getProperty<T>("minValue"));}
		if ( true == image.hasProperty("maxValue") ){
			ni.cal_max = static_cast<float> (image.getProperty<T>("maxValue"));}


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

		util::fvector4 dimensions = image.size();
		LOG(ImageIoLog, util::info) << dimensions;
		ni.ndim = ni.dim[0] = dimensions[3]>1 ? 4 : 3;
		ni.nx = ni.dim[1] = dimensions[0];
		ni.ny = ni.dim[2] = dimensions[1];
		ni.nz = ni.dim[3] = dimensions[2];
		ni.nt = ni.dim[4] = dimensions[3];


		ni.nvox=image.volume();

		util::fvector4 readVec  = image.getProperty<util::fvector4>("readVec");

		util::fvector4 phaseVec = image.getProperty<util::fvector4>("phaseVec");
		util::fvector4 sliceVec = image.getProperty<util::fvector4>("sliceVec");
		util::fvector4 centerVec = image.getProperty<util::fvector4>("centerVec");
		LOG(ImageIoLog, util::info) << centerVec;
		util::fvector4 voxelSizeVector = image.getProperty<util::fvector4>("voxelSize");
		ni.dx = ni.pixdim[1] = voxelSizeVector[0];
		ni.dy = ni.pixdim[2] = voxelSizeVector[1];
		ni.dz = ni.pixdim[3] = voxelSizeVector[2];
		ni.dt = ni.pixdim[4] = voxelSizeVector[3];

		if (true == image.hasProperty("study/description")){
			std::string descrip = (image.getProperty<std::string>("study/description"));
			snprintf(ni.descrip, 80, "%s", descrip.c_str());}
		if (true == image.hasProperty("series/description")){
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
		LOG(ImageIoLog, util::info) << "ReadVec " << readVec * ni.pixdim[1] << "  phaseVec" <<phaseVec * ni.pixdim[2]<< "sliceVec"  << sliceVec* ni.pixdim[3];
		util::fvector4 offsets = image.getProperty<util::fvector4>("offsetVec");
//		ni.qoffset_x = offsets[0];
//		ni.qoffset_y = offsets[1];
//		ni.qoffset_z = offsets[2];
		for(int y =0;y<3;y++) {
			ni.qto_xyz.m[0][y]=readVec[y];//rot[0][y];
			ni.qto_xyz.m[1][y]=phaseVec[y];//rot[1][y];
			ni.qto_xyz.m[2][y]=sliceVec[y];//rot[2][y];
			ni.qto_xyz.m[y][3]=centerVec[y];
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


		//TODO: ANALYZE/NIFTI
		ni.nifti_type = 1;
	}

};//end class definition
}
}//namespace image_io isis


isis::image_io::FileFormat* factory()
{
	return new isis::image_io::ImageFormat_Nifti();
}
