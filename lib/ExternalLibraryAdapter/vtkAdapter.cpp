#include "vtkAdapter.hpp"
#include <vtkXMLImageDataWriter.h>

namespace isis{ namespace adapter{
    
VTKAdapter::VTKAdapter(const boost::shared_ptr<isis::data::Image> src)
    : m_ImageISIS(src), m_vtkImageDataVector()
{}


//return a list of vtkImageData type pointer
VTKAdapter::ImageVector VTKAdapter::makeVtkImageDataList(const boost::shared_ptr<data::Image> src, const VTKAdapter::ChunkArrangement& arrangement)
{
	VTKAdapter* myAdapter = new VTKAdapter(src);
	vtkImageData* vtkImage = vtkImageData::New();
	vtkImageImport* importer = vtkImageImport::New();
	const util::fvector4 dimensions(myAdapter->m_ImageISIS->sizeToVector());
	const util::fvector4 indexOrigin(myAdapter->m_ImageISIS->getProperty<util::fvector4>("indexOrigin"));
	const util::fvector4 spacing(myAdapter->m_ImageISIS->getProperty<util::fvector4>("voxelSize"));
	LOG(DataDebug, info) << dimensions;
	//go through all the chunks and check for consistent datatype
	if(not checkChunkDataType(myAdapter->m_ImageISIS)) { 
		LOG(data::Runtime, error) << "Inconsistent chunk datatype!"; 
		//TODO exception handling
	}
	
	//set the datatype for the vtkImage object
	LOG(DataDebug, info) << "dim4: " << dimensions[3];
	LOG(DataDebug, info) << "datatype: " << myAdapter->m_ImageISIS->chunksBegin()->typeName();
	//TODO check datatypes
	switch(myAdapter->m_ImageISIS->chunksBegin()->typeID()){
		case util::TypePtr<int8_t>::staticID: vtkImage->SetScalarTypeToChar(); importer->SetDataScalarTypeToUnsignedChar();break;
		
		case util::TypePtr<u_int8_t>::staticID: vtkImage->SetScalarTypeToUnsignedChar(); importer->SetDataScalarTypeToUnsignedChar();break;
		
		case util::TypePtr<int16_t>::staticID: vtkImage->SetScalarTypeToShort();importer->SetDataScalarTypeToShort();break;
			
		case util::TypePtr<u_int16_t>::staticID: vtkImage->SetScalarTypeToUnsignedShort();importer->SetDataScalarTypeToUnsignedShort();break;
		
		case util::TypePtr<int32_t>::staticID: vtkImage->SetScalarTypeToLong();importer->SetDataScalarTypeToInt();break;
			
		case util::TypePtr<u_int32_t>::staticID: vtkImage->SetScalarTypeToUnsignedLong();importer->SetDataScalarTypeToInt();break;
			
		case util::TypePtr<float>::staticID: vtkImage->SetScalarTypeToFloat();importer->SetDataScalarTypeToFloat(); break;
		
		case util::TypePtr<double>::staticID: vtkImage->SetScalarTypeToDouble();importer->SetDataScalarTypeToDouble(); break;
		default:
			LOG(data::Runtime, error) << "Unknown datatype!";
			//TODO  exception handle in constructor ??
	}
	
	importer->SetWholeExtent(0,dimensions[0]-1,0,dimensions[1]-1,0,dimensions[2]-1);
	importer->SetDataExtentToWholeExtent();
	vtkImage->SetOrigin(indexOrigin[0],indexOrigin[1],indexOrigin[2]);
	vtkImage->SetSpacing(spacing[0],spacing[1],spacing[2]);
	//go through every timestep (dimensions[3] and add the chunk to the image list 
	for (unsigned int dim=0;dim<dimensions[3];dim++)
	{
		importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<short>(0,0,0,dim));
		importer->Update();
		vtkImage = importer->GetOutput();
		myAdapter->m_vtkImageDataVector.push_back(vtkImage);
	}
	
	return myAdapter->m_vtkImageDataVector;
}


//private functions

bool VTKAdapter::checkChunkDataType(const boost::shared_ptr<data::Image> image)
{
	unsigned int firstTypeID = image->chunksBegin()->typeID();
	unsigned int chunkCounter = 0;
	for (data::Image::ChunkIterator ci = image->chunksBegin();ci != image->chunksEnd(); *ci++)
	{
		chunkCounter++;
		if(not ci->typeID() == firstTypeID) return false;   
	}
	LOG(DataDebug, info) << "chunkCounter: " << chunkCounter;
	return true;
}


}} //end namespace