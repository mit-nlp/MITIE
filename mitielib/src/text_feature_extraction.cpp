#include <mitie/text_feature_extraction.h>
#include <mitie/stemmer.h>

using namespace dlib;

namespace mitie
{

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
        }
        all_sum /= words.size();

        for (long i = 0; i < all_sum.size(); ++i)
            result.push_back(make_pair(i+MAX_FEAT, all_sum(i)));

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

        for (unsigned long i = 0L; i < words.size(); ++i)
        {
            result.push_back(make_feat(shash(words[i],0)));
            result.push_back(make_feat(shash(stem_word(words[i]),10)));
        }

        make_sparse_vector_inplace(result);
        return result;
    }

    // ----------------------------------------------------------------------------------------
    text_sample_type extract_combined_features (
            const std::vector<std::string>& words,
            const std::vector<matrix<float,0,1> >& feats
    )
    {
        text_sample_type result_BoW = extract_BoW_features(words);
        text_sample_type result_text = extract_text_features(words, feats);

        result_BoW.insert(result_BoW.end(), result_text.begin(), result_text.end());
        return result_BoW;
    }
}

