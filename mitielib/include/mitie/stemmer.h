// Copyright (C) 2012 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_STEM_WoRD_H_
#define MIT_LL_STEM_WoRD_H_

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

#endif // MIT_LL_STEM_WoRD_H_

