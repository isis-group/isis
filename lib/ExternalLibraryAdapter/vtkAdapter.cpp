#include "vtkAdapter.hpp"
#include "vtkImageData.h"
#include "vtkImageImport.h"

namespace isis{ namespace adapter{
    
VTKAdapter::VTKAdapter(const boost::shared_ptr<isis::data::Image> src)
    : m_ImageISIS(src), m_vtkImageList()
{}


std::list<vtkImageData*> VTKAdapter::makeVtkImageList(const boost::shared_ptr<isis::data::Image> src)
{
    VTKAdapter* myAdapter = new VTKAdapter(src);
    vtkImageData* vtkImage = vtkImageData::New();
    vtkImageImport* importer = vtkImageImport::New();
    const util::fvector4 dimensions(myAdapter->m_ImageISIS->sizeToVector());
    unsigned int firstTypeID = myAdapter->m_ImageISIS->chunksBegin()->typeID();
    unsigned int chunkCounter = 0;
    //go through all the chunks and check for consistent datatype
    for (data::Image::ChunkIterator ci = myAdapter->m_ImageISIS->chunksBegin();ci != myAdapter->m_ImageISIS->chunksEnd(); *ci++)
    {
	chunkCounter++;
	if(not ci->typeID() == firstTypeID)
	{
	    LOG(data::Runtime, error) << "Inconsistent chunk datatype!";
	    //TODO exception handle in constructor ??
	}
    }
    LOG(DataDebug, info) << "chunkCounter: " << chunkCounter;
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
    //Set extend (offsetx, x,  offsety, y, offsetz, z);
    vtkImage->SetDimensions(myAdapter->m_ImageISIS->chunksBegin()->sizeToVector()[0], 
			    myAdapter->m_ImageISIS->chunksBegin()->sizeToVector()[1],
			    1);
    vtkImage->SetSpacing(1,1,1);
    vtkImage->SetNumberOfScalarComponents(1);
    vtkImage->SetOrigin(0,0,0);
    vtkImage->AllocateScalars();
    short* ptr = static_cast<short*>(vtkImage->GetScalarPointer());
    
    for (unsigned int x = 0; x <  myAdapter->m_ImageISIS->chunksBegin()->sizeToVector()[0]; x++)
	{
	    for (unsigned int y = 0; y <  myAdapter->m_ImageISIS->chunksBegin()->sizeToVector()[1]; y++)
	    {
	      
		*ptr++ =  myAdapter->m_ImageISIS->chunksBegin()->voxel<short>(x,y,100,0);
		std::cout << x << "," << y << ":" << *ptr << "_";

	    }
	}
    
    myAdapter->m_vtkImageList.push_back(vtkImage);
    return myAdapter->m_vtkImageList;
}




}} //end namespace