// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include <dlib/algs.h>
#include <mitie/binary_relation_detector.h>
#include <dlib/hash.h>
#include <vector>


using namespace dlib;
using namespace mitie;

namespace 
{

    inline std::pair<uint64,uint64> hash_string (
        const std::string& str,
        const uint32 seed
    )
    {
        if (str.size() == 0)
            return make_pair(0, 0);
        return murmur_hash3_128bit(&str[0], str.size(), seed);
    }

// ----------------------------------------------------------------------------------------

    inline void accum_123gram_feats (
        sparse_vector_type& vect,
        const std::pair<unsigned long, unsigned long>& range,
        const std::vector<std::string>& tokens,
        const unsigned long num_hash_dims,
        const unsigned long offset,
        const unsigned long hash_seed
    )
    {
        std::pair<uint64,uint64> h[3];
        h[0] = make_pair(0,0);
        h[1] = make_pair(0,0);
        h[2] = make_pair(0,0);
        for (unsigned long i = range.first; i < range.second; ++i)
        {
            // shift h right 1
            h[2] = h[1];
            h[1] = h[0];
            h[0] = hash_string(tokens[i], hash_seed);

            std::pair<uint64,uint64> temp;
            double sign;

            // add a 1-gram feature
            sign = (h[0].second&1) ? +1 : -1;
            vect.push_back(make_pair<unsigned long>(h[0].first%num_hash_dims + offset, sign));

            if (i > range.first)
            {
                // add a 2-gram feature
                temp = murmur_hash3_128bit_3(h[0].first, h[1].first, 0);
                sign = (temp.second&1) ? +1 : -1;
                vect.push_back(make_pair<unsigned long>(temp.first%num_hash_dims + offset, sign));
            }
            if (i > range.first+1)
            {
                // add a 3-gram feature
                temp = murmur_hash3_128bit_3(h[0].first, h[1].first, h[2].first);
                sign = (temp.second&1) ? +1 : -1;
                vect.push_back(make_pair<unsigned long>(temp.first%num_hash_dims + offset, sign));
            }
        }
    }

// ----------------------------------------------------------------------------------------

    inline std::pair<unsigned long,double> make_feat (
        uint32 v1,
        uint32 v2,
        uint32 v3,
        const unsigned long num_hash_dims,
        const unsigned long offset
    )
    {
        std::pair<uint64,uint64> temp = murmur_hash3_128bit_3(v1,v2,v3);
        double sign = (temp.second&1) ? +1 : -1;
        return make_pair<unsigned long>(temp.first%num_hash_dims + offset, sign);
    }

// ----------------------------------------------------------------------------------------

    inline uint32 hash_range(
        const std::vector<std::string>& tokens,
        const std::pair<unsigned long, unsigned long>& range,
        const unsigned long hash_seed
    )
    {
        uint32 h = hash_seed;
        for (unsigned long i = range.first; i < range.second; ++i)
        {
            h = dlib::hash(tokens[i], h);
        }
        return h;
    }

}

// ----------------------------------------------------------------------------------------

namespace mitie
{

    binary_relation extract_binary_relation (
        const std::vector<std::string>& tokens,
        const std::pair<unsigned long, unsigned long>& rel_arg1,
        const std::pair<unsigned long, unsigned long>& rel_arg2,
        const total_word_feature_extractor& tfe
    )
    {
        DLIB_CASSERT(rel_arg1.first < rel_arg1.second && rel_arg1.second <= tokens.size(),"invalid inputs");
        DLIB_CASSERT(rel_arg2.first < rel_arg2.second && rel_arg2.second <= tokens.size(),"invalid inputs");

        // get dense word features for the two arguments.
        matrix<float,0,1> arg1, arg2, temp;
        for (unsigned long i = rel_arg1.first; i < rel_arg1.second; ++i)
        {
            tfe.get_feature_vector(tokens[i], temp);
            arg1 += temp;
        }
        arg1 /= (rel_arg1.second-rel_arg1.first);
        for (unsigned long i = rel_arg2.first; i < rel_arg2.second; ++i)
        {
            tfe.get_feature_vector(tokens[i], temp);
            arg2 += temp;
        }
        arg2 /= (rel_arg2.second-rel_arg2.first);
        // Put the dense vectors into the sparse format
        binary_relation rel;
        rel.total_word_feature_extractor_fingerprint = tfe.get_fingerprint();
        long offset = 0;
        for (long i = 0; i < arg1.size(); ++i)
            rel.feats.push_back(make_pair(offset + i, arg1(i)));
        offset += arg1.size();
        for (long i = 0; i < arg2.size(); ++i)
            rel.feats.push_back(make_pair(offset + i, arg2(i)));
        offset += arg2.size();




        typedef std::pair<unsigned long, unsigned long> range_t;
        range_t range1 = rel_arg1;
        range_t range2 = rel_arg2;
        unsigned int hash_seed = 0;
        if (range1.first > range2.first)
        {
            swap(range1, range2);
            // Use a different hash seed because that allows us to model the ordering of the
            // arguments. 
            hash_seed = 100000;
        }
        const unsigned int win = 2;
        range_t rbefore_first  = make_pair(range1.first>=win?range1.first-win:0, range1.first);
        range_t rbetween       = make_pair(min(range1.second,range2.second), max(range1.first,range2.first));
        range_t rafter_second  = make_pair(range2.second, range2.second+win<=tokens.size()?range2.second+win:tokens.size());
        const unsigned int win2 = 5;
        range_t rbefore_first2  = make_pair(range1.first>=win2?range1.first-win2:0, range1.first);
        range_t rafter_second2  = make_pair(range2.second, range2.second+win2<=tokens.size()?range2.second+win2:tokens.size());


        const long num_hash_dims = 100000;
        accum_123gram_feats(rel.feats, rbefore_first,  tokens, num_hash_dims, offset, hash_seed); ++hash_seed;
        accum_123gram_feats(rel.feats, rbefore_first2, tokens, num_hash_dims, offset, hash_seed); ++hash_seed;
        accum_123gram_feats(rel.feats, rbetween,       tokens, num_hash_dims, offset, hash_seed); ++hash_seed;
        accum_123gram_feats(rel.feats, rafter_second,  tokens, num_hash_dims, offset, hash_seed); ++hash_seed;
        accum_123gram_feats(rel.feats, rafter_second2, tokens, num_hash_dims, offset, hash_seed); ++hash_seed;

        const uint32 h1 = hash_range(tokens, rbefore_first, hash_seed);
        const uint32 h2 = hash_range(tokens, rbetween, hash_seed);
        const uint32 h3 = hash_range(tokens, rafter_second, hash_seed);

        rel.feats.push_back(make_feat(h1,h2,0,  num_hash_dims, offset));
        rel.feats.push_back(make_feat( 0,h2,0,  num_hash_dims, offset));
        rel.feats.push_back(make_feat( 0,h2,h3, num_hash_dims, offset));
        rel.feats.push_back(make_feat(h1,h2,h3, num_hash_dims, offset));

        make_sparse_vector_inplace(rel.feats);
        return rel;
    }

// ----------------------------------------------------------------------------------------

}


