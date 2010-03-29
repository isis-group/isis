#include "vtkAdapter.hpp"
#include <vtkXMLImageDataWriter.h>

namespace isis{ namespace adapter{
    
VTKAdapter::VTKAdapter(const boost::shared_ptr<isis::data::Image> src)
    : m_ImageISIS(src), m_vtkImageDataList(), m_vtkImageImportList()
{}

//return a list of vtkImageImport type pointer
std::list< vtkSmartPointer<vtkImageImport> > VTKAdapter::makeVtkImageImportList(const boost::shared_ptr<data::Image> src)
{
  VTKAdapter* myAdapter = new VTKAdapter(src);
  vtkImageImport* importer = vtkImageImport::New();
  const util::fvector4 dimensions(myAdapter->m_ImageISIS->sizeToVector());
   //go through all the chunks and check for consistent datatype
  if(not checkChunkDataType(myAdapter->m_ImageISIS)) { 
    LOG(data::Runtime, error) << "Inconsistent chunk datatype!"; 
    //TODO exception handling
  }
  //TODO handling of 4th dimension, metadata, datatype, amount of chunks
  importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<short>(0,0,0,0));
  importer->SetWholeExtent(0,dimensions[0]-1,0,dimensions[1]-1,0,dimensions[2]-1);
  importer->SetDataExtentToWholeExtent();
  importer->SetDataScalarTypeToShort();
  importer->Update();
    
  myAdapter->m_vtkImageImportList.push_back(importer);
  return myAdapter->m_vtkImageImportList;
}



//return a list of vtkImageData type pointer
std::list< vtkSmartPointer<vtkImageData> > VTKAdapter::makeVtkImageDataList(const boost::shared_ptr<data::Image> src)
{
    VTKAdapter* myAdapter = new VTKAdapter(src);
    vtkImageData* vtkImage = vtkImageData::New();
    vtkImageImport* importer = vtkImageImport::New();
    const util::fvector4 dimensions(myAdapter->m_ImageISIS->sizeToVector());
    
    //go through all the chunks and check for consistent datatype
    if(not checkChunkDataType(myAdapter->m_ImageISIS)) { 
      LOG(data::Runtime, error) << "Inconsistent chunk datatype!"; 
      //TODO exception handling
    }
    
    //set the datatype for the vtkImage object
    LOG(DataDebug, info) << "dim4: " << dimensions[3];
    switch(myAdapter->m_ImageISIS->chunksBegin()->typeID()){
	case util::TypePtr<int8_t>::staticID: vtkImage->SetScalarTypeToChar(); 
	
	case util::TypePtr<u_int8_t>::staticID: vtkImage->SetScalarTypeToUnsignedChar();
	
	case util::TypePtr<int16_t>::staticID: vtkImage->SetScalarTypeToShort();
		
	case util::TypePtr<u_int16_t>::staticID: vtkImage->SetScalarTypeToUnsignedShort();
	
	case util::TypePtr<int32_t>::staticID: vtkImage->SetScalarTypeToLong();
		
	case util::TypePtr<u_int32_t>::staticID: vtkImage->SetScalarTypeToUnsignedLong();
		
	case util::TypePtr<float>::staticID: vtkImage->SetScalarTypeToFloat(); break;
	
	case util::TypePtr<double>::staticID: vtkImage->SetScalarTypeToDouble(); break;
	default:
	    LOG(data::Runtime, error) << "Unknown datatype!";
	    //TODO  exception handle in constructor ??
	}
    LOG(DataDebug, info) << "datatype: " << myAdapter->m_ImageISIS->chunksBegin()->typeName();
    importer->SetImportVoidPointer(&myAdapter->m_ImageISIS->voxel<short>(0,0,0,0));
    importer->SetWholeExtent(0,dimensions[0]-1,0,dimensions[1]-1,0,dimensions[2]-1);
    importer->SetDataExtentToWholeExtent();
    importer->SetDataScalarTypeToShort();
    importer->Update();
    //Set extend (offsetx, x,  offsety, y, offsetz, z);
    //TODO handling of 4th dimension, metadata, datatype, amount of chunks
    vtkImage->SetDimensions(dimensions[0], 
			    dimensions[1],
			    dimensions[2]);
    vtkImage->SetWholeExtent(0,dimensions[0]-1,0,dimensions[1]-1,0,dimensions[2]-1);
    
    vtkImage->SetSpacing(1,1,1);
    vtkImage->SetNumberOfScalarComponents(1);
    vtkImage->SetOrigin(0,0,0);
    vtkImage->AllocateScalars();
    vtkImage->Update();
    vtkImage = importer->GetOutput();
       
  
    myAdapter->m_vtkImageDataList.push_back(vtkImage);
    return myAdapter->m_vtkImageDataList;
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