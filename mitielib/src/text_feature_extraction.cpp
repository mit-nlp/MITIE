#include <mitie/text_feature_extraction.h>
#include <mitie/stemmer.h>

using namespace dlib;

namespace mitie
{
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
         * Here, we use the average word vector to represent the doc vector
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

// ----------------------------------------------------------------------------------------

    text_sample_type extract_BoW_features (
            const std::vector<std::string>& words
    )
    {
        DLIB_CASSERT(words.size() > 0, "words can't be empty");

        text_sample_type result;
        result.reserve(1000);

        /*
         * Here, we use the bag-of-words hashing vectorizer to represent the doc vector
         */

        std::vector<double> hash_vectorizer(10000, 0);

        for (unsigned long i = 0L; i < words.size(); ++i)
        {
            std::pair<uint64,uint64> hash = shash(words[i], 0);
            const double rand_sign = (hash.first&1) ? 1 : -1;
            unsigned long idx = hash.second % 10000;
            if (rand_sign == 1)
                hash_vectorizer[idx] += 1;
            else
                hash_vectorizer[idx] -= 1;
        }

        make_sparse_vector_inplace(result);
        // append on the dense part of the feature space
        for (long i = 0; i < hash_vectorizer.size(); ++i)
            result.push_back(make_pair(i+max_feat, hash_vectorizer[i]));

        hash_vectorizer.clear();
        return result;
    }

    // ----------------------------------------------------------------------------------------
    text_sample_type extract_combined_features (
            const std::vector<std::string>& words,
            const std::vector<matrix<float,0,1> >& feats
    )
    {
        text_sample_type result = extract_text_features(words, feats);
        text_sample_type result_BoW = extract_BoW_features(words);
        result.insert(result.end(), result_BoW.begin(), result_BoW.end());
        return result;
    }
}

