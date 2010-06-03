ADD_EXECUTABLE(valign2d valign2D.cxx)
TARGET_LINK_LIBRARIES(valign2d  ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio utils data isisAdapter_itk )
