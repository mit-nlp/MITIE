// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include <dlib/matrix.h>
#include <fstream>
#include <string>
#include <mitie/conll_tokenizer.h>
using namespace dlib;
using namespace std;
using namespace mitie;



void mex_function (
    const std::string& filename,
    std::vector<std::string>& tokens
) 
{
    tokens.clear();
    ifstream fin(filename.c_str());
    if (!fin)
        throw error("Unable to load input text file: " + filename);

    // The conll_tokenizer splits the contents of an istream into a bunch of words and is
    // MITIE's default tokenization method. 
    conll_tokenizer tok(fin);
    string token;
    // Read the tokens out of the file one at a time and store into tokens.
    while(tok(token))
        tokens.push_back(token);

}


// #including this brings in all the mex boiler plate needed by MATLAB.
#include "mex_wrapper.cpp"

