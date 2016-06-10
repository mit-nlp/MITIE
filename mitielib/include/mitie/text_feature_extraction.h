#ifndef MIT_LL_MITIE_TexT_FEATURE_EXTRACTION_H_
#define MIT_LL_MITIE_TexT_FEATURE_EXTRACTION_H_

#include <map>
#include <string>
#include <vector>
#include <dlib/uintn.h>
#include <dlib/matrix.h>
#include <mitie/total_word_feature_extractor.h>
#include <mitie/ner_feature_extraction.h>

namespace mitie
{

// ----------------------------------------------------------------------------------------

    class text_feature_extractor
    {

    public:
        text_feature_extractor() :num_feats(1) {}

        text_feature_extractor (
            unsigned long num_feats_
        ) :
            num_feats(num_feats_)
        {}

        unsigned long num_feats;

        unsigned long num_features() const { return num_feats; }

    };

    inline void serialize(const text_feature_extractor& item, std::ostream& out)
    {
        dlib::serialize(item.num_feats, out);
    }
    inline void deserialize(text_feature_extractor& item, std::istream& in)
    {
        dlib::deserialize(item.num_feats, in);
    }

// ----------------------------------------------------------------------------------------

    typedef std::vector<std::pair<dlib::uint32, double> > text_sample_type;

    text_sample_type extract_text_features (
        const std::vector<std::string>& words,
        const std::vector<dlib::matrix<float,0,1> >& feats
    );
    /*!
        requires
            - words.size() == feats.size()
        ensures
            - returns a sparse feature vector that describes the property of the range of
              words starting with first word and ending just at the last word.
              The feature vector can be suitable for predicting the type of document.
    !*/

// ----------------------------------------------------------------------------------------
    text_sample_type extract_BoW_features (
            const std::vector<std::string>& words
    );
    /*!
        ensures
            - returns a sparse feature vector using bag-of-words to describes
              the property of the range of words starting with first word and
              ending just at the last word.
              The feature vector can be suitable for predicting the type of document.
    !*/

// ----------------------------------------------------------------------------------------
    text_sample_type extract_combined_features (
            const std::vector<std::string>& words,
            const std::vector<dlib::matrix<float,0,1> >& feats
    );
    /*!
        ensures
            - returns a feature vector that append bag-of-words hashing vector with
              total words feature vectors together, to represent the document.
              The feature vector is expected to be more suitable for predicting the
              type of document.
    !*/

// ----------------------------------------------------------------------------------------
}

#endif // MIT_LL_MITIE_TexT_FEATURE_EXTRACTION_H_

