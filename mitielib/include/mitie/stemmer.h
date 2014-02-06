// Created by Davis E. King on August 17, 2012
#ifndef MIT_LL_STEM_WoRD_H__
#define MIT_LL_STEM_WoRD_H__

#include <string>

namespace mitie
{
    std::string stem_word (const std::string& word);
    /*!
        ensures
            - lowercases word and then applies the Porter stemmer.  The
              results are returned.
    !*/
}

#endif // MIT_LL_STEM_WoRD_H__

