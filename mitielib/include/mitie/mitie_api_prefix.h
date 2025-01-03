#ifndef MITIE_API_PREFIX_H_
#define MITIE_API_PREFIX_H_

#ifdef MITIE_SHARED
#    if defined(_WIN32)
#        ifdef MITIE_BUILD
#            define MITIE_API __declspec(dllexport)
#        else
#            define MITIE_API __declspec(dllimport)
#        endif
#    else
#        define MITIE_API __attribute__ ((visibility ("default")))
#    endif
#else
#    define MITIE_API
#endif

#endif // MITIE_API_PREFIX_H_
