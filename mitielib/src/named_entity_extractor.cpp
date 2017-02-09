// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include <mitie/named_entity_extractor.h>

using namespace dlib;

namespace mitie
{

// ----------------------------------------------------------------------------------------

    named_entity_extractor::
    named_entity_extractor(
        const std::vector<std::string>& tag_name_strings_,
        const total_word_feature_extractor& fe_,
        const dlib::sequence_segmenter<ner_feature_extractor>& segmenter_,
        const dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<ner_sample_type>,unsigned long>& df_
    ) : tag_name_strings(tag_name_strings_), fe(fe_), segmenter(segmenter_), df(df_),
        pure_model_version(get_max_supported_pure_model_version())
    { 
        // make sure the requirements are not violated.
        DLIB_CASSERT(df.number_of_classes() >= tag_name_strings.size(),"invalid inputs"); 
        DLIB_CASSERT(segmenter.get_feature_extractor().num_features() == fe.get_num_dimensions(),"invalid inputs"); 
        std::set<unsigned long> df_tags(df.get_labels().begin(), df.get_labels().end());
        for (unsigned long i = 0; i < tag_name_strings.size(); ++i)
        {
            DLIB_CASSERT(df_tags.count(i) == 1, "The classifier must be capable of predicting each possible tag as output.");
        }
        tfe_fingerprint = fe.get_fingerprint();
        compute_fingerprint();
    }

    named_entity_extractor::
    named_entity_extractor(const std::string& pureModelName,
                           const std::string& extractorName
    )
    {
        std::string classname;
        dlib::proxy_deserialize stream_wrap = dlib::deserialize(pureModelName);
        stream_wrap >> classname;
        if(classname == "mitie::named_entity_extractor_pure_model") {
            pure_model_version = pure_model_version_0;
        } else if (classname == "mitie::named_entity_extractor_pure_model_with_version") {
            stream_wrap >> pure_model_version;
        } else {
            throw dlib::error(
                    "This file does not contain a mitie::named_entity_extractor_pure_model. Contained: " + classname);
        }

        switch(pure_model_version)
        {
            case pure_model_version_0:
                stream_wrap >> df >> segmenter >> tag_name_strings;
                break;

            case pure_model_version_1:
                stream_wrap >> df >> segmenter >> tag_name_strings >> tfe_fingerprint;
                break;

            default:
                throw dlib::error(
                        "Unsupported version of pure model found. "
                        "Found: " + dlib::cast_to_string(pure_model_version) +
                        "Supported upto : " + dlib::cast_to_string(get_max_supported_pure_model_version()));
        }


        dlib::deserialize(extractorName) >> classname;
        if (classname != "mitie::total_word_feature_extractor")
            throw dlib::error(
                    "This file does not contain a mitie::total_word_feature_extractor. Contained: " + classname);

        dlib::deserialize(extractorName) >> classname >> fe;

        if(pure_model_version != pure_model_version_0 && tfe_fingerprint != fe.get_fingerprint())
            throw dlib::error(
                    "Fingerprint mismatch. "
                    "Feature extractor must be same as the one used for training the model");

        compute_fingerprint();
    }

    named_entity_extractor::
    named_entity_extractor(const std::string& pureModelName
    )
    {
        std::string classname;
        dlib::proxy_deserialize stream_wrap = dlib::deserialize(pureModelName);
        stream_wrap >> classname;
        if(classname == "mitie::named_entity_extractor_pure_model") {
            pure_model_version = pure_model_version_0;
        } else if (classname == "mitie::named_entity_extractor_pure_model_with_version") {
            stream_wrap >> pure_model_version;
        } else {
            throw dlib::error(
                    "This file does not contain a mitie::named_entity_extractor_pure_model. Contained: " + classname);
        }

        switch(pure_model_version)
        {
            case pure_model_version_0:
                stream_wrap >> df >> segmenter >> tag_name_strings;
                break;

            case pure_model_version_1:
                stream_wrap >> df >> segmenter >> tag_name_strings >> tfe_fingerprint;
                break;

            default:
                throw dlib::error(
                        "Unsupported version of pure model found. "
                        "Found: " + dlib::cast_to_string(pure_model_version) +
                        "Supported upto : " + dlib::cast_to_string(get_max_supported_pure_model_version()));
        }
        compute_fingerprint();
    }
// ----------------------------------------------------------------------------------------

