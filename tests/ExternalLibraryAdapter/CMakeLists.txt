if(VTK_FOUND)
	add_executable(vtkAdapterTest vtkAdapterTest.cpp)
	target_link_libraries(vtkAdapterTest data utils isisAdapter_vtk vtkRendering)
endif(VTK_FOUND)

	add_executable(itkAdapterTest itkAdapterTest.cpp)
	target_link_libraries(itkAdapterTest data utils isisAdapter_itk ITKCommon ITKIO)
