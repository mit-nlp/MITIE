
cmake_minimum_required(VERSION 2.8.4)

PROJECT(mex_functions)

include(cmake_mex_wrapper)

include(../../dclib/dlib/cmake)

add_mex_function(example_mex_function dlib)
add_mex_function(example_mex_callback dlib)

