// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_WORD_MORPHOLOGY_FEATURE_ExTRACTOR_H_
#define MIT_LL_WORD_MORPHOLOGY_FEATURE_ExTRACTOR_H_

#include "approximate_substring_set.h"
#include <dlib/matrix.h>

namespace mitie
{
    class word_morphology_feature_extractor
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool for turning a word into a short and dense vector which describes
                what kind of places in text a word can appear.  This is done based purely on
                morphological features of the word.

            THREAD SAFETY
                Note that this object uses mutable internal scratch space.  Therefore, it is
                unsafe for two threads to touch the same instance of this object at a time
                without mutex locking it first.
        !*/

    public:
        word_morphology_feature_extractor(){}
        /*!
            ensures
                - #get_num_dimensions() == 0
        !*/

        word_morphology_feature_extractor (
            const approximate_substring_set& substrings_,
            const dlib::matrix<float>& morph_trans_
        ) :
            substrings(substrings_),
            morph_trans(morph_trans_) 
        {}

        unsigned long get_num_dimensions(
        ) const
        /*!
            ensures
                - returns the dimensionality of the feature vectors produced by this
                  object.
        !*/
        {
            return morph_trans.nc();
        }

        void get_feature_vector (
            const char* begin,
            const char* end,
            dlib::matrix<float,0,1>& feats
        ) const
        /*!
            requires
                - begin <= end
                - get_num_dimensions() != 0
            ensures
                - Extracts a dense word morphology based feature vector from the string 
                  in the half open range [begin, end) and stores it into #feats.
                - #feats.size() == get_num_dimensions()
        !*/
        {
            substrings.find_substrings(begin, end, hits);
            hits_to_vect(hits, feats);
        }

        void premultiply_vectors_by (
            double value
        )
        /*!
            requires
                - all subsequent calls to get_feature_vector() will output features that 
                  are value times the previous feature vectors.
        !*/
        {
            morph_trans *= value;
        }

        void get_feature_vector (
            const std::string& word,
            dlib::matrix<float,0,1>& feats
        ) const
        /*!
            requires
                - get_num_dimensions() != 0
            ensures
                - This function is identical to the above get_feature_vector() routine
                  except that it takes its input string as a std::string instead of an
                  iterator range.
        !*/
        {
            substrings.find_substrings(word, hits);
            hits_to_vect(hits, feats);
        }

        friend void serialize (const word_morphology_feature_extractor& item, std::ostream& out)
        {
            int version = 1;
            dlib::serialize(version, out);
            serialize(item.substrings, out);
            dlib::serialize(item.morph_trans, out);
        }

        friend void deserialize (word_morphology_feature_extractor& item, std::istream& in)
        {
            int version = 0;
            dlib::deserialize(version, in);
            if (version != 1)
                throw dlib::serialization_error("Unexpected version found while deserializing mitie::word_morphology_feature_extractor");
            deserialize(item.substrings, in);
            dlib::deserialize(item.morph_trans, in);
        }

    // ------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------
    //                                  IMPLEMENTATION DETAILS
    // ------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------

    private:

        void hits_to_vect (
            const std::vector<dlib::uint16>& hits,
            dlib::matrix<float,0,1>& feats
        ) const
        /*!
            ensures
                - This function just treats hits like a sparse vector and performs
                  the equivalent to:
                    - feats = trans(morph_trans)*hits
        !*/
        {
            if (hits.size() == 0)
            {
                feats.set_size(morph_trans.nc());
                feats = 0;
            }
            else
            {
                feats = trans(rowm(morph_trans,hits[0]));
                for (unsigned long i = 1; i < hits.size(); ++i)
                    feats += trans(rowm(morph_trans,hits[i]));
            }
        }

        approximate_substring_set substrings;
        dlib::matrix<float> morph_trans;

        // This member doesn't logically contribute to the state of this object.  It is here
        // only to avoid reallocating it over and over every time get_feature_vector() is
        // called.
        mutable std::vector<dlib::uint16> hits;
    };
}

#endif // MIT_LL_WORD_MORPHOLOGY_FEATURE_ExTRACTOR_H_

