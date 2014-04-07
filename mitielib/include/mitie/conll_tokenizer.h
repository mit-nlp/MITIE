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
        ) : in(0) {}
        /*!
            ensures
                - any attempts to get a token will return false.  I.e. this will look like a 
                  tokenizer that has run out of tokens.
        !*/

        conll_tokenizer (
            std::istream& in_
        ) : in(&in_) { }
        /*!
            ensures
                - This object will read tokens from the supplied input stream.
        !*/

        bool operator() (std::string& token)
        /*!
            ensures
                - reads the next token from the input stream given to this object's constructor
                  and stores it in #token. 
                - if (there is not a next token) then
                    - #token.size() == 0
                    - returns false
                - else
                    - #token.size() != 0
                    - returns true
        !*/
        {
            bool result = get_next_token(token);

            // Check if token has a UTF-8 ’ character in it and if so then split it into
            // two tokens based on that.
            for (unsigned long i = 0; i < token.size(); ++i)
            {
                if ((unsigned char)token[i]   == 0xE2 &&
                    i+2 < token.size() && 
                    (unsigned char)token[i+1] == 0x80 &&
                    (unsigned char)token[i+2] == 0x99)
                {
                    // Save the second half of the string as the next token and return the
                    // first half.
                    next_token = token.substr(i+2);
                    next_token[0] = '\'';
                    token.resize(i);
                    return result;
                }
            }

            return result;
        }

    private:

        bool get_next_token (std::string& token)
        {
            if (next_token.size() != 0)
            {
                token.swap(next_token);
                next_token.clear();
                return true;
            }
            token.clear();
            if (!in)
                return false;

            while (in->peek() != EOF)
            {
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
                        return true;
                    }
                    else if (ch == '.' && (token.size() == 1 || 
                            (token.size() >= 1 && token[token.size()-1] == '.') ||
                            (token.size() >= 2 && token[token.size()-2] == '.')))
                    {
                        token += (char)in->get();
                    }
                    else
                    {
                        // if this is a number followed by a comma or period then just keep
                        // accumulating the token.
                        const char last = token[token.size()-1];
                        if ((ch == ',' || ch == '.') && '0' <= last && last <= '9')
                        {
                            token += (char)in->get();
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
                    if (token.size() != 0)
                        return true;
                }
                else
                {
                    token += (char)in->get();
                }
            }

            if (token.size() != 0)
            {
                return true;
            }

            return false;
        }

        std::istream* in;
        std::string next_token;
    };

// ----------------------------------------------------------------------------------------

}

#endif // MIT_LL_XTECH_CONLL_ToKENIZER_H__

