
if(ISIS_QT5)
	find_package(Qt5Widgets)
	add_executable( qt5test qt5test.cpp )
	target_link_libraries( qt5test ${Boost_LIBRARIES} ${isis_core_lib} ${isis_adapter_qt5_lib} Qt5::Widgets)
	add_test(NAME qt5test COMMAND qt5test)
endif(ISIS_QT5)

if(ISIS_ITK)
	find_package(ITK REQUIRED ITKCommon ITKTransform ITKIONIFTI ITKIOImageBase)
	include(${ITK_USE_FILE})

	add_executable( itktest itktest.cpp )
	target_link_libraries( itktest ${Boost_LIBRARIES} ${isis_core_lib} ${isis_math_lib} ${isis_adapter_itk4_lib} ${ITK_LIBRARIES})
	add_test(NAME itktest COMMAND itktest)
endif(ISIS_ITK)

