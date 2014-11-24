// Copyright (C) 2012 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_XTECH_UNIGRAM_ToKENIZER_H_
#define MIT_LL_XTECH_UNIGRAM_ToKENIZER_H_

#include <string>
#include <iostream>
#include <fstream>
#include <mitie/conll_tokenizer.h>

namespace mitie
{

// ----------------------------------------------------------------------------------------

    class unigram_tokenizer
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool for reading a sequence of unigrams from a file.  It is just
                a version of the conll_tokenizer except that it also replaces any numbers
                with # characters.
        !*/

    public:
        typedef std::string token_type;

        unigram_tokenizer (
        )  {}

        unigram_tokenizer (
            std::istream& in
        ) : tok(in) { }

        bool operator() (std::string& token)
        {
            const bool result = tok(token);
            convert_numbers(token);
            return result;
        }

    private:

        static inline void convert_numbers (
            std::string& str
        )
        {
            for (unsigned long i = 0; i < str.size(); ++i)
            {
                if ('0' <= str[i] && str[i] <= '9')
                    str[i] = '#';
            }
        }

        conll_tokenizer tok;
    };

// ----------------------------------------------------------------------------------------

}

#endif // MIT_LL_XTECH_UNIGRAM_ToKENIZER_H_

