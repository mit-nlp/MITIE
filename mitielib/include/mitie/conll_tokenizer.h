// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis.king@ll.mit.edu)
#ifndef MIT_LL_XTECH_CONLL_ToKENIZER_H__
#define MIT_LL_XTECH_CONLL_ToKENIZER_H__

#include <string>
#include <iostream>
#include <fstream>

namespace mitie
{

// ----------------------------------------------------------------------------------------

    class conll_tokenizer
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool for reading a sequence of tokens from a file.  In this case
                we try to tokenize the text in the same way as the CoNLL 2003 dataset was
                tokenized.
        !*/

    public:
        typedef std::string token_type;

        conll_tokenizer (
        ) : offset(0),in(0) {}
        /*!
            ensures
                - any attempts to get a token will return false.  I.e. this will look like a 
                  tokenizer that has run out of tokens.
        !*/

        conll_tokenizer (
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

                if (ch == '\'')
                {
                    if (token.size() != 0)
                    {
                        return true;
                    }
                    else
                    {
                        token += (char)in->get();
                        ++offset;
                    }
                }
                else if (ch == '[' ||
                    ch == ']' ||
                    ch == '.' ||
                    ch == '!' ||
                    ch == ',' ||
                    ch == '"' ||
                    ch == ':' ||
                    ch == '|' ||
                    ch == '?')
                {
                    if (token.size() == 0 )
                    {
                        token += (char)in->get();
                        ++offset;
                        return true;
                    }
                    else if (ch == '.' && (token.size() == 1 || 
                            (token.size() >= 1 && token[token.size()-1] == '.') ||
                            (token.size() >= 2 && token[token.size()-2] == '.')))
                    {
                        token += (char)in->get();
                        ++offset;
                    }
                    else
                    {
                        // if this is a number followed by a comma or period then just keep
                        // accumulating the token.
                        const char last = token[token.size()-1];
                        if ((ch == ',' || ch == '.') && '0' <= last && last <= '9')
                        {
                            token += (char)in->get();
                            ++offset;
                        }
                        else
                        {
                            return true;
                        }
                    }
                }
                else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
                {
                    // discard whitespace
                    in->get();
                    ++offset;
                    if (token.size() != 0)
                        return true;
                }
                else
                {
                    token += (char)in->get();
                    ++offset;
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

#endif // MIT_LL_XTECH_CONLL_ToKENIZER_H__

