#include <mitie/text_feature_extraction.h>
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
        return std::make_pair((dlib::uint32)(hash.second%max_feat), rand_sign*feat_weight);
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

    inline std::pair<uint64,uint64> ifeat (
            const uint32 seed
    )
    {
        return murmur_hash3_128bit_3(seed,0,0);
    }

// ----------------------------------------------------------------------------------------

    text_sample_type extract_text_features (
            const std::vector<std::string>& words,
            const std::vector<matrix<float,0,1> >& feats
    )
    {
        DLIB_CASSERT(words.size() == feats.size(), "range can't be empty");

        text_sample_type result;
        result.reserve(1000);

        /*
         * TODO: add CBOW or other schemes to better represent the document vector by reserving
         * the sequence, word occurrence, and/or other useful information
         *
         * Currently, we simply use the average word vector to represent the doc vector
         */
        matrix<float,0,1> all_sum;
        for (unsigned long i = 0L; i < words.size(); ++i)
        {
            all_sum += feats[i];
            result.push_back(make_feat(shash(words[i],0)));
            result.push_back(make_feat(shash(stem_word(words[i]),10)));
        }
        all_sum /= words.size();

        make_sparse_vector_inplace(result);
        // append on the dense part of the feature space
        for (long i = 0; i < all_sum.size(); ++i)
            result.push_back(make_pair(i+max_feat, all_sum(i)));

        return result;
    }
}