    void named_entity_extractor::
    predict (
        const std::vector<std::string>& sentence,
        std::vector<std::pair<unsigned long, unsigned long> >& chunks,
        std::vector<unsigned long>& chunk_tags,
        std::vector<double>& chunk_scores
    ) const
    {
        predict(sentence, chunks, chunk_tags, chunk_scores, fe);
    }

    void named_entity_extractor::
    predict (
        const std::vector<std::string>& sentence,
        std::vector<std::pair<unsigned long, unsigned long> >& chunks,
        std::vector<unsigned long>& chunk_tags,
        std::vector<double>& chunk_scores,
        const total_word_feature_extractor& fe
    ) const
    {
        if(pure_model_version != pure_model_version_0 && this->tfe_fingerprint != fe.get_fingerprint())
        {
            throw dlib::error(
                    "Fingerprint mismatch. "
                    "Feature extractor must be same as the one used for training the model");
        }
        const std::vector<matrix<float,0,1> >& sent = sentence_to_feats(fe, sentence);
        segmenter.segment_sequence(sent, chunks);


        std::vector<std::pair<unsigned long, unsigned long> > final_chunks;
        final_chunks.reserve(chunks.size());
        chunk_tags.clear();
        chunk_scores.clear();
        // now label each chunk
        for (unsigned long j = 0; j < chunks.size(); ++j)
        {
            const std::pair<unsigned long, double> temp = df.predict(extract_ner_chunk_features(sentence, sent, chunks[j]));
            const unsigned long tag = temp.first;
            const double score = temp.second;

            // Only output this chunk if it is predicted to be an entity.  Recall that if
            // the classifier outputs a ID outside the range of our labels then it's
            // predicting "this isn't an entity at all".
            if (tag < tag_name_strings.size())
            {
                final_chunks.push_back(chunks[j]);
                chunk_tags.push_back(tag);
                chunk_scores.push_back(score);
            }
        }

        final_chunks.swap(chunks);
    }

// ----------------------------------------------------------------------------------------

    void named_entity_extractor::
    operator() (
        const std::vector<std::string>& sentence,
        std::vector<std::pair<unsigned long, unsigned long> >& chunks,
        std::vector<unsigned long>& chunk_tags
    ) const
    {
        (*this).operator ()(sentence, chunks, chunk_tags, fe);
    }

    void named_entity_extractor::
    operator() (
        const std::vector<std::string>& sentence,
        std::vector<std::pair<unsigned long, unsigned long> >& chunks,
        std::vector<unsigned long>& chunk_tags,
        const total_word_feature_extractor& fe
    ) const
    {
        if(pure_model_version != pure_model_version_0 && this->tfe_fingerprint != fe.get_fingerprint())
        {
            throw dlib::error(
                    "Fingerprint mismatch. "
                    "Feature extractor must be same as the one used for training the model");
        }
        const std::vector<matrix<float,0,1> >& sent = sentence_to_feats(fe, sentence);
        segmenter.segment_sequence(sent, chunks);


        std::vector<std::pair<unsigned long, unsigned long> > final_chunks;
        final_chunks.reserve(chunks.size());
        chunk_tags.clear();
        // now label each chunk
        for (unsigned long j = 0; j < chunks.size(); ++j)
        {
            const unsigned long tag = df(extract_ner_chunk_features(sentence, sent, chunks[j]));

            // Only output this chunk if it is predicted to be an entity.  Recall that if
            // the classifier outputs a ID outside the range of our labels then it's
            // predicting "this isn't an entity at all".
            if (tag < tag_name_strings.size())
            {
                final_chunks.push_back(chunks[j]);
                chunk_tags.push_back(tag);
            }
        }

        final_chunks.swap(chunks);
    }

// ----------------------------------------------------------------------------------------

}

