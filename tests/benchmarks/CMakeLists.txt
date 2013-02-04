############################################################
# The ISIS project
# 
# Main CMake configuration file of the ISIS DataStorage tests.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Tue, 28 Jun 2011 18:46:55 +0200
# 
############################################################

############################################################
# configure targets
############################################################

add_executable( imageStresstest imageStresstest.cpp )
add_executable( valueArrayStresstest valueArrayStresstest.cpp )
add_executable( valueIteratorStresstest valueIteratorStresstest.cpp)
add_executable( typedIteratorStresstest typedIteratorStresstest.cpp)
add_executable( vectorStresstest vectorStresstest.cpp)
add_executable( chunkVoxelStressTest chunkVoxelStressTest.cpp )

target_link_libraries( valueIteratorStresstest ${Boost_LIBRARIES} ${isis_core_lib} )
target_link_libraries( typedIteratorStresstest ${Boost_LIBRARIES} ${isis_core_lib} )
target_link_libraries( imageStresstest ${Boost_LIBRARIES} ${isis_core_lib} )
target_link_libraries( valueArrayStresstest ${Boost_LIBRARIES} ${isis_core_lib} )
target_link_libraries( vectorStresstest ${Boost_LIBRARIES} ${isis_core_lib} )
target_link_libraries( chunkVoxelStressTest ${Boost_LIBRARIES} ${isis_core_lib} )

############################################################
# add unit test targets
############################################################

# benchmarks are no default unit test targets
# add_test(NAME imageStresstest COMMAND imageStresstest)
# add_test(NAME typePtrStresstest COMMAND typePtrStresstest)

