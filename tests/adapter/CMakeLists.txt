
if(ISIS_QT5)
	find_package(Qt5Widgets)
	add_executable( qt5test qt5test.cpp )
	target_link_libraries( qt5test ${Boost_LIBRARIES} ${isis_core_lib} ${isis_adapter_qt5_lib} Qt5::Widgets)
	add_test(NAME qt5test COMMAND qt5test)
endif(ISIS_QT5)

