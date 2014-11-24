// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_MITIE_NeR_FEATURE_EXTRACTION_H_
#define MIT_LL_MITIE_NeR_FEATURE_EXTRACTION_H_

#include <map>
#include <string>
#include <vector>
#include <dlib/uintn.h>
#include <dlib/matrix.h>
#include <mitie/total_word_feature_extractor.h>

namespace mitie
{

// ----------------------------------------------------------------------------------------

    class ner_feature_extractor
    {

    public:
        typedef std::vector<dlib::matrix<float,0,1> > sequence_type;

        ner_feature_extractor() :num_feats(1) {}

        ner_feature_extractor (
            unsigned long num_feats_
        ) :
            num_feats(num_feats_)
        {}

        unsigned long num_feats;

        const static bool use_BIO_model           = false;
        const static bool use_high_order_features = false;
        const static bool allow_negative_weights  = true;

        unsigned long window_size()  const { return 3; }

        unsigned long num_features() const { return num_feats; }

        template <typename feature_setter>
        void get_features (
            feature_setter& set_feature,
            const sequence_type& sentence,
            unsigned long position
        ) const
        {
            const dlib::matrix<float,0,1>& feats = sentence[position];
            for (long i = 0; i < feats.size(); ++i)
                set_feature(i, feats(i));
        }
    };

    inline void serialize(const ner_feature_extractor& item, std::ostream& out) 
    {
        dlib::serialize(item.num_feats, out);
    }
    inline void deserialize(ner_feature_extractor& item, std::istream& in) 
    {
        dlib::deserialize(item.num_feats, in);
    }

// ----------------------------------------------------------------------------------------

    std::vector<dlib::matrix<float,0,1> > sentence_to_feats (
        const total_word_feature_extractor& fe,
        const std::vector<std::string>& sentence
    );
    /*!
        ensures
            - returns an array of vectors V such that:
                - V.size() == sentence.size()
                - for all valid i:
                    V[i] == the word feature vector for the word sentence[i]
    !*/

// ----------------------------------------------------------------------------------------

    typedef std::vector<std::pair<dlib::uint32,double> > ner_sample_type;

    ner_sample_type extract_ner_chunk_features (
        const std::vector<std::string>& words,
        const std::vector<dlib::matrix<float,0,1> >& feats,
        const std::pair<unsigned long, unsigned long>& chunk_range
    );
    /*!
        requires
            - words.size() == feats.size()
            - chunk_range.first < chunk_range.second
              (i.e. The chunk of words can't be empty)
        ensures
            - returns a sparse feature vector that describes the property of the range of
              words tarting with words[chunk_range.first] and ending just before
              words[chunk_range.second].  The feature vector will be suitable for
              predicting the type of named entity contained within this range. 
    !*/

// ----------------------------------------------------------------------------------------

}

#endif // MIT_LL_MITIE_NeR_FEATURE_EXTRACTION_H_

