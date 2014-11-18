
cmake_minimum_required(VERSION 2.8.4)

PROJECT(mex_functions)

include(../../dlib/dlib/matlab/cmake_mex_wrapper)
include(../../mitielib/cmake)

add_mex_function(extract_entities mitie)
add_mex_function(tokenize_file mitie)

