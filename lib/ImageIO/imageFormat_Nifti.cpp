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
 * Author: Lydia Hellrung, hellrung@cbs.mppg.de, 2009
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
#include "common.hpp"

//SYSTEM INCLUDES
#include <nifti1_io.h>


namespace isis
{
namespace image_io
{

class ImageFormat_Nifti : public FileFormat
{
public:

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

	isis::data::ChunkList load(std::string filename, std::string dialect)
	{

		nifti_image* ni = nifti_image_read(filename.c_str(), true);
		if (NULL == ni)
		{
			return isis::data::ChunkList();
		}

		data::ChunkList chunkListFromNifti;
		util::fvector4 dimensions(ni->dim[1], ni->ndim >= 2 ? ni->dim[2] : 1, ni->ndim >= 3 ? ni->dim[3] : 1, ni->ndim
		        >= 4 ? ni->dim[4] : 1);
		LOG(ImageIoLog, isis::util::info)
		<<	"size of chunk " << dimensions << "/" << ni->ndim << std::endl;

		//copy ni->data to ChunkList
		switch(ni->datatype) {
			case DT_UINT8:
			break;
			case DT_INT8:
			break;
			case DT_UINT16:
			for(int t = 0; t < dimensions[3]; t++) {
				//a nifti special scale factor meaning if mm = 1/1000
				float scale = ni->scl_slope?:1;
				LOG(ImageIoLog, util::info)
				<< "scale " << scale;
				data::MemChunk<float> ch(dimensions[0], dimensions[1], dimensions[2]);
				LOG(ImageIoLog, util::info)
				<< "dims at all " << dimensions;
				util::fvector4 offsets(ni->qoffset_x, ni->qoffset_y, ni->qoffset_z, 0);
				LOG(ImageIoLog, util::info)
				<< "Offset values from nifti" << offsets << std::endl;
				ch.setProperty("indexOrigin", util::fvector4(ni->qoffset_x, ni->qoffset_y, ni->qoffset_z, 0));
				//TODO
				//ch.setProperty("acquisitionTime", );
				ch.setProperty("acquisitionNumber", t);
				ch.setProperty("sequenceNumber", 1);
				ch.setProperty("readVec",readVector);
				ch.setProperty("phaseVec",phaseVector);
				ch.setProperty("voxelSize",voxelSizeVector);

				unsigned short *pNiftiData = (unsigned short*) ni->data;
				for(int z = 0; z < dimensions[2]; z++) {
					for(int y = 0; y < dimensions[1]; y++) {
						for(int x = 0; x < dimensions[0]; x++) {
							ch.voxel<float> (x, y, z) = (scale * (*pNiftiData));
							pNiftiData++;
						}
					}
				}
				chunkListFromNifti.push_back(ch);
			}
			case DT_INT16:
			//
			for(int t = 0; t < dimensions[3]; t++) {
				//a nifti special scale factor meaning if mm = 1/1000
				float scale = ni->scl_slope;
				LOG(ImageIoLog, util::info)
				<< "scale " << scale;
				data::MemChunk<float> ch(dimensions[0], dimensions[1], dimensions[2]);
				LOG(ImageIoLog, util::info)
				<< "dims at all " << dimensions;
				util::fvector4 offsets(ni->qoffset_x, ni->qoffset_y, ni->qoffset_z, 0);
				LOG(ImageIoLog, util::info)
				<< "Offset values from nifti" << offsets << std::endl;
				ch.setProperty("indexOrigin", util::fvector4(ni->qoffset_x, ni->qoffset_y, ni->qoffset_z, 0));
				//TODO
				//ch.setProperty("acquisitionTime", );
				ch.setProperty("acquisitionNumber", t);
				ch.setProperty("sequenceNumber", 1);
				ch.setProperty("readVec",readVector);
				ch.setProperty("phaseVec",phaseVector);
				ch.setProperty("voxelSize", voxelSizeVector);

				signed short *pNiftiData = (short*) ni->data;
				for(int z = 0; z < dimensions[2]; z++) {
					for(int y = 0; y < dimensions[1]; y++) {
						for(int x = 0; x < dimensions[0]; x++) {
							ch.voxel<float> (x, y, z) = (scale * (*pNiftiData));
							pNiftiData++;
						}
					}
				}
				chunkListFromNifti.push_back(ch);
			}
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
			LOG(ImageIoDebug, isis::util::error)
			<< "Unsupported datatype " << ni->datatype << std::endl;
			return data::ChunkList();
			break;
		}
		LOG(ImageIoDebug, isis::util::info)
		<< "datatype load from nifti " << ni->datatype << std::endl;

		nifti_image_free(ni);
		return chunkListFromNifti;

	}

