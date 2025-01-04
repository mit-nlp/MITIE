#
# This is a CMake makefile.  You can find the cmake utility and
# information about it at http://www.cmake.org
#
cmake_minimum_required(VERSION 3.10)
project(mitie C CXX)


include(tell_visual_studio_to_use_static_runtime.cmake)


if (NOT TARGET mitie)
   if (NOT DEFINED BUILD_SHARED_LIBS)
      set(BUILD_SHARED_LIBS ON)
   endif()

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
   set_target_properties(mitie PROPERTIES POSITION_INDEPENDENT_CODE ON)
   target_compile_definitions(mitie PRIVATE MITIE_BUILD)

   if(BUILD_SHARED_LIBS)
      target_compile_definitions(mitie PUBLIC MITIE_SHARED)
   endif()

   target_include_directories(mitie PUBLIC include)
   
   if(MITIE_INTEGRATED_DLIB)
      target_link_libraries(mitie dlib)
   else()
      find_package(dlib REQUIRED)
      target_link_libraries(mitie dlib::dlib)
   endif()

   # Determine the path to our CMakeLists.txt file.
   string(REGEX REPLACE "CMakeLists.txt$" "" base_path ${CMAKE_CURRENT_LIST_FILE})
   install(TARGETS mitie
      DESTINATION ${base_path}
      )

endif()
