
find_package(Boost REQUIRED COMPONENTS spirit)

include_directories(${Boost_INCLUDE_DIR})

add_library(isisImageFormat_Vista_sa SHARED imageFormat_Vista_sa.cpp)

target_link_libraries(isisImageFormat_Vista_sa isis_core  ${ISIS_LIB_DEPENDS} ${Boost_LIBRARIES})

set(TARGETS ${TARGETS} isisImageFormat_Vista_sa)