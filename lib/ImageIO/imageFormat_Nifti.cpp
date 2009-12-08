/*
 * ImageFormatNii.cpp
 *
 *  Created on: Aug 12, 2009
 *      Author: hellrung
 */

//LOCAL INCLUDES
#include <DataStorage/io_interface.h>
#include "common.hpp"

//SYSTEM INCLUDES
#include <nifti1_io.h>

/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= OPERATIONS ===================================
namespace isis {
namespace image_io {

class ImageFormat_Nifti : public FileFormat
{
public:

	std::string suffixes() {
		return std::string(".nii.gz .nii .hdr");
	}
	std::string dialects() {
		return std::string("fsl");
	}
	std::string name() {
		//TODO: wahrscheinlich sollten die Namen irgendwie so aussehen "mpg.cbs.nii"?
		return "Nifti";
	}

	bool tainted() {
		return false;//internal plugins are not tainted
	}

	isis::data::ChunkList load(
	    std::string filename, std::string dialect) {

		nifti_image* ni = nifti_image_read(filename.c_str(), true);
		data::ChunkList chunkListFromNifti;
		util::fvector4 dimensions(ni->dim[1], ni->ndim >= 2 ? ni->dim[2] : 1, ni->ndim >= 3 ? ni->dim[3] : 1, ni->ndim
		        >= 4 ? ni->dim[4] : 1);
		LOG(ImageIoLog, isis::util::info)
			<< "size of chunk " << dimensions << "/" << ni->ndim << std::endl;

		//copy ni->data to ChunkList
		switch(ni->datatype) {
		case DT_UINT8:
			break;
		case DT_INT8:
			break;
		case DT_UINT16:
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

		return true; //TODO
	}

	//============================= ACCESS     ===================================
	//============================= INQUIRY    ===================================
	/////////////////////////////// PROTECTED  ///////////////////////////////////

	/////////////////////////////// PRIVATE    ///////////////////////////////////
private:

};//end class definition
}
}//namespace image_io isis


isis::image_io::FileFormat* factory() {
	return new isis::image_io::ImageFormat_Nifti();
}
