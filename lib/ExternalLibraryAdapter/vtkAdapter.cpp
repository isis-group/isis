#include "vtkAdapter.hpp"

namespace isis{ namespace adapter{
    
vtkAdapter::vtkAdapter(const boost::shared_ptr<isis::data::Image> src)
    : m_ImageISIS(src), m_vtkImageDataVector()
{}

//return a list of vtkImageData type pointer
vtkAdapter::ImageVector vtkAdapter::makeVtkImageDataList(const boost::shared_ptr<data::Image> src)
{
	vtkAdapter* myAdapter = new vtkAdapter(src);
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
	LOG(DataDebug, info) << "datatype: " << myAdapter->m_ImageISIS->chunksBegin()->typeName();
	//TODO check datatypes	
	importer->SetWholeExtent(0,dimensions[0]-1,0,dimensions[1]-1,0,dimensions[2]-1); //TODO what is exactly defined by the whole extend????????????
	importer->SetDataExtentToWholeExtent();
	vtkImage->SetOrigin(indexOrigin[0],indexOrigin[1],indexOrigin[2]);
	vtkImage->SetSpacing(spacing[0],spacing[1],spacing[2]);
	//go through every timestep (dimensions[3] and add the chunk to the image list 
	for (unsigned int dim=0;dim<dimensions[3];dim++)
	{
		switch(myAdapter->m_ImageISIS->chunksBegin()->typeID()){
			case util::TypePtr<int8_t>::staticID: 
				importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<signed char>(0,0,0,dim));
				vtkImage->SetScalarTypeToSignedChar(); 
				break;
			
			case util::TypePtr<u_int8_t>::staticID: 
				importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<unsigned char>(0,0,0,dim));
				vtkImage->SetScalarTypeToUnsignedChar();
				break;
			
			case util::TypePtr<int16_t>::staticID: 
				importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<signed short>(0,0,0,dim));
				vtkImage->SetScalarTypeToShort();
				break;
				
			case util::TypePtr<u_int16_t>::staticID: 
				vtkImage->SetScalarTypeToUnsignedShort();
				importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<unsigned short>(0,0,0,dim));
				break;
			
			case util::TypePtr<int32_t>::staticID: 
				vtkImage->SetScalarTypeToLong();
				importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<signed long>(0,0,0,dim));
				break;
				
			case util::TypePtr<u_int32_t>::staticID: 
				vtkImage->SetScalarTypeToUnsignedLong();
				importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<unsigned long>(0,0,0,dim));
				break;
				
			case util::TypePtr<float>::staticID: 
				vtkImage->SetScalarTypeToFloat();
				importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<float>(0,0,0,dim));
				break;
			
			case util::TypePtr<double>::staticID: 
				vtkImage->SetScalarTypeToDouble();
				importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<double>(0,0,0,dim));
				break;
		}
		importer->Update();
		vtkImage = importer->GetOutput();
		myAdapter->m_vtkImageDataVector.push_back(vtkImage);
	}
	
	return myAdapter->m_vtkImageDataVector;
}

}} //end namespace