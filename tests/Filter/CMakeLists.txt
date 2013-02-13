############################################################
# The ISIS project
#
# Main CMake configuration file of the ISIS Filter tests.
#
# Author: Erik Tuerke <etuerke@googlemail.com>
# Date: Tue, 08 May 2012 14:46:25 +0200
#
############################################################

include_directories(${CMAKE_SOURCE_DIR}/lib/Filter)

############################################################
# configure targets
############################################################

add_executable( basicFilterTest basicFilterTest.cpp )

target_link_libraries( basicFilterTest ${Boost_LIBRARIES} ${isis_core_lib} isisFilter_Basic)

############################################################
# add unit test targets
############################################################

add_test(NAME basic_filter_test COMMAND basicFilterTest )