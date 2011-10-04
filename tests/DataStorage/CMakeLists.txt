############################################################
# The ISIS project
# 
# Main CMake configuration file of the ISIS DataStorage tests.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 14:46:25 +0200
# 
############################################################

############################################################
# configure targets
############################################################

add_executable( chunkTest chunkTest.cpp )
add_executable( sortedchunklistTest sortedchunklistTest.cpp)
add_executable( imageTest imageTest.cpp )
add_executable( imageListTest imageListTest.cpp )
add_executable( imageStresstest imageStresstest.cpp )
add_executable( typePtrTest typePtrTest.cpp )

target_link_libraries( typePtrTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS} )
target_link_libraries( chunkTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS} )
target_link_libraries( sortedchunklistTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS} )
target_link_libraries( imageTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS} )
target_link_libraries( imageListTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS} )
target_link_libraries( imageStresstest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS} )

