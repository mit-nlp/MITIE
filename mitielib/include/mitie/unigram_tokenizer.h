// Copyright (C) 2012 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis.king@ll.mit.edu)
#ifndef MIT_LL_XTECH_UNIGRAM_ToKENIZER_H__
#define MIT_LL_XTECH_UNIGRAM_ToKENIZER_H__

#include <string>
#include <iostream>
#include <fstream>

namespace mitie
{

// ----------------------------------------------------------------------------------------

    class unigram_tokenizer
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool for reading a sequence of unigrams from a file.  In this case
                a unigram is any of the following:
                    - a word made up of [a-zA-Z]
                    - any of the individual characters from the string "[].!,?:|"
        !*/

    public:
        typedef std::string token_type;

        unigram_tokenizer (
        ) : offset(0),in(0) {}
        /*!
            ensures
                - any attempts to get a token will return false.  I.e. this will look like a 
                  tokenizer that has run out of tokens.
        !*/

        unigram_tokenizer (
            std::istream& in_
        ) : offset(0),in(&in_) { }
        /*!
            ensures
                - This object will read tokens from the supplied input stream.
        !*/

        bool operator() (std::string& token)
        {
            unsigned long junk;
            return (*this)(token, junk);
        }

        bool operator() (std::string& token, unsigned long& pos)
        /*!
            ensures
                - reads the next token from the input stream given to this object's constructor
                  and stores it in #token. 
                - #pos == the number of characters read from the input stream prior to
                  encountering the returned token.  That is, this value is the character
                  offset from the beginning of the stream that indicates the position of the
                  first character in token.
                - if (there is not a next token) then
                    - #token.size() == 0
                    - returns false
                - else
                    - #token.size() != 0
                    - returns true
        !*/
        {
            pos = offset;
            token.clear();
            if (!in)
                return false;

            while (in->peek() != EOF)
            {
                if (token.size() == 0)
                    pos = offset;

                const char ch = (char)in->peek();

                if (('a' <= ch && ch <= 'z') || 
                    ('A' <= ch && ch <= 'Z') ||
                    (ch == '\'' && token.size() != 0) || 
                    ('0' <= ch && ch <= '9') || 
                    ((ch == ',' || ch == '.') && token.size() != 0 && token[token.size()-1] == '#') || 
                    (ch == '#') // this is here because it will let us retokenize strings made out of already tokenized text.
                    )
                {
                    if ('0' <= ch && ch <= '9')
                    {
                        token += '#';
                        in->get();
                        ++offset;
                    }
                    else if (ch != ',' && ch != '.')
                    {
                        token += (char)in->get();
                        ++offset;
                    }
                    else
                    {
                        in->get();
                        ++offset;
                    }
                }
                else if (token.size() != 0)
                {
                    return true;
                }
                else
                {
                    if (ch == '[' ||
                        ch == ']' ||
                        ch == '.' ||
                        ch == '!' ||
                        ch == ',' ||
                        ch == ':' ||
                        ch == '|' ||
                        ch == '?')
                    {
                        token += (char)in->get();
                        ++offset;
                        return true;
                    }
                    else
                    {
                        // ignore the next character
                        in->get();
                        ++offset;
                    }
                }
            }

            if (token.size() != 0)
            {
                return true;
            }

            return false;
        }

    private:

        unsigned long offset;
        std::istream* in;
    };

// ----------------------------------------------------------------------------------------

}

#endif // MIT_LL_XTECH_UNIGRAM_ToKENIZER_H__

