#include "vtkAdapter.hpp"
#include "vtkImageData.h"

namespace isis{ namespace adapter{
    
VTKAdapter::VTKAdapter()
{}

std::list<VTKAdapter::Superclass*> VTKAdapter::makeVtkImageList(const boost::shared_ptr<isis::data::Image> src)
{
    m_ImageISIS = src;
    vtkImageData* vtkImage = vtkImageData::New();
    const util::fvector4 dimensions(m_ImageISIS->sizeToVector());
    Superclass* importer = VTKAdapter::New();
    std::list<Superclass*> imageList;
    unsigned int firstTypeID = m_ImageISIS->chunksBegin()->typeID();
    unsigned int chunkCounter = 0;
    //go through all the chunks and check for consistent datatype
    for (data::Image::ChunkIterator ci = m_ImageISIS->chunksBegin();ci != m_ImageISIS->chunksEnd(); *ci++)
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
    switch(m_ImageISIS->chunksBegin()->typeID()){
	case util::TypePtr<int8_t>::staticID: importer->SetDataScalarTypeToUnsignedChar(); break;
	case util::TypePtr<u_int8_t>::staticID: importer->SetDataScalarTypeToUnsignedChar(); break;
	
	case util::TypePtr<int16_t>::staticID: importer->SetDataScalarTypeToShort(); break;
	case util::TypePtr<u_int16_t>::staticID: importer->SetDataScalarTypeToUnsignedShort(); break;
	
	case util::TypePtr<int32_t>::staticID: importer->SetDataScalarTypeToDouble(); break;
	case util::TypePtr<u_int32_t>::staticID: importer->SetDataScalarTypeToDouble(); break;
	
	case util::TypePtr<float>::staticID: importer->SetDataScalarTypeToFloat(); break;
	case util::TypePtr<double>::staticID: importer->SetDataScalarTypeToDouble(); break;
	default:
	    LOG(data::Runtime, error) << "Unknown datatype!";
	    //TODO  exception handle in constructor ??
	}
    LOG(DataDebug, info) << "datatype: " << m_ImageISIS->chunksBegin()->typeName();
    //Set extend (offsetx, x,  offsety, y, offsetz, z)

    importer->SetWholeExtent(0,dimensions[0]-1,0,dimensions[1]-1,0,dimensions[2]-1);
    importer->SetImportVoidPointer(m_ImageISIS->chunksBegin()->get());
    
    imageList.push_back(importer);
    return imageList;
}

}} //end namespace