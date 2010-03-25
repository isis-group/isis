#list all source files here

add_library(utils typeptr_converter.cpp type_converter.cpp type_base.cpp types.cpp property.cpp propmap.cpp message.cpp singletons.cpp)
target_link_libraries(utils ${Boost_LIBRARIES})

set_target_properties(utils PROPERTIES LINK_FLAGS "--no-undefined")
