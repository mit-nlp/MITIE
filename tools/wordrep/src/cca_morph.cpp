// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include "cca_morph.h"
#include <map>
#include <dlib/matrix.h>
#include <dlib/statistics.h>
#include <mitie/approximate_substring_set.h>
#include <mitie/total_word_feature_extractor.h>

using namespace std;
using namespace dlib;
using namespace mitie;

typedef std::vector<std::pair<dlib::uint32, float> > sparse_vector_type;

// ----------------------------------------------------------------------------------------

sparse_vector_type dense_to_sparse (
    const matrix<float,0,1>& vect
)
{
    sparse_vector_type res(vect.size());
    for (long i = 0; i < vect.size(); ++i)
        res[i] = std::make_pair(i, vect(i));
    return res;
}

// ----------------------------------------------------------------------------------------

void learn_morphological_dimension_reduction (
    const approximate_substring_set& substrs,
    const std::map<std::string, matrix<float,0,1> >& word_vectors,
    const long num_correlations,
    matrix<float>& morph_trans
)
{
    std::map<std::string, matrix<float,0,1> >::const_iterator i;


    std::vector<sparse_vector_type> L, R;

    sparse_vector_type temp;
    std::vector<dlib::uint16> hits;
    
    cout << "building morphological vectors" << endl;
    for (i = word_vectors.begin(); i != word_vectors.end(); ++i)
    {
        L.push_back(dense_to_sparse(i->second));
        substrs.find_substrings(i->first, hits);
        temp.clear();
        for (unsigned long i = 0; i < hits.size(); ++i)
            temp.push_back(make_pair(hits[i],1));
        make_sparse_vector_inplace(temp);
        R.push_back(temp);
    }
    cout << "L.size(): " << L.size() << endl;
    cout << "R.size(): " << R.size() << endl;

    cout << "Now running CCA on word <-> morphology..." << endl;
    matrix<float> Ltrans;
    cout << "correlations: " << trans(cca(L, R, Ltrans, morph_trans, num_correlations, 1000, 2)) << endl;
    //print_true_correlations(L, R, Ltrans, morph_trans);
}

// ----------------------------------------------------------------------------------------

void cca_morph(const dlib::command_line_parser& parser)
{
    const long num_morph_correlations = 90;

    std::ifstream fin("word_vects.dat", ios::binary);
    std::map<std::string, matrix<float,0,1> > word_vectors;
    deserialize(word_vectors, fin);
    cout << "num word vectors loaded: " << word_vectors.size() << endl;
    cout << "got word vectors, now learn how they correlate with morphological features." << endl;

    fin.close();
    fin.open("substring_set.dat", ios::binary);
    approximate_substring_set substring_set;
    deserialize(substring_set, fin);

    matrix<float> morph_trans;
    learn_morphological_dimension_reduction(substring_set, word_vectors, num_morph_correlations, morph_trans);

    // morph_trans should have a row for every possible output from substring_set.  But
    // since we work with sparse vectors and some outputs might not have been observed in
    // learn_morphological_dimension_reduction() we need to make sure this is the case.
    if (morph_trans.nr() != substring_set.max_substring_id()+1)
    {
        matrix<float> temp(substring_set.max_substring_id()+1, morph_trans.nc());
        set_subm(temp, get_rect(morph_trans)) = morph_trans;
        temp.swap(morph_trans);
    }


    word_morphology_feature_extractor fe(substring_set, morph_trans);
    cout << "morphological feature dimensionality: "<< fe.get_num_dimensions() << endl;

    ofstream fout("word_morph_feature_extractor.dat", ios::binary);
    serialize(fe, fout);

    total_word_feature_extractor tfe(word_vectors, fe);
    cout << "total word feature dimensionality: "<< tfe.get_num_dimensions() << endl;
    serialize("total_word_feature_extractor.dat") << "mitie::total_word_feature_extractor" << tfe;
}

// ----------------------------------------------------------------------------------------

