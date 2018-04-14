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

set(CMAKE_POSITION_INDEPENDENT_CODE True)
include(tell_visual_studio_to_use_static_runtime.cmake)

if (NOT TARGET mitie)
   if (NOT DEFINED BUILD_SHARED_LIBS)
      set(BUILD_SHARED_LIBS 1)
   endif()

   set(DLIB_LINK_WITH_LIBPNG OFF CACHE STRING "not needed by MITIE" FORCE)
   set(DLIB_LINK_WITH_LIBJPEG OFF CACHE STRING "not needed by MITIE" FORCE)
   set(DLIB_LINK_WITH_SQLITE3 OFF CACHE STRING "not needed by MITIE" FORCE)


   include(../dlib/dlib/cmake)
   include_directories(include)

   set(source_files 
         src/stemmer.cpp
         src/ner_feature_extraction.cpp
         src/named_entity_extractor.cpp
         src/binary_relation_detector.cpp
         src/binary_relation_detector_trainer.cpp
         src/stem.c
         src/mitie.cpp
         src/conll_parser.cpp
         src/ner_trainer.cpp
         src/text_categorizer_trainer.cpp
         src/text_categorizer.cpp
         src/text_feature_extraction.cpp
         )

   add_library(mitie ${source_files})
   target_link_libraries(mitie dlib)

   # Determine the path to our CMakeLists.txt file.
   string(REGEX REPLACE "CMakeLists.txt$" "" base_path ${CMAKE_CURRENT_LIST_FILE})
   install(TARGETS mitie
      DESTINATION ${base_path}
      )

endif()
