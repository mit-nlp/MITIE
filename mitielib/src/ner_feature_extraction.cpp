// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis.king@ll.mit.edu)


#include <mitie/ner_feature_extraction.h>
#include <dlib/sparse_vector.h>
#include <dlib/hash.h>
#include <mitie/stemmer.h>

using namespace dlib;

namespace mitie
{
    std::vector<matrix<float,0,1> > sentence_to_feats (
        const total_word_feature_extractor& fe,
        const std::vector<std::string>& sentence
    )
    {
        std::vector<matrix<float,0,1> > temp;
        temp.resize(sentence.size());
        for (unsigned long i = 0; i < sentence.size(); ++i)
            fe.get_feature_vector(sentence[i], temp[i]);
        return temp;
    }

// ----------------------------------------------------------------------------------------

    const unsigned long max_feat = 500000;
    inline std::pair<dlib::uint32,double> make_feat (
        const std::pair<uint64,uint64>& hash
    )
    {
        const double feat_weight = 1.5;
        const double rand_sign = (hash.first&1) ? 1 : -1;
        return std::make_pair(hash.second%max_feat, rand_sign*feat_weight);
    }

    inline std::pair<uint64,uint64> shash ( 
        const std::string& word,
        const uint32 seed 
    )
    {
        if (word.size() == 0)
            return make_pair(0,0);
        return murmur_hash3_128bit(&word[0], word.size(), seed);
    }

    inline std::pair<uint64,uint64> prefix ( 
        const std::string& word,
        const uint32 seed 
    )
    {
        if (word.size() == 0)
            return make_pair(0,0);
        dlib::uint32 l1 = 0, l2 = 0, l3 = 0;
        if (word.size() > 0)
            l1 = word[0];
        if (word.size() > 1)
            l2 = word[1];
        if (word.size() > 2)
            l3 = word[2];
        return murmur_hash3_128bit(l1,l2,l3,seed);
    }

    inline std::pair<uint64,uint64> suffix ( 
        const std::string& word,
        const uint32 seed 
    )
    {
        if (word.size() == 0)
            return make_pair(0,0);
        dlib::uint32 l1 = 0, l2 = 0, l3 = 0;
        if (word.size() > 0)
            l1 = word[word.size()-1];
        if (word.size() > 1)
            l2 = word[word.size()-2];
        if (word.size() > 2)
            l3 = word[word.size()-3];
        return murmur_hash3_128bit(l1,l2,l3,seed);
    }

    inline std::pair<uint64,uint64> ifeat ( 
        const uint32 seed 
    )
    {
        return murmur_hash3_128bit_3(seed,0,0);
    }

// ----------------------------------------------------------------------------------------

    bool is_caps ( const std::string& word)  
    {
        return (word.size() != 0 && 'A' <= word[0] && word[0] <= 'Z');
    }

    bool is_all_caps ( const std::string& word)  
    {
        for (unsigned long i = 0; i < word.size(); ++i)
        {
            if (!('A' <= word[i] && word[i] <= 'Z'))
                return false;
        }
        return true;
    }

    bool contains_numbers ( const std::string& word)  
    {
        for (unsigned long i = 0; i < word.size(); ++i)
        {
            if ('0' <= word[i] && word[i] <= '9')
                return true;
        }
        return false;
    }

    bool contains_letters ( const std::string& word)  
    {
        for (unsigned long i = 0; i < word.size(); ++i)
        {
            if ('a' <= word[i] && word[i] <= 'z')
                return true;
            if ('A' <= word[i] && word[i] <= 'Z')
                return true;
        }
        return false;
    }

    bool contains_letters_and_numbers ( const std::string& word)  
    {
        return contains_letters(word) && contains_numbers(word);
    }

    bool is_all_numbers ( const std::string& word)  
    {
        for (unsigned long i = 0; i < word.size(); ++i)
        {
            if (!('0' <= word[i] && word[i] <= '9'))
                return false;
        }
        return true;
    }

    bool contains_hyphen ( const std::string& word)  
    {
        for (unsigned long i = 0; i < word.size(); ++i)
        {
            if (word[i] == '-')
                return true;
        }
        return false;
    }

    inline std::pair<uint64,uint64> caps_pattern ( 
        const std::vector<std::string>& words,
        const std::pair<unsigned long, unsigned long>& pos
    ) 
    {
        unsigned long val = 0;
        if (pos.first != 0 && is_caps(words[pos.first-1])) 
            val |= 1;
        if (is_caps(words[pos.first])) 
            val |= 1;
        if (is_caps(words[pos.second-1])) 
            val |= 1;
        if (pos.second < words.size() && is_caps(words[pos.second]))
            val |= 1;

        return murmur_hash3_128bit_3(val,12345,5739453);
    }

    typedef std::vector<std::pair<dlib::uint32,double> > ner_sample_type;

