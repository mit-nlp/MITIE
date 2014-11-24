// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_BINARY_rELATION_DETECTION_H_
#define MIT_LL_BINARY_rELATION_DETECTION_H_

#include <dlib/hash.h>
#include <dlib/svm.h>
#include <dlib/serialize.h>
#include <vector>
#include <mitie/total_word_feature_extractor.h>

namespace mitie
{

// ----------------------------------------------------------------------------------------

    typedef std::vector<std::pair<unsigned long,double> > sparse_vector_type;

// ----------------------------------------------------------------------------------------

    struct binary_relation
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a simple container for a binary relation feature vector and the
                fingerprint for the total_word_feature_extractor that was used to generate
                the feature vector.
        !*/

        sparse_vector_type feats;
        dlib::uint64 total_word_feature_extractor_fingerprint;
    };

    binary_relation extract_binary_relation (
        const std::vector<std::string>& tokens,
        const std::pair<unsigned long, unsigned long>& rel_arg1,
        const std::pair<unsigned long, unsigned long>& rel_arg2,
        const total_word_feature_extractor& tfe
    );
    /*!
        requires
            - rel_arg1.first < rel_arg1.second <= tokens.size()
            - rel_arg2.first < rel_arg2.second <= tokens.size()
        ensures
            - returns a binary_relation BR such that:
                - BR.total_word_feature_extractor_fingerprint == tfe.get_fingerprint()
                - BR.feats == a feature vector that describes the relation given by the two
                  relation argument positions rel_arg1 and rel_arg2.  rel_arg1 and rel_arg2
                  are interpreted as half open ranges in tokens.
    !*/

// ----------------------------------------------------------------------------------------

    struct binary_relation_detector 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a simple container for a binary classifier and the fingerprint 
                for the total_word_feature_extractor it is designed to use.
        !*/

        binary_relation_detector(
        ) : total_word_feature_extractor_fingerprint(0) {}

        std::string relation_type;
        dlib::uint64 total_word_feature_extractor_fingerprint;
        dlib::decision_function<dlib::sparse_linear_kernel<sparse_vector_type> > df;

        double operator() (
            const binary_relation& rel
        ) const
        {
            if (rel.total_word_feature_extractor_fingerprint != total_word_feature_extractor_fingerprint)
                throw dlib::error("Incompatible total_word_feature_extractor used with binary_relation_detector.");
            return df(rel.feats);
        }
    };

    inline void serialize(
        const binary_relation_detector& item,
        std::ostream& out
    )
    {
        int version = 1;
        dlib::serialize(version, out);
        dlib::serialize(item.relation_type, out);
        dlib::serialize(item.total_word_feature_extractor_fingerprint, out);
        dlib::serialize(item.df, out);
    }

    inline void deserialize(
        binary_relation_detector& item,
        std::istream& in
    )
    {
        int version = 0;
        dlib::deserialize(version, in);
        if (version != 1)
            throw dlib::serialization_error("Unexpected version found while deserializing mitie::binary_relation_detector.");
        dlib::deserialize(item.relation_type, in);
        dlib::deserialize(item.total_word_feature_extractor_fingerprint, in);
        dlib::deserialize(item.df, in);
    }

// ----------------------------------------------------------------------------------------

}

#endif // MIT_LL_BINARY_rELATION_DETECTION_H_
