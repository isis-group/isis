include_directories(${CMAKE_SOURCE_DIR}/lib/Core)

option(ISIS_BUILD_BASIC_FILTER "Build the basic filters." ON)


if( ISIS_BUILD_BASIC_FILTER )
  add_subdirectory(BasicFilter)
endif( ISIS_BUILD_BASIC_FILTER )