    ner_sample_type extract_ner_chunk_features (
        const std::vector<std::string>& words,
        const std::vector<matrix<float,0,1> >& feats,
        const std::pair<unsigned long, unsigned long>& chunk_range
    )
    {
        DLIB_CASSERT(words.size() == feats.size(), "range can't be empty");
        DLIB_CASSERT(chunk_range.first != chunk_range.second, "range can't be empty");


        ner_sample_type result;
        result.reserve(1000);

        const std::pair<unsigned long, unsigned long> wide_range(
            std::max(0L, (long)chunk_range.first-8),
            std::min(words.size(), chunk_range.second+8));
        for (unsigned long i = wide_range.first; i < chunk_range.first; ++i)
            result.push_back(make_feat(shash(words[i],1000)));
        for (unsigned long i = chunk_range.second; i < wide_range.second; ++i)
            result.push_back(make_feat(shash(words[i],1001)));

        matrix<float,0,1> all_sum;
        for (unsigned long i = chunk_range.first; i < chunk_range.second; ++i)
        {
            all_sum += feats[i];
            result.push_back(make_feat(shash(words[i],0)));
            result.push_back(make_feat(shash(stem_word(words[i]),10)));

            if (is_caps(words[i]))                      result.push_back(make_feat(ifeat(21)));
            if (is_all_caps(words[i]))                  result.push_back(make_feat(ifeat(22)));
            if (contains_numbers(words[i]))             result.push_back(make_feat(ifeat(23)));
            if (contains_letters(words[i]))             result.push_back(make_feat(ifeat(24)));
            if (contains_letters_and_numbers(words[i])) result.push_back(make_feat(ifeat(25)));
            if (is_all_numbers(words[i]))               result.push_back(make_feat(ifeat(26)));
            if (contains_hyphen(words[i]))              result.push_back(make_feat(ifeat(27)));

            result.push_back(make_feat(prefix(words[i],50)));
            result.push_back(make_feat(suffix(words[i],51)));
        }
        all_sum /= chunk_range.second-chunk_range.first;

        result.push_back(make_feat(caps_pattern(words, chunk_range)));

        matrix<float,0,1> first = feats[chunk_range.first];
        matrix<float,0,1> last = feats[chunk_range.second-1];

        result.push_back(make_feat(shash(words[chunk_range.first],1)));
        result.push_back(make_feat(shash(words[chunk_range.second-1],2)));
        result.push_back(make_feat(shash(stem_word(words[chunk_range.first]),11)));
        result.push_back(make_feat(shash(stem_word(words[chunk_range.second-1]),12)));
        result.push_back(make_feat(prefix(words[chunk_range.first],52)));
        result.push_back(make_feat(suffix(words[chunk_range.first],53)));
        result.push_back(make_feat(prefix(words[chunk_range.second-1],54)));
        result.push_back(make_feat(suffix(words[chunk_range.second-1],55)));

        if (is_caps(words[chunk_range.first]))                      result.push_back(make_feat(ifeat(27)));
        if (is_all_caps(words[chunk_range.first]))                  result.push_back(make_feat(ifeat(28)));
        if (contains_numbers(words[chunk_range.first]))             result.push_back(make_feat(ifeat(29)));
        if (contains_letters(words[chunk_range.first]))             result.push_back(make_feat(ifeat(30)));
        if (contains_letters_and_numbers(words[chunk_range.first])) result.push_back(make_feat(ifeat(31)));
        if (is_all_numbers(words[chunk_range.first]))               result.push_back(make_feat(ifeat(32)));
        if (contains_hyphen(words[chunk_range.first]))              result.push_back(make_feat(ifeat(33)));

        if (is_caps(words[chunk_range.second-1]))                      result.push_back(make_feat(ifeat(34)));
        if (is_all_caps(words[chunk_range.second-1]))                  result.push_back(make_feat(ifeat(35)));
        if (contains_numbers(words[chunk_range.second-1]))             result.push_back(make_feat(ifeat(36)));
        if (contains_letters(words[chunk_range.second-1]))             result.push_back(make_feat(ifeat(37)));
        if (contains_letters_and_numbers(words[chunk_range.second-1])) result.push_back(make_feat(ifeat(38)));
        if (is_all_numbers(words[chunk_range.second-1]))               result.push_back(make_feat(ifeat(39)));
        if (contains_hyphen(words[chunk_range.second-1]))              result.push_back(make_feat(ifeat(40)));

        matrix<float,0,1> before, after;
        if (chunk_range.first != 0)
        {
            before = feats[chunk_range.first-1];
            result.push_back(make_feat(shash(words[chunk_range.first-1],3)));
            result.push_back(make_feat(shash(stem_word(words[chunk_range.first-1]),13)));

            result.push_back(make_feat(prefix(words[chunk_range.first-1],56)));
            result.push_back(make_feat(suffix(words[chunk_range.first-1],57)));

            if (is_caps(words[chunk_range.first-1]))                      result.push_back(make_feat(ifeat(60)));
            if (is_all_caps(words[chunk_range.first-1]))                  result.push_back(make_feat(ifeat(61)));
            if (contains_numbers(words[chunk_range.first-1]))             result.push_back(make_feat(ifeat(62)));
            if (contains_letters(words[chunk_range.first-1]))             result.push_back(make_feat(ifeat(63)));
            if (contains_letters_and_numbers(words[chunk_range.first-1])) result.push_back(make_feat(ifeat(64)));
            if (is_all_numbers(words[chunk_range.first-1]))               result.push_back(make_feat(ifeat(65)));
            if (contains_hyphen(words[chunk_range.first-1]))              result.push_back(make_feat(ifeat(66)));
        }
        else
        {
            before = zeros_matrix<float>(first.size(),1);
        }

        if (chunk_range.first > 1)
        {
            result.push_back(make_feat(shash(words[chunk_range.first-2],103)));
            result.push_back(make_feat(shash(stem_word(words[chunk_range.first-2]),113)));

            result.push_back(make_feat(prefix(words[chunk_range.first-2],156)));
            result.push_back(make_feat(suffix(words[chunk_range.first-2],157)));

            if (is_caps(words[chunk_range.first-2]))                      result.push_back(make_feat(ifeat(160)));
            if (is_all_caps(words[chunk_range.first-2]))                  result.push_back(make_feat(ifeat(161)));
            if (contains_numbers(words[chunk_range.first-2]))             result.push_back(make_feat(ifeat(162)));
            if (contains_letters(words[chunk_range.first-2]))             result.push_back(make_feat(ifeat(163)));
            if (contains_letters_and_numbers(words[chunk_range.first-2])) result.push_back(make_feat(ifeat(164)));
            if (is_all_numbers(words[chunk_range.first-2]))               result.push_back(make_feat(ifeat(165)));
            if (contains_hyphen(words[chunk_range.first-2]))              result.push_back(make_feat(ifeat(166)));
        }

        if (chunk_range.second+1 < feats.size())
        {
            result.push_back(make_feat(shash(words[chunk_range.second+1],104)));
            result.push_back(make_feat(shash(stem_word(words[chunk_range.second+1]),114)));

            result.push_back(make_feat(prefix(words[chunk_range.second+1],158)));
            result.push_back(make_feat(suffix(words[chunk_range.second+1],159)));

            if (is_caps(words[chunk_range.second+1]))                      result.push_back(make_feat(ifeat(167)));
            if (is_all_caps(words[chunk_range.second+1]))                  result.push_back(make_feat(ifeat(168)));
            if (contains_numbers(words[chunk_range.second+1]))             result.push_back(make_feat(ifeat(169)));
            if (contains_letters(words[chunk_range.second+1]))             result.push_back(make_feat(ifeat(170)));
            if (contains_letters_and_numbers(words[chunk_range.second+1])) result.push_back(make_feat(ifeat(171)));
            if (is_all_numbers(words[chunk_range.second+1]))               result.push_back(make_feat(ifeat(172)));
            if (contains_hyphen(words[chunk_range.second+1]))              result.push_back(make_feat(ifeat(173)));
        }

        if (chunk_range.second < feats.size())
        {
            after = feats[chunk_range.second];
            result.push_back(make_feat(shash(words[chunk_range.second],4)));
            result.push_back(make_feat(shash(stem_word(words[chunk_range.second]),14)));

            result.push_back(make_feat(prefix(words[chunk_range.second],58)));
            result.push_back(make_feat(suffix(words[chunk_range.second],59)));

            if (is_caps(words[chunk_range.second]))                      result.push_back(make_feat(ifeat(67)));
            if (is_all_caps(words[chunk_range.second]))                  result.push_back(make_feat(ifeat(68)));
            if (contains_numbers(words[chunk_range.second]))             result.push_back(make_feat(ifeat(69)));
            if (contains_letters(words[chunk_range.second]))             result.push_back(make_feat(ifeat(70)));
            if (contains_letters_and_numbers(words[chunk_range.second])) result.push_back(make_feat(ifeat(71)));
            if (is_all_numbers(words[chunk_range.second]))               result.push_back(make_feat(ifeat(72)));
            if (contains_hyphen(words[chunk_range.second]))              result.push_back(make_feat(ifeat(73)));
        }
        else
        {
            after = zeros_matrix<float>(first.size(),1);
        }

        const double lnorm = 0.5;
        first /= lnorm*length(first)+1e-10;
        last /= lnorm*length(last)+1e-10;
        all_sum /= lnorm*length(all_sum)+1e-10;
        before /= lnorm*length(before)+1e-10;
        after /= lnorm*length(after)+1e-10;

        matrix<double,0,1> temp = matrix_cast<double>(join_cols(join_cols(join_cols(join_cols(first,last),all_sum),before),after));

        make_sparse_vector_inplace(result);
        // append on the dense part of the feature space
        for (long i = 0; i < temp.size(); ++i)
            result.push_back(make_pair(i+max_feat, temp(i)));

        return result;
    }

}

