#
# This is a CMake makefile.  You can find the cmake utility and
# information about it at http://www.cmake.org
#

cmake_minimum_required(VERSION 3.10)



set(project_name "${MITIE_EXECUTABLES_PREFIX}relation_extraction_c${MITIE_EXAMPLES_SUFFIX}")
set(source
   relation_extraction_example.c
   )


PROJECT(${project_name})


include(../../../mitielib/cmake)


ADD_EXECUTABLE(${project_name} ${source})
TARGET_LINK_LIBRARIES(${project_name} mitie)


include(GNUInstallDirs)
install(TARGETS ${project_name})
