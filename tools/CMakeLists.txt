add_executable(isisdump isisdump.cpp)
target_link_libraries(isisdump data utils)

add_executable(isisdiff isisdiff.cpp)
target_link_libraries(isisdiff data utils)

add_executable(isisconv isisconv.cpp)
target_link_libraries(isisconv data utils)

add_executable(isisflip isisflip.cpp)
target_link_libraries(isisflip data utils)
