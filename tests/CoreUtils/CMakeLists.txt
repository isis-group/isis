############################################################
# The ISIS project
# 
# Main CMake configuration file of the ISIS CoreUtils tests.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 14:41:35 +0200
# 
############################################################

############################################################
# configure targets
############################################################

add_executable( propertyTest propertyTest.cpp )
add_executable( typeTest typeTest.cpp )
add_executable( vectorTest vectorTest.cpp )
add_executable( propMapTest propMapTest.cpp )
add_executable( singletonTest singletonTest.cpp )
add_executable( selectionTest selectionTest.cpp )
add_executable( commonTest commonTest.cpp )
add_executable( istringTest istringTest.cpp )

target_link_libraries( commonTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS} )
target_link_libraries( propertyTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries( typeTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries( vectorTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries( propMapTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries( singletonTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries( selectionTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})
target_link_libraries( istringTest ${Boost_LIBRARIES} isis_core ${ISIS_LIB_DEPENDS})

############################################################
# add ctest targets
############################################################

add_test(NAME propertyTest COMMAND propertyTest)
add_test(NAME commonTest COMMAND commonTest)
add_test(NAME typeTest COMMAND typeTest)
add_test(NAME vectorTest COMMAND vectorTest)
add_test(NAME propMapTest COMMAND propMapTest)
add_test(NAME singletonTest COMMAND singletonTest)
add_test(NAME selectionTest COMMAND selectionTest)
add_test(NAME istringTest COMMAND istringTest)
