
// Put the global functions in our api into a java class called global.
%module global 

%{
#include "swig_api.h"
%}

// Convert all C++ exceptions into java.lang.Exception
%exception {
    try {
        $action
    } catch(std::exception& e) {
        jclass clazz = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(clazz, e.what());
        return $null;
    }
}

%include "swig_api.h"

