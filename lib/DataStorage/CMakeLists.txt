#list all source files here
add_library(data io_application.cpp needed.cpp chunk.cpp ndimensional.cpp image.cpp io_factory.cpp typeptr_converter.cpp numeric_convert.cpp io_interface.cpp typeptr_base.cpp)

target_link_libraries(data utils ${CMAKE_DL_LIBS} ${Boost_LIBRARIES})

set_target_properties(data PROPERTIES LINK_FLAGS "--no-undefined")
