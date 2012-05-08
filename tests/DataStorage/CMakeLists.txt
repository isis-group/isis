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
add_executable( typePtrTest valueArrayTest.cpp )
add_executable( filePtrTest filePtrTest.cpp )

target_link_libraries( typePtrTest ${Boost_LIBRARIES} isis_core )
target_link_libraries( filePtrTest ${Boost_LIBRARIES} isis_core )
target_link_libraries( chunkTest ${Boost_LIBRARIES} isis_core )
target_link_libraries( sortedchunklistTest ${Boost_LIBRARIES} isis_core )
target_link_libraries( imageTest ${Boost_LIBRARIES} isis_core )
target_link_libraries( imageListTest ${Boost_LIBRARIES} isis_core )

############################################################
# add unit test targets
############################################################

add_test(NAME chunkTest COMMAND chunkTest)
add_test(NAME sortedchunklistTest COMMAND sortedchunklistTest)
add_test(NAME imageTest COMMAND imageTest)
add_test(NAME imageListTest COMMAND imageListTest)
add_test(NAME typePtrTest COMMAND typePtrTest)
