// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#include "doc_vects.h"
#include <dlib/matrix.h>
#include <dlib/statistics.h>
#include <map>
#include <mitie/group_tokenizer.h>
#include <mitie/gigaword_reader.h>
#include <mitie/unigram_tokenizer.h>

using namespace std;
using namespace dlib;
using namespace mitie;

typedef std::vector<std::pair<dlib::uint32, float> > sparse_vector_type;

// ----------------------------------------------------------------------------------------

static unsigned long get_word_id (
    const std::map<std::string, unsigned long>& vocab,
    const std::string& word
)
{
    std::map<std::string, unsigned long>::const_iterator i;
    i = vocab.find(word);
    if (i != vocab.end())
        return i->second;
    else
        return vocab.size();
}

// ----------------------------------------------------------------------------------------

static std::map<std::string, unsigned long> make_word_to_int_mapping (
    const std::map<std::string, unsigned long>& words,
    unsigned long num
)
{
    std::vector<std::pair<unsigned long, std::string> > temp;
    temp.reserve(words.size());
    for (std::map<std::string, unsigned long>::const_iterator i = words.begin(); i != words.end(); ++i)
    {
        temp.push_back(make_pair(i->second, i->first));
    }

    std::sort(temp.rbegin(), temp.rend());

    // pick the most common words for our result set 
    std::map<std::string, unsigned long> res;
    for (unsigned long i = 0; i < temp.size() && res.size() < num; ++i)
    {
        res[temp[i].second] = i;
    }
    return res;
}

// ----------------------------------------------------------------------------------------

void gigaword_doc_to_vects (
    const std::map<std::string, unsigned long>& words,
    const gigaword_document& doc,
    sparse_vector_type& lhs,
    sparse_vector_type& rhs,
    dlib::rand& rnd
)
{
    lhs.clear();
    rhs.clear();
    istringstream sin(doc.text + " " + doc.headline);
    unigram_tokenizer tok(sin);
    string word;
    while (tok(word))
    {
        const unsigned long id = get_word_id(words, word);
        if (rnd.get_random_float() > 0.5)
        {
            lhs.push_back(make_pair(id, 1));
        }
        else
        {
            rhs.push_back(make_pair(id, 1));
        }
    }

    make_sparse_vector_inplace(lhs);
    make_sparse_vector_inplace(rhs);
}

// ----------------------------------------------------------------------------------------

void make_doc_vects(const dlib::command_line_parser& parser)
{
    const long vocab_size = 300000;
    const long num_contexts = 40000000;
    const long num_correlations = get_option(parser, "dims", 500);

    ifstream fin("top_word_counts.dat", ios::binary);
    std::map<std::string, unsigned long> words;
    deserialize(words, fin);
    cout << "num words in dictionary: " << words.size() << endl;

    words = make_word_to_int_mapping(words,vocab_size);

    std::vector<dlib::file> files = get_files_in_directory_tree(directory(parser[0]), match_all());
    cout << "number of gigaword XML files found: " << files.size() << endl;
    gigaword_reader reader(files);

    random_subset_selector<sparse_vector_type> L, R;
    L.set_max_size(num_contexts);
    R.set_max_size(num_contexts);

    // read a bunch of document vectors out of the gigaword corpus
    gigaword_document doc;
    sparse_vector_type lhs, rhs;
    dlib::rand rnd;
    while (reader(doc))
    {
        gigaword_doc_to_vects(words, doc, lhs, rhs, rnd);
        L.add(lhs);
        R.add(rhs);
    }

    cout << "Number of document vectors collected: " << L.size() << endl;
    cout << "Done gathering data, now running CCA." << endl;

    matrix<float> Ltrans, Rtrans;
    matrix<float> cors = cca(L, R, Ltrans, Rtrans, num_correlations, 30);
    cout << "CCA correlations: "<< trans(cors) << endl;

    // now build a map of words to their CCA reduced representation
    std::map<string, matrix<float,0,1> > word_vectors;
    for (std::map<string,unsigned long>::const_iterator i = words.begin(); i != words.end(); ++i)
    {
        // Ltrans and Rtrans should really contain the same basic information.  So we just
        // use Ltrans and ignore the other since it should be redundant.
        word_vectors[i->first] = trans(rowm(Ltrans, get_word_id(words, i->first)));
    }

    cout << "Saving word to vector map to doc_vects.dat file..." << endl;
    std::ofstream fout("doc_vects.dat", ios::binary);
    serialize(word_vectors, fout);
}

// ----------------------------------------------------------------------------------------


