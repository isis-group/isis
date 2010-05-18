ADD_EXECUTABLE(vdotrans3d vdotrans3D.cxx)
TARGET_LINK_LIBRARIES(vdotrans3d  ITKIO ITKCommon extitk data utils isisAdapter_itk viaio gslcblas)
