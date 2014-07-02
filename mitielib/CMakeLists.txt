#
# This is a CMake makefile.  You can find the cmake utility and
# information about it at http://www.cmake.org
#

# setting this makes CMake allow normal looking if else statements
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

cmake_minimum_required(VERSION 2.4)

# Suppress cmake warnings about changes in new versions.
if(COMMAND cmake_policy) 
   cmake_policy(SET CMP0003 NEW) 
endif()


if (NOT TARGET mitie)
   if (NOT DEFINED BUILD_SHARED_LIBS)
      set(BUILD_SHARED_LIBS 1)
   endif()

   include(../dlib/dlib/cmake)
   include_directories(include)

   set(source_files 
         src/stemmer.cpp
         src/ner_feature_extraction.cpp
         src/named_entity_extractor.cpp
         src/binary_relation_detector.cpp
         src/stem.c
         src/mitie.cpp
         src/conll_parser.cpp
         src/ner_trainer.cpp
         )

   add_library(mitie ${source_files})
   target_link_libraries(mitie dlib)

   install(TARGETS mitie
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      )
   install(FILES include/mitie.h DESTINATION include/)

endif()
