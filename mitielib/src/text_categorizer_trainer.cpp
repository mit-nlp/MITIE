// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include <mitie/text_categorizer_trainer.h>
#include <dlib/svm_threaded.h>

using namespace std;
using namespace dlib;

namespace mitie
{

// ----------------------------------------------------------------------------------------
    text_categorizer_trainer::
    text_categorizer_trainer ( ) : beta(0.5), num_threads(4)
    {

    }

// ----------------------------------------------------------------------------------------

    text_categorizer_trainer::
    text_categorizer_trainer (
        const std::string& filename
    ) : beta(0.5), num_threads(4)
    {
        string classname;
        dlib::deserialize(filename) >> classname >> tfe;
    }

// ----------------------------------------------------------------------------------------

    unsigned long text_categorizer_trainer::
    size() const 
    {
        return contents.size();
    }

// ----------------------------------------------------------------------------------------

    void text_categorizer_trainer::
    add (
            const std::vector<std::string>& text,
            const std::string& label
    ) 
    {
        contents.push_back(text);
        text_labels.push_back(get_label_id(label));
    }

// ----------------------------------------------------------------------------------------

    void text_categorizer_trainer::
    add (
            const std::vector<std::vector<std::string> >& texts,
            const std::vector<std::string>& labels
    ) 
    /*!
        requires
            - it must be legal to call add(texts[i], labels[i]) for all i.
        ensures
            - For all valid i: performs:  add(texts[i], labels[i]);
                (i.e. This function is just a convenience for adding a bunch of training
                data into a trainer in one call).
    !*/
    {
        DLIB_CASSERT(texts.size() == labels.size(),"");

        for (unsigned long i = 0; i < texts.size(); ++i)
            add(texts[i], labels[i]);
    }

// ----------------------------------------------------------------------------------------

    unsigned long text_categorizer_trainer::
    get_num_threads (
    ) const { return num_threads; }

// ----------------------------------------------------------------------------------------

    void text_categorizer_trainer::
    set_num_threads (
        unsigned long num
    ) { num_threads = num; }

// ----------------------------------------------------------------------------------------

    double text_categorizer_trainer::
    get_beta (
    ) const { return beta; }

// ----------------------------------------------------------------------------------------

    void text_categorizer_trainer::
    set_beta (
        double new_beta
    )
    {
        DLIB_CASSERT(new_beta >= 0, "Invalid beta");
        beta = new_beta;
    }

// ----------------------------------------------------------------------------------------

