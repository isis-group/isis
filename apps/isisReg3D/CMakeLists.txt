ADD_EXECUTABLE(isisreg3d isisreg3d.cxx)
TARGET_LINK_LIBRARIES(isisreg3d  ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio data isisAdapter_itk utils)
