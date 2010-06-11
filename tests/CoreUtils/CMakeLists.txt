
add_executable( propertyTest propertyTest.cpp )
add_executable( typeTest typeTest.cpp )
add_executable( vectorTest vectorTest.cpp )
add_executable( propMapTest propMapTest.cpp )
add_executable( singletonTest singletonTest.cpp )
add_executable( selectionTest selectionTest.cpp )

target_link_libraries( propertyTest utils ${Boost_LIBRARIES} )
target_link_libraries( typeTest utils ${Boost_LIBRARIES} )
target_link_libraries( vectorTest utils ${Boost_LIBRARIES} )
target_link_libraries( propMapTest utils ${Boost_LIBRARIES} )
target_link_libraries( singletonTest utils ${Boost_LIBRARIES} )
target_link_libraries( selectionTest utils ${Boost_LIBRARIES} )