cmake_minimum_required(VERSION 3.10)
project(mitie_project)


option(MITIE_INTEGRATED_DLIB "mitie: build and statically link integrated dlib as shipped with MITIE codebase (otherwise look for system dlib)" ON)
option(MITIE_INTEGRATED_DLIB_DISABLE_OPT_COMPONENTS "mitie: disable optional components not required for MITIE when building integrated dlib (image libraries, FFmpeg, SQLite3)" ON)
option(MITIE_BUILD_TOOLS "mitie: build tools" ON)
option(MITIE_BUILD_EXAMPLES "mitie: build examples" ON)
option(MITIE_APPLY_EXECUTABLES_PREFIX "mitie: add 'mitie-' prefix to executables" OFF)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(MITIE_APPLY_EXECUTABLES_PREFIX)
   set(MITIE_EXECUTABLES_PREFIX "mitie-")
else()
   set(MITIE_EXECUTABLES_PREFIX "")
endif()


# Build integrated dlib if requested
if(MITIE_INTEGRATED_DLIB)
   if(MITIE_INTEGRATED_DLIB_DISABLE_OPT_COMPONENTS)
      set(DLIB_PNG_SUPPORT       OFF CACHE STRING "not needed by MITIE" FORCE)
      set(DLIB_JPEG_SUPPORT      OFF CACHE STRING "not needed by MITIE" FORCE)
      set(DLIB_JXL_SUPPORT       OFF CACHE STRING "not needed by MITIE" FORCE)
      set(DLIB_GIF_SUPPORT       OFF CACHE STRING "not needed by MITIE" FORCE)
      set(DLIB_WEBP_SUPPORT      OFF CACHE STRING "not needed by MITIE" FORCE)
      set(DLIB_USE_FFMPEG        OFF CACHE STRING "not needed by MITIE" FORCE)
      set(DLIB_LINK_WITH_SQLITE3 OFF CACHE STRING "not needed by MITIE" FORCE)
   endif()

   add_subdirectory(dlib)
endif()


# MITIE core library
add_subdirectory(mitielib)

# MITIE tools
if(MITIE_BUILD_TOOLS)
   add_subdirectory(tools/ner_conll)
   add_subdirectory(tools/ner_stream)
   add_subdirectory(tools/train_freebase_relation_detector)
   add_subdirectory(tools/wordrep)
endif()

# MITIE examples (C and C++)
if(MITIE_BUILD_EXAMPLES)
   add_subdirectory(examples/C/ner)
   add_subdirectory(examples/C/relation_extraction)
   add_subdirectory(examples/cpp/ner)
   add_subdirectory(examples/cpp/relation_extraction)
   add_subdirectory(examples/cpp/text_categorizer)
   add_subdirectory(examples/cpp/train_ner)
   add_subdirectory(examples/cpp/train_relation_extraction)
   add_subdirectory(examples/cpp/train_text_categorizer)
   add_subdirectory(examples/cpp/train_text_categorizer_BoW)
endif()
