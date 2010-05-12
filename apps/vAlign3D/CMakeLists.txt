ADD_EXECUTABLE(valign3d valign3D.cxx)
TARGET_LINK_LIBRARIES(valign3d  ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio data isisAdapter_itk utils gslcblas)
