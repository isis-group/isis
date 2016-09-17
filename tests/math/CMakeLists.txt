############################################################
# configure targets
############################################################

add_executable( fftTest fftTest.cpp )

target_link_libraries( fftTest ${Boost_LIBRARIES} ${isis_core_lib} ${isis_math_lib})

############################################################
# add ctest targets
############################################################

add_test(NAME fftTest COMMAND fftTest)
