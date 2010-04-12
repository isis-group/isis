if(VTK_FOUND)
	add_executable(vtkAdapterTest vtkAdapterTest.cpp)
	target_link_libraries(vtkAdapterTest data utils vtkAdapter vtkRendering)
endif(VTK_FOUND)

if(ITK_FOUND)
	add_executable(itkAdapterTest itkAdapterTest.cpp)
	target_link_libraries(itkAdapterTest data utils itkAdapter ITKCommon ITKIO)
endif(ITK_FOUND)