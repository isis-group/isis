
IF(ITK_FOUND)
	add_library(isisAdapter_itk itkAdapter.hpp)
	# use add_library with hpp files as well
	set_target_properties(isisAdapter_itk PROPERTIES LINKER_LANGUAGE C)
	target_link_libraries(isisAdapter_itk utils data )
ENDIF(ITK_FOUND)



IF(VTK_FOUND)
	add_library(isisAdapter_vtk vtkAdapter.cpp)
	target_link_libraries(isisAdapter_vtk utils data )
	set_target_properties(isisAdapter_vtk PROPERTIES LINK_FLAGS "--no-undefined" )
ENDIF(VTK_FOUND)

if(${CMAKE_PROJECT_NAME}_HAVE_QT4)
	add_subdirectory(qt4)
endif(${CMAKE_PROJECT_NAME}_HAVE_QT4)
