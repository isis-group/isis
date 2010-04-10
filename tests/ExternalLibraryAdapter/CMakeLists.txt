if(HAVE_VTK)
	add_executable(vtkAdapterTest vtkAdapterTest.cpp)
	target_link_libraries(vtkAdapterTest data utils vtkAdapter vtkRendering)
endif(HAVE_VTK)

if(HAVE_ITK)
	add_executable(itkAdapterTest itkAdapterTest.cpp)
	target_link_libraries(itkAdapterTest data utils itkAdapter ITKCommon)
endif(HAVE_ITK)