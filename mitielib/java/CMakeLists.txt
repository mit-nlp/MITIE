
cmake_minimum_required (VERSION 2.8.4)
project (javamitie)
set(java_package_name edu.mit.ll.mitie)
set(source_files
   ../src/binary_relation_detector.cpp
   ../src/binary_relation_detector_trainer.cpp
   ../src/conll_parser.cpp
   ../src/mitie.cpp
   ../src/named_entity_extractor.cpp
   ../src/ner_feature_extraction.cpp
   ../src/ner_trainer.cpp
   ../src/stem.c
   ../src/stemmer.cpp
   )

include_directories(
   .
   ../include
   )

# Additional dependencies
include(../../dlib/dlib/cmake)
set(additional_link_libraries dlib)

# tell swig to put the output files into the mitielib folder.
set(install_target_output_folder ..)
include(cmake_swig_jni)


