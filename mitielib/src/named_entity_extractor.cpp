// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis.king@ll.mit.edu)

#include <mitie/named_entity_extractor.h>
#include <set>

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
    ) : tag_name_strings(tag_name_strings_), fe(fe_), segmenter(segmenter_), df(df_) 
    { 
        // make sure the requirements are not violated.
        DLIB_CASSERT(df.number_of_classes() == tag_name_strings.size()+1,"invalid inputs"); 
        DLIB_CASSERT(segmenter.get_feature_extractor().num_features() == fe.get_num_dimensions(),"invalid inputs"); 
        std::set<unsigned long> df_tags(df.get_labels().begin(), df.get_labels().end());
        for (unsigned long i = 0; i <= tag_name_strings.size(); ++i)
        {
            DLIB_CASSERT(df_tags.count(i) == 1, "invalid inputs");
        }

        compute_fingerprint();
    }

// ----------------------------------------------------------------------------------------

    void named_entity_extractor::
    operator() (
        const std::vector<std::string>& sentence,
        std::vector<std::pair<unsigned long, unsigned long> >& chunks,
        std::vector<unsigned long>& chunk_tags
    ) const
    {
        const std::vector<matrix<float,0,1> >& sent = sentence_to_feats(fe, sentence);
        segmenter.segment_sequence(sent, chunks);

        const unsigned long NOT_AN_ENTITY = df.number_of_classes()-1;

        std::vector<std::pair<unsigned long, unsigned long> > final_chunks;
        final_chunks.reserve(chunks.size());
        chunk_tags.clear();
        // now label each chunk
        for (unsigned long j = 0; j < chunks.size(); ++j)
        {
            const unsigned long tag = df(extract_ner_chunk_features(sentence, sent, chunks[j]));

            // if this chunk is predicted to not be an entity then don't output it 
            if (tag != NOT_AN_ENTITY)
            {
                final_chunks.push_back(chunks[j]);
                chunk_tags.push_back(tag);
            }
        }

        final_chunks.swap(chunks);
    }

// ----------------------------------------------------------------------------------------

}

