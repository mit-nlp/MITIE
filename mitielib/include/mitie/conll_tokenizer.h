// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_XTECH_CONLL_ToKENIZER_H_
#define MIT_LL_XTECH_CONLL_ToKENIZER_H_

#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>

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
        ) : in(0), current_stream_offset(0),next_token_front_padding(0) {}
        /*!
            ensures
                - any attempts to get a token will return false.  I.e. this will look like a 
                  tokenizer that has run out of tokens.
        !*/

        conll_tokenizer (
            std::istream& in_
        ) : in(&in_), current_stream_offset(0),next_token_front_padding(0) { }
        /*!
            ensures
                - This object will read tokens from the supplied input stream.  Note that it holds a
                  pointer to the input stream so the stream should continue to exist for the lifetime
                  of this conll_tokenizer.
        !*/

        bool operator() (std::string& token)
        {
            unsigned long ignored;
            return (*this)(token, ignored);
        }

        bool operator() (
            std::string& token,
            unsigned long& token_offset
        )
        /*!
            ensures
                - reads the next token from the input stream given to this object's constructor
                  and stores it in #token. 
                - if (there is not a next token) then
                    - #token.size() == 0
                    - returns false
                - else
                    - #token.size() != 0
                    - #token_offset == the byte offset for the first character in #token
                      within the input stream this tokenizer reads from.
                    - returns true
        !*/
        {
            bool result = get_next_token(token, token_offset);

            // check if the token starts with a unicode double quote, if so, break it into
            // two tokens.
            if (token.size() >= 4 && 
                (unsigned char)token[0] == 0xE2 &&
                (unsigned char)token[1] == 0x80 &&
                (unsigned char)token[2] == 0x9C)
            {
                next_token_offset = token_offset + 3;
                next_token_front_padding = 0;
                next_token = token.substr(3);
                token.resize(3);
                return result;
            }
            else if (token.size() >= 4 &&  // check if it ends with a unicode double code and break if so.
                (unsigned char)token[token.size()-3] == 0xE2 &&
                (unsigned char)token[token.size()-2] == 0x80 &&
                (unsigned char)token[token.size()-1] == 0x9D)
            {
                next_token_offset = token_offset + token.size()-3;
                next_token_front_padding = 0;
                next_token = token.substr(token.size()-3);
                token.resize(token.size()-3);
                return result;
            }
            else
            {
                // Check if token has a UTF-8 ’ character in it and if so then split it into
                // two tokens based on that.
                for (unsigned long i = 1; i < token.size(); ++i)
                {
                    if ((unsigned char)token[i]   == 0xE2 &&
                        i+2 < token.size() && 
                        (unsigned char)token[i+1] == 0x80 &&
                        (unsigned char)token[i+2] == 0x99)
                    {
                        // Save the second half of the string as the next token and return the
                        // first half.
                        next_token_offset = token_offset + i + next_token_front_padding;
                        // we drop 2 bytes off the front of this next token, if there are
                        // subsequent UTF-8 ’ characters here we split again and need to keep
                        // track that we dropped these two bytes so we can output the correct
                        // token_offset.
                        next_token_front_padding = 2;
                        next_token = token.substr(i+2);
                        next_token[0] = '\'';
                        token.resize(i);
                        return result;
                    }
                }
            }

            next_token_front_padding = 0;
            return result;
        }

    private:

        bool get_next_token (
            std::string& token,
            unsigned long& token_offset
        )
        {
            if (next_token.size() != 0)
            {
                token.swap(next_token);
                next_token.clear();
                token_offset = next_token_offset;
                return true;
            }
            token_offset = current_stream_offset;
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
                        token += get_next_char();
                    }
                }
                else if (ch == '[' ||
                    ch == ']' ||
                    ch == '.' ||
                    ch == '(' ||
                    ch == ')' ||
                    ch == '!' ||
                    ch == ',' ||
                    ch == '"' ||
                    ch == ':' ||
                    ch == '|' ||
                    ch == '?')
                {
                    if (token.size() == 0 )
                    {
                        token += get_next_char();
                        return true;
                    }
                    else if (ch == '.' && (token.size() == 1 || 
                            (token.size() >= 1 && token[token.size()-1] == '.') ||
                            (token.size() >= 2 && token[token.size()-2] == '.')))
                    {
                        token += get_next_char();
                    }
                    // catch stuff like Jr.  or St.
                    else if (ch == '.' && token.size() == 2 && isupper(token[0]) && islower(token[1]))
                    {
                        get_next_char(); // but drop the trailing .
                        return true;
                    }
                    else
                    {
                        // if this is a number followed by a comma or period then just keep
                        // accumulating the token.
                        const char last = token[token.size()-1];
                        if ((ch == ',' || ch == '.') && '0' <= last && last <= '9')
                        {
                            token += get_next_char();
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
                    get_next_char();
                    if (token.size() != 0)
                        return true;
                    else
                        ++token_offset;
                }
                else if ((unsigned char)ch == 0xc2) // Check if this is a unicode non-breaking space.  We want to treat those like whitespace.
                {
                    get_next_char();
                    if ((unsigned char)in->peek() == 0xa0) // if it was a non-breaking space then do whitespace handling
                    {
                        // discard whitespace
                        get_next_char();
                        if (token.size() != 0)
                            return true;
                        else
                            token_offset+=2;
                    }
                    else
                    {
                        token += ch;
                    }
                }
                else
                {
                    token += get_next_char();
                }
            }

            if (token.size() != 0)
            {
                return true;
            }

            return false;
        }

        inline char get_next_char()
        {
            ++current_stream_offset;
            return (char)in->get();
        }

        std::istream* in;
        std::string next_token;
        unsigned long current_stream_offset;
        unsigned long next_token_offset;
        unsigned long next_token_front_padding;

        /*!
            CONVENTION
                - current_stream_offset == The byte offset for the byte in the stream returned by the next call to in->get()
                - if (next_token.size() != 0) then
                    - The next token we should return is stored in next_token
                    - next_token_offset == the byte offset for next_token
        !*/
    };

// ----------------------------------------------------------------------------------------

}

#endif // MIT_LL_XTECH_CONLL_ToKENIZER_H_

