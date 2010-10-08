ADD_EXECUTABLE(isisdotrans3d isisdotrans3d.cxx)
TARGET_LINK_LIBRARIES(isisdotrans3d ITKIO ITKCommon extitk data utils isisAdapter_itk viaio )
