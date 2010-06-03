ADD_EXECUTABLE(vdotrans2d vdotrans2D.cxx)
TARGET_LINK_LIBRARIES(vdotrans2d  ITKIO ITKCommon extitk data utils isisAdapter_itk viaio)