	bool save(
			const isis::data::ChunkList& chunks, std::string filename, std::string dialect) {
		return false;
	}

	size_t maxDim() {
		return 4; //TODO
	}

	bool write(
			const data::Image &image, std::string filename, std::string dialect) {

		util::fvector4 dimensions = image.size();
		//		LOG(ImageIoDebug, util::info) << dimensions;
		//		for (int t = 0; t < 1; t++){//erster Zeitschritt
		//			for (int z = 0; z < 1; z++){//erste Schicht
		//				for (int y = 0; y < dimensions[1]; y++){//alle Voxel
		//					for (int x = 0; x < dimensions[0];x++)
		//					{
		//						std::cout <<  image.voxel<float>(x,y,z,t) << " ";
		//					}
		//					std::cout << std::endl;
		//				}
		//			}
		//		}


		nifti_image ni;
		memset(&ni,0, sizeof(nifti_image)); //set everything to zero - default value for "not used"
		ni.nu=ni.nv=ni.nw=1;

		return true; //TODO
	}

	//============================= ACCESS     ===================================
	//============================= INQUIRY    ===================================
	/////////////////////////////// PROTECTED  ///////////////////////////////////

	/////////////////////////////// PRIVATE    ///////////////////////////////////
private:
	bool readOrientation(const nifti_image& ni)
	{
		 float spatscale=1.0;
		 if (ni.xyz_units==NIFTI_UNITS_METER) {
			 spatscale=1.0e3;}
		 if (ni.xyz_units==NIFTI_UNITS_MICRON){
			 spatscale=1.0e-3;}

		 //read size of voxels

		 voxelSizeVector = isis::util::fvector4(ni.pixdim[1], ni.pixdim[2], ni.pixdim[3]);


		if(ni.nifti_type>0) { // only for non-ANALYZE

			//      RotMatrix scaleMat;//method 2
			if(ni.qform_code>0) {// just tranform to the nominal space of the scanner
				LOG(ImageIoLog,isis::util::info) << "Reading orientation from qform";

				for(unsigned short i=0;i<3;i++) {
					readVector[i] =ni.qto_xyz.m[i][0]/ni.dx;
					phaseVector[i] =ni.qto_xyz.m[i][1]/ni.dy;
					sliceVector[i] =ni.qto_xyz.m[i][2]/ni.dz;
					centerVector[i]=ni.qto_xyz.m[i][3]*spatscale;
				}
			} else if(ni.sform_code>0) { // method 3
				LOG(ImageIoLog,isis::util::info) << "Reading orientation from sform";
				for(unsigned short i=0;i<3;i++) {
					readVector[i] =ni.sto_xyz.m[i][0]/ni.dx;
					sliceVector[i] =ni.sto_xyz.m[i][1]/ni.dy;
					sliceVector[i] =ni.sto_xyz.m[i][2]/ni.dz;
					centerVector[i]=ni.sto_xyz.m[i][3]*spatscale;
				}
			} else {
				LOG(ImageIoLog,isis::util::error) << "can't read Orientation";
				return false;
			}
			//TODO
//			const dvector ivector =//diagonale trougth the image in "normal" space
//			(geometry.get_FOV(readDirection)-ni.dx)*readvec+
//			(geometry.get_FOV(phaseDirection)-ni.dy)*phasevec+
//			(geometry.get_FOV(sliceDirection)-ni.dz)*slicevec;
//			centervec+=ivector/2;

			LOG(ImageIoLog,isis::util::info) << "FOV read/phase/slice:"<< readVector << " / " << phaseVector << " / " << sliceVector;
			//LOG(ImageIoLog,isis::util::info) << "image diagonale "<< ivector.printbody() <<"/" << sqrt(ivector[0]*ivector[0]+ivector[1]*ivector[1]+ivector[2]*ivector[2])<< STD_endl;
			//LOG(ImageIoLog,isis::util::info) << "center " << centerVector;
			//		@todo should be set here
			// 		for(unsigned short i=0;i<3;i++)
			// 			scaleMat[i][i]=1/ni.pixdim[i+1];

			//LOG(ImageIoLog,isis::util::info) << "set up gradrot matrix ";
			LOG(ImageIoLog,isis::util::info) << "set up center (offset) " << centerVector;
			return true;
		}
	}
	private:

	//bool readOrientation(const nifti_image& ni);

	isis::util::fvector4 readVector;
	isis::util::fvector4 phaseVector;
	isis::util::fvector4 sliceVector;
	isis::util::fvector4 centerVector;
	isis::util::fvector4 voxelSizeVector;


};//end class definition
}
}//namespace image_io isis


isis::image_io::FileFormat* factory()
{
	return new isis::image_io::ImageFormat_Nifti();
}
