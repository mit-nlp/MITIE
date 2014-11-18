// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include <dlib/matrix.h>
#include <mitie/named_entity_extractor.h>
#include <string>

using namespace dlib;
using namespace std;
using namespace mitie;


#define ARG_4_DEFAULT ""

string classname;
named_entity_extractor ner;
bool did_init = false;

void mex_function (
    const std::vector<std::string>& tokens,
    std::vector<matrix<double,1> >& entities,
    std::vector<std::string>& labels,
    const std::string& model_filename
) 
{
    if (!did_init)
    {
        if (model_filename == "")
            throw dlib::error("You need to give a MITIE ner model file as the second argument the first time you call this function.");

        dlib::deserialize(model_filename) >> classname >> ner;
        did_init = true;
    }

    std::vector<pair<unsigned long, unsigned long> > chunks;
    std::vector<unsigned long> chunk_tags;
    // Now detect all the entities in the tokens.  The output of this function is a set of
    // "chunks" of tokens, each a named entity.
    ner(tokens, chunks, chunk_tags);

    const std::vector<string> tagstr = ner.get_tag_name_strings();
    entities.clear();
    labels.clear();
    // copy the entities into the output variables
    for (unsigned int i = 0; i < chunks.size(); ++i)
    {
        labels.push_back(tagstr[chunk_tags[i]]);
        const unsigned long length = chunks[i].second-chunks[i].first;
        entities.push_back(linspace(chunks[i].first+1, chunks[i].second, length));
    }
}



#include "mex_wrapper.cpp"

