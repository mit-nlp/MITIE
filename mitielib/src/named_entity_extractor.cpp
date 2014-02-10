// Created by Davis E. King on Feb 10, 2014

#include <mitie/named_entity_extractor.h>
#include <set>

using namespace dlib;

namespace mitie
{

// ----------------------------------------------------------------------------------------

    named_entity_extractor::
    named_entity_extractor(
        const std::map<unsigned long, std::string>& possible_tags_,
        const total_word_feature_extractor& fe_,
        const dlib::sequence_segmenter<ner_feature_extractor>& segmenter_,
        const dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<ner_sample_type>,unsigned long>& df_
    ) : possible_tags(possible_tags_), fe(fe_), segmenter(segmenter_), df(df_) 
    { 
        // make sure the requirements are not violated.
        DLIB_CASSERT(df.number_of_classes() == possible_tags.size()+1,"invalid inputs"); 
        DLIB_CASSERT(segmenter.get_feature_extractor().num_features() == fe.get_num_dimensions(),"invalid inputs"); 
        std::set<unsigned long> df_tags(df.get_labels().begin(), df.get_labels().end());
        for (unsigned long i = 0; i <= possible_tags.size(); ++i)
        {
            DLIB_CASSERT(df_tags.count(i) == 1, "invalid inputs");
            if (i < possible_tags.size())
            {
                DLIB_CASSERT(possible_tags.count(i) == 1, "invalid inputs");
            }
        }
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

        chunk_tags.resize(chunks.size());
        // now label each chunk
        for (unsigned long j = 0; j < chunks.size(); )
        {
            chunk_tags[j] = df(extract_ner_chunk_features(sentence, sent, chunks[j]));

            // if this chunk is predicted to not be an entity then erase it
            if (chunk_tags[j] == NOT_AN_ENTITY)
            {
                std::swap(chunk_tags[j], chunk_tags.back());
                std::swap(chunks[j], chunks.back());
                chunk_tags.resize(chunk_tags.size()-1);
                chunks.resize(chunks.size()-1);
            }
            else
            {
                ++j;
            }
        }
    }

// ----------------------------------------------------------------------------------------

}

