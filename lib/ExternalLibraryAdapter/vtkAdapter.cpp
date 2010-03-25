#include "vtkAdapter.hpp"
#include "vtkImageData.h"

namespace isis{ namespace adapter{
    
VTKAdapter::VTKAdapter(const boost::shared_ptr<isis::data::Image> src)
    : m_ImageISIS(src), m_vtkImageList()
{}


std::list<vtkImageData*> VTKAdapter::makeVtkImageList(const boost::shared_ptr<isis::data::Image> src)
{
    VTKAdapter* myAdapter = new VTKAdapter(src);
    vtkImageData* vtkImage = vtkImageData::New();
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
	case util::TypePtr<int8_t>::staticID: vtkImage->SetScalarTypeToChar(); break;
	case util::TypePtr<u_int8_t>::staticID: vtkImage->SetScalarTypeToUnsignedChar(); break;
	
	case util::TypePtr<int16_t>::staticID: vtkImage->SetScalarTypeToShort(); break;
	case util::TypePtr<u_int16_t>::staticID: vtkImage->SetScalarTypeToUnsignedShort(); break;
	
	case util::TypePtr<int32_t>::staticID: vtkImage->SetScalarTypeToLong(); break;
	case util::TypePtr<u_int32_t>::staticID: vtkImage->SetScalarTypeToUnsignedLong(); break;
	
	case util::TypePtr<float>::staticID: vtkImage->SetScalarTypeToFloat(); break;
	case util::TypePtr<double>::staticID: vtkImage->SetScalarTypeToDouble(); break;
	default:
	    LOG(data::Runtime, error) << "Unknown datatype!";
	    //TODO  exception handle in constructor ??
	}
    LOG(DataDebug, info) << "datatype: " << myAdapter->m_ImageISIS->chunksBegin()->typeName();
    //Set extend (offsetx, x,  offsety, y, offsetz, z)
//test purpose
    vtkImage->SetScalarTypeToFloat();
    vtkImage->SetDimensions(256, 256, 1);
    vtkImage->AllocateScalars();
    float *ptr = static_cast<float*>(vtkImage->GetScalarPointer());

    for (unsigned int y = 0; y < 256; ++y)
    {
	for (unsigned int x = 0; x < 256; ++x)
	{
	    *ptr++ =10.0 * sin(0.1 * x) * sin(0.1 * y);

	}
    }

    myAdapter->m_vtkImageList.push_back(vtkImage);
    return myAdapter->m_vtkImageList;
}




}} //end namespace