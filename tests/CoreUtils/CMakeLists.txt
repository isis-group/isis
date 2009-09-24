
add_executable(propertyTest propertyTest.cpp)
add_executable(typeTest typeTest.cpp)

target_link_libraries(propertyTest utils)
target_link_libraries(typeTest utils)

