// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_MITIE_NaMED_ENTITY_EXTRACTOR_H_
#define MIT_LL_MITIE_NaMED_ENTITY_EXTRACTOR_H_

#include <mitie/total_word_feature_extractor.h>
#include <mitie/ner_feature_extraction.h>
#include <dlib/svm.h>
#include <dlib/vectorstream.h>
#include <dlib/hash.h>

namespace mitie
{
    class named_entity_extractor
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object is a simple tool for identifying the named entities in
                tokenized text.  In particular, it's just a wrapper around a
                dlib::sequence_segmenter and a multiclass classifier that predicts the type
                of each named entity.

            THREAD SAFETY
                Note that this object uses mutable internal scratch space.  Therefore, it is
                unsafe for two threads to touch the same instance of this object at a time
                without mutex locking it first.
        !*/
    public:

        named_entity_extractor():fingerprint(0){}
        /*!
            ensures
                - When used this object won't output any entities.   You need to either use
                  the other constructor or deserialize a saved named_entity_extractor to
                  get something that is useful.
        !*/

        named_entity_extractor(
            const std::vector<std::string>& tag_name_strings,
            const total_word_feature_extractor& fe,
            const dlib::sequence_segmenter<ner_feature_extractor>& segmenter,
            const dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<ner_sample_type>,unsigned long>& df
        ); 
        /*!
            requires
                - segmenter.get_feature_extractor().num_features() == fe.get_num_dimensions() 
                - df must be designed to work with fe (i.e. it must have been trained with
                  features from fe and extract_ner_chunk_features()).
                - df.number_of_classes() => tag_name_strings.size()
                  (i.e. the classifier needs to predict all the possible tags and also
                  optionally a "not entity" tag which it does by predicting a value >=
                  tag_name_strings.size())
                - for all i where 0 <= i < tag_name_strings.size():
                    - df.get_labels() contains an element equal to i.
                      (All we are saying there is that the labels need to be contiguous
                      integers and that the tag_name_strings vector and the decision function
                      need to agree on the set of labels)
            ensures
                - Just loads the given objects into *this.  
                - The interpretation of tag_name_strings is that it maps the output of df
                  into a meaningful text name for the NER tag.  
        !*/

        dlib::uint64 get_fingerprint(
        ) const { return fingerprint; }
        /*!
            ensures
                - returns a 64bit ID number that uniquely identifies this object instance.
                  The ID is computed based on the state of this object, so any copy of it
                  that has the same state will have the same fingerprint ID number.  This
                  is useful since down stream models that have been trained to use a
                  specific named_entity_extractor instance can record the fingerprint of
                  the named_entity_extractor they used and use that fingerprint ID to
                  verify that the same named_entity_extractor is being used later on.
        !*/

        void predict(
            const std::vector<std::string>& sentence,
            std::vector<std::pair<unsigned long, unsigned long> >& chunks,
            std::vector<unsigned long>& chunk_tags,
            std::vector<double>& chunk_scores
        ) const;
        /*!
            ensures
                - Runs the named entity recognizer on the sequence of tokenized words
                  inside sentence.  The detected named entities are stored into chunks.  
                - #chunks == the locations of the named entities. 
                - The identified named entities are listed inside chunks in the order in
                  which they appeared in the input sentence.  
                - #chunks.size() == #chunk_tags.size()
                - for all valid i:
                    - #chunk_tags[i] == the label for the entity at location #chunks[i].  Moreover, 
                      chunk tag ID numbers are contiguous and start at 0.  Therefore we have:
                        - 0 <= #chunk_tags[i] < get_tag_name_strings().size()
                    - #chuck_score[i] == the score for the entity at location #chunks[i]. The value
                      represents a confidence score, but does not represent a probability. Accordingly,
                      the value may range outside of the closed interval of 0 to 1. A larger value
                      represents a higher confidence. A value < 0 indicates that the label is likely
                      incorrect. That is, the canonical decision threshold is at 0.
                    - #chunks[i] == a half open range indicating where the entity is within
                      sentence.  In particular, the entity is composed of the tokens
                      sentence[#chunks[i].first] through sentence[#chunks[i].second-1].
                    - The textual label for the i-th entity is get_tag_name_strings()[#chunk_tags[i]].
        !*/

        void operator() (
            const std::vector<std::string>& sentence,
            std::vector<std::pair<unsigned long, unsigned long> >& chunks,
            std::vector<unsigned long>& chunk_tags
        ) const;
        /*!
            ensures
                - Runs the named entity recognizer on the sequence of tokenized words
                  inside sentence.  The detected named entities are stored into chunks.  
                - #chunks == the locations of the named entities. 
                - The identified named entities are listed inside chunks in the order in
                  which they appeared in the input sentence.  
                - #chunks.size() == #chunk_tags.size()
                - for all valid i:
                    - #chunk_tags[i] == the label for the entity at location #chunks[i].  Moreover, 
                      chunk tag ID numbers are contiguous and start at 0.  Therefore we have:
                        - 0 <= #chunk_tags[i] < get_tag_name_strings().size()
                    - #chunks[i] == a half open range indicating where the entity is within
                      sentence.  In particular, the entity is composed of the tokens
                      sentence[#chunks[i].first] through sentence[#chunks[i].second-1].
                    - The textual label for the i-th entity is get_tag_name_strings()[#chunk_tags[i]].
        !*/

        const std::vector<std::string>& get_tag_name_strings (
        ) const { return tag_name_strings; }
        /*!
            ensures
                - Returns a vector that maps entity numeric ID tags into their string labels.  
        !*/

        friend void serialize(const named_entity_extractor& item, std::ostream& out)
        {
            int version = 2;
            dlib::serialize(version, out);
            dlib::serialize(item.fingerprint, out);
            dlib::serialize(item.tag_name_strings, out);
            serialize(item.fe, out);
            serialize(item.segmenter, out);
            serialize(item.df, out);
        }

        friend void deserialize(named_entity_extractor& item, std::istream& in)
        {
            int version = 0;
            dlib::deserialize(version, in);
            if (version != 2)
                throw dlib::serialization_error("Unexpected version found while deserializing mitie::named_entity_extractor.");
            dlib::deserialize(item.fingerprint, in);
            dlib::deserialize(item.tag_name_strings, in);
            deserialize(item.fe, in);
            deserialize(item.segmenter, in);
            deserialize(item.df, in);
        }

        const total_word_feature_extractor& get_total_word_feature_extractor(
        ) const { return fe; }

    private:
        void compute_fingerprint()
        {
            std::vector<char> buf;
            dlib::vectorstream sout(buf);
            sout << "fingerprint";
            dlib::serialize(tag_name_strings, sout);
            serialize(fe.get_fingerprint(), sout);
            serialize(segmenter, sout);
            serialize(df, sout);

            fingerprint = dlib::murmur_hash3_128bit(&buf[0], buf.size()).first;
        }

        dlib::uint64 fingerprint;
        std::vector<std::string> tag_name_strings;
        total_word_feature_extractor fe;
        dlib::sequence_segmenter<ner_feature_extractor> segmenter;
        dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<ner_sample_type>,unsigned long> df;
    };
}

#endif // MIT_LL_MITIE_NaMED_ENTITY_EXTRACTOR_H_

