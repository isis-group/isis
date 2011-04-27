############################################################
# The ISIS project
# 
# Main CMake configuration file of the ISIS ImageIO tests.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 14:48:25 +0200
# 
############################################################

############################################################
# configure targets
############################################################

add_executable(imageIOLoadDicom imageIOLoadDicom.cpp)
add_executable(imageIONullTest imageIONullTest.cpp)
add_executable(imageIONiiTest imageIONiiTest.cpp)
add_executable(imageIOVistaTest imageIOVistaTest.cpp)
add_executable(imageIOTest imageIOTest.cpp)

target_link_libraries(imageIOLoadDicom ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries(imageIONullTest  ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries(imageIONiiTest   ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries(imageIOVistaTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries(imageIOTest      ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
