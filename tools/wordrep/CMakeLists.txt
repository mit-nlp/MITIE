#
# This is a CMake makefile.  You can find the cmake utility and
# information about it at http://www.cmake.org
#


cmake_minimum_required(VERSION 2.6)



set(project_name wordrep)
set(source
   src/main.cpp
   src/basic_morph.cpp
   src/word_vects.cpp
   src/cca_morph.cpp
   src/doc_vects.cpp
   )


PROJECT(${project_name})

include(../../mitielib/cmake)


ADD_EXECUTABLE(${project_name} ${source})
TARGET_LINK_LIBRARIES(${project_name} mitie )



