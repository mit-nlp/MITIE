// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include "word_vects.h"
#include <dlib/matrix.h>
#include <dlib/statistics.h>
#include <dlib/sliding_buffer.h>
#include <map>
#include <mitie/group_tokenizer.h>
#include <mitie/unigram_tokenizer.h>

using namespace std;
using namespace dlib;
using namespace mitie;

typedef std::vector<std::pair<dlib::uint32, float> > sparse_vector_type;

// ----------------------------------------------------------------------------------------

unsigned long get_word_id (
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

template <typename sparse_vector_type>
void get_left_and_right_context_vectors (
    const std::map<std::string, unsigned long>& vocab,
    const dlib::circular_buffer<std::string>& buf,
    sparse_vector_type& left_context,
    sparse_vector_type& right_context
)
{
    // get the left context vector
    left_context.clear();
    for (unsigned long i = 0; i < buf.size()/2; ++i)
    {
        left_context.push_back(make_pair(i*(vocab.size()+1) + get_word_id(vocab, buf[i]), 1));
    }
    make_sparse_vector_inplace(left_context);


    // get the right context vector
    right_context.clear();
    long k = 0;
    for (unsigned long i = buf.size()/2+1; i < buf.size(); ++i)
    {
        right_context.push_back(make_pair(k*(vocab.size()+1) + get_word_id(vocab, buf[i]), 1));
        ++k;
    }
    make_sparse_vector_inplace(right_context);
}

// ----------------------------------------------------------------------------------------

template <typename tokenizer_type>
void do_cca_on_windows (
    const std::map<std::string, unsigned long>& vocab,
    const long window_size,
    const long num_contexts,
    const long num_correlations,
    tokenizer_type& tok,
    matrix<float>& Ltrans,
    matrix<float>& Rtrans
)
{
    tok.reset();

    random_subset_selector<sparse_vector_type> left_contexts, right_contexts;
    left_contexts.set_max_size(num_contexts);
    right_contexts.set_max_size(num_contexts);


    dlib::circular_buffer<std::string> buf;
    buf.resize(window_size);

    sparse_vector_type left_vect, right_vect;
    cout << "Sample " << num_contexts << " random context vectors" << endl;

    std::string token;
    uint64 count = 0;
    while (tok(token))
    {
        buf.push_back(token);
        // skip the rest of the loop if buf isn't full yet
        ++count;
        if (count < buf.size())
        {
            continue;
        }

        if (left_contexts.next_add_accepts())
        {
            get_left_and_right_context_vectors(vocab, buf, left_vect, right_vect);
            left_contexts.add(left_vect);
            right_contexts.add(right_vect);
        }
        else
        {
            left_contexts.add();
            right_contexts.add();
        }
    }

    std::vector<sparse_vector_type> left(left_contexts.begin(), left_contexts.end());
    std::vector<sparse_vector_type> right(right_contexts.begin(), right_contexts.end());
    // free RAM
    left_contexts.set_max_size(0);
    right_contexts.set_max_size(0);
    
    cout << "Now do CCA (left size: " << left.size() << ", right size: " << right.size() << ")." << endl;
    cout << "correlations: "<< trans(cca(left, right, Ltrans, Rtrans, num_correlations, 40, 5));
    //print_true_correlations(left, right, Ltrans, Rtrans);
}

// ----------------------------------------------------------------------------------------

template <typename tokenizer_type>
void get_average_context_window_vector_per_word (
    const std::map<std::string, unsigned long>& vocab,
    const long window_size,
    tokenizer_type& tok,
    const matrix<float>& Ltrans,
    const matrix<float>& Rtrans,
    std::map<std::string, matrix<float,0,1> >& word_vectors
)
{
    tok.reset();
    word_vectors.clear();

    std::map<std::string,long> word_hits;
    dlib::circular_buffer<std::string> buf;
    buf.resize(window_size);

    sparse_vector_type left_vect, right_vect;

    std::string token;
    unsigned long count = 1;
    while (tok(token))
    {
        buf.push_back(token);
        // skip the rest of the loop if buf isn't full yet
        if (count < buf.size())
        {
            ++count;
            continue;
        }

        const std::string& center_word = buf[buf.size()/2];
        // only consider words in the vocab
        if (vocab.count(center_word) == 0)
            continue;

        get_left_and_right_context_vectors(vocab, buf, left_vect, right_vect);
        word_vectors[center_word] += join_cols(sparse_matrix_vector_multiply(trans(Ltrans), left_vect),
                                               sparse_matrix_vector_multiply(trans(Rtrans), right_vect));
        word_hits[center_word] += 1;
    }


    // Divide all the word vectors by their hits so they are mean vectors
    for (std::map<std::string,long>::iterator i = word_hits.begin(); i != word_hits.end(); ++i)
    {
        if (i->second > 1)
        {
            word_vectors[i->first] /= i->second;
        }
    }
}

// ----------------------------------------------------------------------------------------

std::map<std::string, unsigned long> make_word_to_int_mapping (
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

void word_vects(const dlib::command_line_parser& parser)
{
    const long vocab_size = 200000;
    const long window_size = 9;
    const long num_contexts = 50000000;
    const long num_correlations = 90;

    ifstream fin("top_word_counts.dat", ios::binary);
    std::map<std::string, unsigned long> words;
    deserialize(words, fin);

    words = make_word_to_int_mapping(words,vocab_size);

    std::vector<dlib::file> files = get_files_in_directory_tree(directory(parser[0]), match_all());
    cout << "number of raw ASCII files found: " << files.size() << endl;
    group_tokenizer<unigram_tokenizer> tok(files);

    matrix<float> Ltrans, Rtrans;
    do_cca_on_windows(words, window_size,  num_contexts, num_correlations, tok, Ltrans, Rtrans);
    cout << "CCA done, now build up average word vectors" << endl;

    std::map<std::string, matrix<float,0,1> > word_vectors;
    get_average_context_window_vector_per_word(words, window_size, tok, Ltrans, Rtrans, word_vectors);

    std::ofstream fout("word_vects.dat", ios::binary);
    serialize(word_vectors, fout);
}

// ----------------------------------------------------------------------------------------

