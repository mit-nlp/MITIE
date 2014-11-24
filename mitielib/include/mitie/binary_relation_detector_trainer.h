// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_BINARY_rELATION_DETECTION_TRAINER_H_
#define MIT_LL_BINARY_rELATION_DETECTION_TRAINER_H_

#include <mitie/binary_relation_detector.h>
#include <mitie/named_entity_extractor.h>
#include <mitie.h>

namespace mitie
{
    class binary_relation_detector_trainer 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool for training mitie::binary_relation_detector objects from a
                set of annotated training data.
        !*/
    public:
        binary_relation_detector_trainer (
            const std::string& relation_name,
            const named_entity_extractor& ner
        );
        /*!
            ensures
                - #get_beta() == 0.1
                - #num_threads() == 4
                - A copy of the given ner object is used during the training process.
                  Therefore, ner should be trained for the same language as the data we are
                  going to use to train a binary relation detector. (e.g. Don't use a ner
                  object for Spanish if you are trying to make an English language relation
                  detector).
                - #get_relation_name() == relation_name
                - #num_positive_examples() == 0
                - #num_negative_examples() == 0
        !*/

        std::string get_relation_name(
        ) const;
        /*!
            ensures
                - returns the name we will give to any binary_relation_detector objects
                  created by this object.  That is, this string is used to populate the
                  binary_relation_detector::relation_type field.
        !*/

        unsigned long num_positive_examples(
        ) const;
        /*!
            ensures
                - returns the number of positive training instances that have been added
                  into this object.
        !*/

        unsigned long num_negative_examples(
        ) const;
        /*!
            ensures
                - returns the number of negative training instances that have been added
                  into this object.
        !*/

        void add_positive_binary_relation (
            const std::vector<std::string>& tokens,
            unsigned long arg1_start,
            unsigned long arg1_length,
            unsigned long arg2_start,
            unsigned long arg2_length
        );
        /*!
            requires
                - arg1_length > 0
                - arg2_length > 0
                - The arg indices reference valid elements of the tokens array.  That is,
                  the following expressions should be valid:
                    - tokens[arg1_start]
                    - tokens[arg1_start+arg1_length-1]
                    - tokens[arg2_start]
                    - tokens[arg2_start+arg1_length-1]
                - mitie_entities_overlap(arg1_start,arg1_length,arg2_start,arg2_length) == 0
            ensures
                - This function adds a positive training instance into the trainer.  That
                  is, this function tells the trainer that the given tokens contain an
                  example of the binary relation we are trying to learn.  Moreover, the
                  first argument of the relation is located at tokens[arg1_start] and is
                  arg1_length tokens long.  Similarly, the second argument of the relation
                  is located at tokens[arg2_start] and is arg2_length tokens long.
                - #num_positive_examples() == num_positive_examples() + 1
        !*/

        void add_positive_binary_relation (
            const std::vector<std::string>& tokens,
            const std::pair<unsigned long, unsigned long>& arg1,
            const std::pair<unsigned long, unsigned long>& arg2
        ) { add_positive_binary_relation(tokens, arg1.first, arg1.second-arg1.first, arg2.first, arg2.second-arg2.first); }

        void add_negative_binary_relation (
            const std::vector<std::string>& tokens,
            unsigned long arg1_start,
            unsigned long arg1_length,
            unsigned long arg2_start,
            unsigned long arg2_length
        );
        /*!
            requires
                - arg1_length > 0
                - arg2_length > 0
                - The arg indices reference valid elements of the tokens array.  That is,
                  the following expressions should be valid:
                    - tokens[arg1_start]
                    - tokens[arg1_start+arg1_length-1]
                    - tokens[arg2_start]
                    - tokens[arg2_start+arg1_length-1]
                - mitie_entities_overlap(arg1_start,arg1_length,arg2_start,arg2_length) == 0
            ensures
                - This function adds a negative training instance into the trainer.  That
                  is, this function tells the trainer that the given tokens and argument
                  combination is not a binary relation we are trying to learn.  The argument 
                  indices have the same interpretation as they do for add_positive_binary_relation(). 
                - #num_negative_examples() == num_negative_examples() + 1
        !*/

        void add_negative_binary_relation (
            const std::vector<std::string>& tokens,
            const std::pair<unsigned long, unsigned long>& arg1,
            const std::pair<unsigned long, unsigned long>& arg2
        ) { add_negative_binary_relation(tokens, arg1.first, arg1.second-arg1.first, arg2.first, arg2.second-arg2.first); }

        unsigned long get_num_threads (
        ) const;
        /*!
            ensures
                - returns the number of threads that will be used to perform training.  You 
                  should set this equal to the number of processing cores you have on your 
                  computer.  
        !*/

        void set_num_threads (
            unsigned long num
        );
        /*!
            ensures
                - #get_num_threads() == num
        !*/

        double get_beta (
        ) const;
        /*!
            ensures
                - returns the trainer's beta parameter.  This parameter controls the
                  trade-off between trying to avoid false alarms but also detecting
                  everything.  Different values of beta have the following interpretations:
                    - beta < 1 indicates that you care more about avoiding false alarms
                      than missing detections.  The smaller you make beta the more the
                      trainer will try to avoid false alarms.
                    - beta == 1 indicates that you don't have a preference between avoiding
                      false alarms or not missing detections.  That is, you care about
                      these two things equally.
                    - beta > 1 indicates that care more about not missing detections than
                      avoiding false alarms.
        !*/

        void set_beta (
            double new_beta
        );
        /*!
            requires
                - new_beta >= 0
            ensures
                - #get_beta() == new_beta
        !*/

        binary_relation_detector train (
        ) const;
        /*!
            requires
                - num_positive_examples() > 0
                - num_negative_examples() > 0
            ensures
                - Trains a binary_relation_detector based on the training instances given
                  to this object via add_positive_binary_relation() and
                  add_negative_binary_relation() calls and returns the result.
                - The relation_type field of the returned object will be set to
                  get_relation_name().
        !*/

    private:

        /*!
            CONVENTION
                - pos_sentences.size() == pos_arg1s.size() == pos_arg2s.size()
                - neg_sentences.size() == neg_arg1s.size() == neg_arg2s.size()
        !*/
        total_word_feature_extractor tfe;
        double beta;
        unsigned long num_threads;
        std::string relation_name;
        typedef std::pair<unsigned long, unsigned long> range;
        std::vector<std::vector<std::string> > pos_sentences;
        std::vector<range> pos_arg1s, pos_arg2s;
        std::vector<std::vector<std::string> > neg_sentences;
        std::vector<range> neg_arg1s, neg_arg2s;
    };
}

#endif // MIT_LL_BINARY_rELATION_DETECTION_TRAINER_H_