    text_categorizer text_categorizer_trainer::
    train (
    ) const
    /*!
        requires
            - size() > 0
    !*/
    {
        DLIB_CASSERT(size() > 0, "You can't train a text_categorizer if you don't give any training data.");

        // timestaper used for printouts
        dlib::timestamper ts;

        // Print out all the labels the user gave to the screen.
        std::vector<std::string> all_labels = get_all_labels();
        cout << "Training to recognize " << all_labels.size() << " categories: ";
        for (unsigned long i = 0; i < all_labels.size(); ++i)
        {
            cout << "'" << all_labels[i] << "'";
            if (i+1 < all_labels.size())
                cout << ", ";
        }
        cout << endl;

        cout << "Train classifier" << endl;

        dlib::uint64 start = ts.get_timestamp();
        classifier_type df = train_text_categorizer_classifier();
        dlib::uint64 stop = ts.get_timestamp();
        cout << "Training time: " << (stop - start)/1000/1000 << " seconds." << endl;
        cout << "df.number_of_classes(): "<< df.number_of_classes() << endl << endl;

        return text_categorizer(get_all_labels(), tfe, df);
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    class train_text_classifier_objective
    {
    public:
        train_text_classifier_objective (
            const std::vector<text_sample_type>& samples_,
            const std::vector<unsigned long>& labels_,
            unsigned long num_threads_,
            double beta_,
            unsigned long num_labels_,
            unsigned long max_iterations_
        ) : samples(samples_), labels(labels_), num_threads(num_threads_), beta(beta_), num_labels(num_labels_), max_iterations(max_iterations_)
        {}

        double operator() (
            const double C 
        ) const
        {
            svm_multiclass_linear_trainer<sparse_linear_kernel<text_sample_type>,unsigned long> trainer;

            trainer.set_c(C);
            trainer.set_num_threads(num_threads);
            trainer.set_max_iterations(max_iterations);
            //trainer.be_verbose();
            matrix<double> res = cross_validate_multiclass_trainer(trainer, samples, labels, 2);
            double score = compute_fscore(res, num_labels);
            cout << "C: " << C << "   f-score: "<< score << endl;
            return score;
        }

        double compute_fscore (
            const matrix<double>& res,
            const unsigned long num_labels 
        ) const
        {
            // Any output from the classifier that has a label value >= num_labels is
            // predicting that something is in the "not a relation" category.  For the
            // purposes of computing precision, we ignore this category.
            double correct_relation_predictions = sum(diag(subm(res,0,0, num_labels, num_labels)));
            double total_relation_predictions   = sum(     subm(res,0,0, res.nr(), num_labels));
            double total_truth_relations        = sum(     subm(res,0,0, num_labels, res.nc()));

            double precision = correct_relation_predictions/total_relation_predictions;
            double recall = correct_relation_predictions/total_truth_relations;

            return (1+beta*beta) * precision*recall/(beta*beta*precision + recall);
        }

    private:
        const std::vector<text_sample_type>& samples;
        const std::vector<unsigned long>& labels;
        const unsigned long num_threads;
        const double beta;
        const unsigned long num_labels;
        const unsigned long max_iterations;
    };

// ----------------------------------------------------------------------------------------

    unsigned long text_categorizer_trainer::
    count_of_least_common_label (
        const std::vector<unsigned long>& labels
    ) const
    {
        std::map<unsigned long, unsigned long> counts;
        for (unsigned long i = 0; i < labels.size(); ++i)
        {
            counts[labels[i]]++;
        }
        unsigned long min_count = 1000000000; //std::numeric_limits<unsigned long>::max();
        for (std::map<unsigned long,unsigned long>::iterator i = counts.begin(); i != counts.end(); ++i)
        {
            min_count = std::min(min_count, i->second);
        }
        return min_count;
    }

// ----------------------------------------------------------------------------------------

    text_categorizer_trainer::classifier_type text_categorizer_trainer::
    train_text_categorizer_classifier (
    ) const
    {
        cout << "extracting text features" << endl;
        // do the feature extraction for all the texts
        std::vector<text_sample_type> samples;
        std::vector<unsigned long> labels;
        samples.reserve(contents.size());
        labels.reserve(text_labels.size());
        for (unsigned long i = 0; i < contents.size(); ++i) {
            const std::vector<matrix<float,0,1> >& sent = sentence_to_feats(tfe, contents[i]);
            samples.push_back( extract_combined_features(contents[i], sent) );
            labels.push_back( text_labels[i] );
        }
        randomize_samples(samples, labels);

        cout << "now do training" << endl;
        cout << "num training samples: " << samples.size() << endl;

        svm_multiclass_linear_trainer<sparse_linear_kernel<text_sample_type>,unsigned long> trainer;

        trainer.set_c(300);
        trainer.set_num_threads(num_threads);
        trainer.set_epsilon(0.0001);
        trainer.set_max_iterations(2000);
        //trainer.be_verbose();

        if (count_of_least_common_label(labels) > 1)
        {
            train_text_classifier_objective obj(samples, labels, num_threads, beta, get_all_labels().size(), 2000);

            double C = 300;
            const double min_C = 0.01;
            const double max_C = 5000;
            const double eps = 1;
            try
            {
                find_max_single_variable(obj, C, min_C, max_C, eps, 100, 100);
            }
            catch (optimize_single_variable_failure&)
            {
                // if the optimization ran too long then just use a C of 300
                C = 300;
            }

            cout << "best C: "<< C << endl;
            trainer.set_c(C);
        }

        classifier_type df = trainer.train(samples, labels);
        matrix<double> res = test_multiclass_decision_function(df, samples, labels);
        cout << "test on train: \n" << res << endl;
        cout << "overall accuracy: "<< sum(diag(res))/sum(res) << endl;

        return df;
    }

// ----------------------------------------------------------------------------------------

    unsigned long text_categorizer_trainer::
    get_label_id (
        const std::string& str
    )
    {
        std::map<std::string,unsigned long>::iterator i;
        i = label_to_id.find(str);
        if (i != label_to_id.end())
        {
            return i->second;
        }
        else
        {
            const unsigned long next_id = label_to_id.size();
            label_to_id[str] = next_id;
            return next_id;
        }
    }

// ----------------------------------------------------------------------------------------

    std::vector<std::string> text_categorizer_trainer::
    get_all_labels(
    ) const
    {
        std::vector<std::string> temp(label_to_id.size());
        for (std::map<std::string,unsigned long>::const_iterator i = label_to_id.begin();
            i != label_to_id.end(); ++i)
        {
            temp[i->second] = i->first;
        }
        return temp;
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
}



