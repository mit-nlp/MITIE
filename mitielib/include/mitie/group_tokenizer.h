// Copyright (C) 2012 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_GROUP_ToKENIZER_H_
#define MIT_LL_GROUP_ToKENIZER_H_

#include <string>
#include <vector>
#include <dlib/dir_nav.h>
#include <fstream>


namespace mitie
{
    template <
        typename basic_tokenizer
        >
    class group_tokenizer 
    {
        /*!
            REQUIREMENTS ON basic_tokenizer
                This must be an object with an interface compatible with the unigram_tokenizer.

            WHAT THIS OBJECT REPRESENTS
                This object is a tool for turning a single document tokenizer into one that
                tokenizes a bunch of documents in a set.  It therefore makes it look like all
                the documents have been concatenated together.
        !*/

    public:

        typedef std::string token_type;

        group_tokenizer (
            const char* filename 
        )
        /*!
            ensures
                - This object will read tokens from the file with the given filename.
        !*/
        {
            next_file = 0;
            files.push_back(filename);
        }

        group_tokenizer (
            const std::string& filename 
        ) 
        /*!
            ensures
                - This object will read tokens from the file with the given filename.
        !*/
        {
            next_file = 0;
            files.push_back(filename);
        }

        group_tokenizer (
            const dlib::file& filename
        ) 
        /*!
            ensures
                - This object will read tokens from the file with the given filename.
        !*/
        {
            next_file = 0;
            files.push_back(filename.full_name());
        }

        group_tokenizer (
            const std::vector<dlib::file>& file_list 
        ) 
        /*!
            ensures
                - This object will read tokens out of the list of supplied file_list.
        !*/
        {
            next_file = 0;
            for (unsigned long i = 0; i < file_list.size(); ++i)
            {
                files.push_back(file_list[i].full_name());
            }
        }

        void reset (
        ) { next_file = 0; tok = basic_tokenizer(); }
        /*!
            ensures
                - puts the tokenizer back at the start of the token sequence.  Therefore,
                  calling reset() will allow you to do another pass over the tokens.
        !*/

        bool operator() (
            std::string& token 
        )
        /*!
            ensures
                - reads the next token from the dataset given to this object's constructor
                  and stores it in #token.
                - if (there is not a next token) then
                    - #token.size() == 0
                    - returns false
                - else
                    - #token.size() != 0
                    - returns true
        !*/
        {
            while (true)
            {
                // if there is a token then return it
                if (tok(token))
                    return true;

                if (next_file >= files.size())
                    return false;

                fin.close();
                fin.clear();
                fin.open(files[next_file].c_str());
                ++next_file;
                tok = fin;
            }
        }

    private:
        unsigned long next_file;
        std::vector<std::string> files;
        std::ifstream fin;
        basic_tokenizer tok;
    };
}

#endif // MIT_LL_GROUP_ToKENIZER_H_

