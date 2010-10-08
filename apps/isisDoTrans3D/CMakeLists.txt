ADD_EXECUTABLE(isisdotrans3d isisdotrans3d.cxx isisTransformMerger3D.cpp)
TARGET_LINK_LIBRARIES(isisdotrans3d ITKIO ITKCommon  ${ISIS_LIB} viaio )
