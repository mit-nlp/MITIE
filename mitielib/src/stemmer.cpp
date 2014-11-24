// Copyright (C) 2012 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#include <mitie/stemmer.h>
#include <dlib/string.h>

extern "C"
{
    struct stemmer {
        char * b;       /* buffer for word to be stemmed */
        int k;          /* offset to the end of the string */
        int j;          /* a general offset into the string */
    };

    extern struct stemmer * create_stemmer(void);
    extern void free_stemmer(struct stemmer * z);

    extern int stem(struct stemmer * z, char * b, int k);
}

namespace mitie 
{
    std::string stem_word (const std::string& str)
    {
        std::string temp = dlib::tolower(str);

        if (temp.size() <= 1)
            return temp;

        stemmer z;

        int s = stem(&z, &temp[0], temp.size()-1);

        return std::string(&temp[0], s+1);
    }

}
