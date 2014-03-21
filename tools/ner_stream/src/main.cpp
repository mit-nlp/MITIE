// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis.king@ll.mit.edu)


#include <iostream>
#include <sstream>
#include <mitie/named_entity_extractor.h>
#include <mitie/conll_tokenizer.h>
#include <dlib/time_this.h>

using namespace std;
using namespace mitie;

std::vector<std::string> tokenize (
    const std::string& line
)
{
    istringstream sin(line);
    conll_tokenizer tok(sin);
    std::vector<std::string> words;
    string word;
    while(tok(word))
        words.push_back(word);

    return words;
}

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cerr << "Error, you must give a MITIE ner model file as the first argument to this program!" << endl;
        return 1;
    }


    named_entity_extractor ner;
    ifstream fin(argv[1], ios::binary);
    if (!fin)
    {
        cerr << "Error, unable to open file: " << argv[1] << endl;
        return 1;
    }
    cerr << "Loading MITIE NER model file...";
    TIME_THIS_TO(deserialize(ner, fin), cerr);
    cerr << endl;

    const std::vector<std::string> tags = ner.get_tag_name_strings();

    string line;
    while (getline(cin, line))
    {
        std::vector<std::string> tokens = tokenize(line);
        std::vector<std::pair<unsigned long, unsigned long> > chunks;
        std::vector<unsigned long> chunk_tags;
        ner(tokens, chunks, chunk_tags);

        // Push an empty chunk onto the end so we can avoid complicated bounds checking in
        // the following loop.
        chunks.push_back(make_pair(tokens.size(), tokens.size()));

        unsigned long next = 0;
        for (unsigned long i = 0; i < tokens.size(); ++i)
        {
            if (i == chunks[next].second)
            {
                cout << "] ";
                ++next;
            }

            if (i == chunks[next].first)
                cout << "[" << tags[chunk_tags[next]] << " ";
            cout << tokens[i];
            if (i+1 != chunks[next].second)
                cout << " ";
        }
        cout << endl;
    }
}

