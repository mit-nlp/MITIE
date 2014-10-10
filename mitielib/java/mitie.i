%module mitie 

%include "std_string.i"
%include "std_vector.i"

%{
#include "mitie_swig_api.h"
%}

%template(StringVector) std::vector<std::string>;
%template(TokenIndexVector) std::vector<TokenIndexPair>;
%template(EntityMentionVector) std::vector<EntityMention>;

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

%include "mitie_swig_api.h"

