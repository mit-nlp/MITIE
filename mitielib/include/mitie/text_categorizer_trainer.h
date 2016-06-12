#ifndef MITIE_TEXT_CATEGORIZER_TRAINER_H
#define MITIE_TEXT_CATEGORIZER_TRAINER_H

#include <vector>
#include <string>
#include <utility>
#include <mitie/total_word_feature_extractor.h>
#include <mitie/text_categorizer.h>
#include <mitie/text_feature_extraction.h>
#include <dlib/svm.h>
#include <map>

namespace mitie
{
    class text_categorizer_trainer
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool for training mitie::text_categorizer objects from a
                set of annotated training data.
        !*/

    public:
        explicit text_categorizer_trainer (
        );
        /*!
            ensures
                - #get_beta() == 0.5
                - #num_threads() == 4
                - This function attempts to initialize a trainer for BoW based text categorizer.
        !*/

    public:
        explicit text_categorizer_trainer (
                const std::string& filename
        );
        /*!
            ensures
                - #get_beta() == 0.5
                - #num_threads() == 4
                - This function attempts to load a mitie::total_word_feature_extractor from the
                  file with the given filename.  This feature extractor can be used during the
                  training process to improve the accuracy of text categorizer.
        !*/

        unsigned long size(
        ) const;
        /*!
            ensures
                - returns the number of training instances that have been added into this object.
        !*/

        void add (
                const std::vector<std::string>& text,
                const std::string& label
        );
        /*!
            requires
                - texts.size() == labels.size()
            ensures
                - This function adds one training instances into the trainer. That
                  is, this function tells the trainer that the each text vector belongs to
                  a certain kind of labelled category we are trying to learn.
        !*/

        void add (
                const std::vector<std::vector<std::string> >& texts,
                const std::vector<std::string>& labels
        );
        /*!
            requires
                - texts.size() == labels.size()
            ensures
                - This function adds training instances into the trainer in batch.
        !*/

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

        text_categorizer train (
        ) const;
        /*!
            requires
                - size() > 0
            ensures
                - Trains a text_categorizer based on the training instances given to
                  this object via add() calls and returns the result.
        !*/

    private:

        unsigned long count_of_least_common_label (
                const std::vector<unsigned long>& labels
        ) const;

        typedef dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<text_sample_type>,unsigned long> classifier_type;
        classifier_type train_text_categorizer_classifier (
        ) const;

        unsigned long get_label_id (
                const std::string& str
        );

        std::vector<std::string> get_all_labels(
        ) const;

        total_word_feature_extractor tfe;
        double beta;
        unsigned long num_threads;
        std::map<std::string,unsigned long> label_to_id;
        std::vector<std::vector<std::string> > contents;
        std::vector<unsigned long> text_labels;
    };

// ----------------------------------------------------------------------------------------

}

#endif //MITIE_TEXT_CATEGORIZER_TRAINER_H
