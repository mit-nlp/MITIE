// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include <mitie/binary_relation_detector_trainer.h>
#include <dlib/svm_threaded.h>
#include <dlib/optimization.h>

using namespace dlib;
using namespace std;

namespace mitie
{

// ----------------------------------------------------------------------------------------

    binary_relation_detector_trainer::
    binary_relation_detector_trainer (
        const std::string& relation_name_,
        const named_entity_extractor& ner
    ) : 
        tfe(ner.get_total_word_feature_extractor()), 
        beta(0.1), 
        num_threads(4),
        relation_name(relation_name_)
    {
    }

// ----------------------------------------------------------------------------------------

    std::string binary_relation_detector_trainer::
    get_relation_name(
    ) const
    {
        return relation_name;
    }

// ----------------------------------------------------------------------------------------

    unsigned long binary_relation_detector_trainer::
    num_positive_examples(
    ) const
    {
        return pos_sentences.size();
    }

// ----------------------------------------------------------------------------------------

    unsigned long binary_relation_detector_trainer::
    num_negative_examples(
    ) const
    {
        return neg_sentences.size();
    }

// ----------------------------------------------------------------------------------------

    void binary_relation_detector_trainer::
    add_positive_binary_relation (
        const std::vector<std::string>& tokens,
        unsigned long arg1_start,
        unsigned long arg1_length,
        unsigned long arg2_start,
        unsigned long arg2_length
    )
    {
        DLIB_CASSERT(arg1_length > 0 && arg2_length > 0, "Invalid Inputs");
        DLIB_CASSERT(mitie_entities_overlap(arg1_start,arg1_length,arg2_start,arg2_length) == 0, 
            "Binary relation arguments can't overlap.");
        DLIB_CASSERT(arg1_start+arg1_length <= tokens.size() && arg2_start+arg2_length <= tokens.size(), 
            "Invalid Inputs");

        pos_sentences.push_back(tokens);
        pos_arg1s.push_back(std::make_pair(arg1_start, arg1_start+arg1_length));
        pos_arg2s.push_back(std::make_pair(arg2_start, arg2_start+arg2_length));
    }

// ----------------------------------------------------------------------------------------

    void binary_relation_detector_trainer::
    add_negative_binary_relation (
        const std::vector<std::string>& tokens,
        unsigned long arg1_start,
        unsigned long arg1_length,
        unsigned long arg2_start,
        unsigned long arg2_length
    )
    {
        DLIB_CASSERT(arg1_length > 0 && arg2_length > 0, "Invalid Inputs");
        DLIB_CASSERT(mitie_entities_overlap(arg1_start,arg1_length,arg2_start,arg2_length) == 0, 
            "Binary relation arguments can't overlap.");
        DLIB_CASSERT(arg1_start+arg1_length <= tokens.size() && arg2_start+arg2_length <= tokens.size(), 
            "Invalid Inputs");

        neg_sentences.push_back(tokens);
        neg_arg1s.push_back(std::make_pair(arg1_start, arg1_start+arg1_length));
        neg_arg2s.push_back(std::make_pair(arg2_start, arg2_start+arg2_length));
    }

// ----------------------------------------------------------------------------------------

    unsigned long binary_relation_detector_trainer::
    get_num_threads (
    ) const
    {
        return num_threads;
    }

// ----------------------------------------------------------------------------------------

    void binary_relation_detector_trainer::
    set_num_threads (
        unsigned long num
    )
    {
        num_threads = num;
    }

// ----------------------------------------------------------------------------------------

    double binary_relation_detector_trainer::
    get_beta (
    ) const
    {
        return beta;
    }

// ----------------------------------------------------------------------------------------

    void binary_relation_detector_trainer::
    set_beta (
        double new_beta
    )
    {
        DLIB_CASSERT(new_beta >= 0, "Invalid beta");
        beta = new_beta;
    }

// ----------------------------------------------------------------------------------------

    class brdt_cv_objective
    {
    public:
        brdt_cv_objective(
            const unsigned long num_threads_,
            const int cv_folds_,
            const double beta_,
            const std::vector<sparse_vector_type>& samples_,
            const std::vector<double>& labels_
        ) : num_threads(num_threads_), cv_folds(cv_folds_), beta(beta_), samples(samples_), labels(labels_) {}

        double operator()(matrix<double,2,1> params) const
        {
            params = exp(params);
            svm_c_linear_dcd_trainer<sparse_linear_kernel<sparse_vector_type> > trainer;
            trainer.set_c_class1(params(0));
            trainer.set_c_class2(params(1));
            cout << "testing with params: " << trans(params);
            matrix<double> res = cross_validate_trainer_threaded(trainer, samples, labels, cv_folds, num_threads);
            cout << "cv: "<< res;

            const double fscore = (1+beta*beta) * res(0)*res(1) / (beta*beta*res(1) + res(0));

            cout << "fscore: "<< fscore << endl << endl;
            return fscore;
        }

    private:
        const unsigned long num_threads;
        const int cv_folds;
        const double beta;
        const std::vector<sparse_vector_type>& samples;
        const std::vector<double>& labels;
    };

    binary_relation_detector binary_relation_detector_trainer::
    train (
    ) const
    {
        DLIB_CASSERT(num_positive_examples() > 0, "Not enough training data given.");
        DLIB_CASSERT(num_negative_examples() > 0, "Not enough training data given.");

        std::vector<sparse_vector_type> samples;
        std::vector<double> labels;

        for (unsigned long i = 0; i < pos_sentences.size(); ++i)
        {
            samples.push_back(extract_binary_relation(pos_sentences[i], pos_arg1s[i], pos_arg2s[i], tfe).feats);
            labels.push_back(+1);
        }
        for (unsigned long i = 0; i < neg_sentences.size(); ++i)
        {
            samples.push_back(extract_binary_relation(neg_sentences[i], neg_arg1s[i], neg_arg2s[i], tfe).feats);
            labels.push_back(-1);
        }

        randomize_samples(samples, labels);

        const int cv_folds = 6;
        brdt_cv_objective obj(num_threads, cv_folds, beta, samples, labels);

        matrix<double,2,1> params;
        params = 5000.0/samples.size(), 5000.0/samples.size();
        // We do the parameter search in log space.
        params = log(params);
        // can't do the parameter search if we don't have enough data.   So if we don't
        // have much data then just use the default parameters.
        if (pos_sentences.size() > (unsigned)cv_folds)
        {
            matrix<double,2,1> lower_params, upper_params;
            lower_params = 1.0/samples.size(), 1.0/samples.size();
            upper_params = 100000.0/samples.size(), 100000.0/samples.size();
            lower_params = log(lower_params);
            upper_params = log(upper_params);
            const double rho_begin = min(upper_params-lower_params)*0.15;
            const double rho_end = log(1.2/samples.size()) - log(1.0/samples.size());
            find_max_bobyqa(obj, params, params.size()*2+1, lower_params, upper_params, rho_begin, rho_end, 200);
        }


        // Note that we rescale the parameters to account for the fact that the cross
        // validation was done on a dataset slightly smaller than the one we ultimately train
        // on and the C parameters of this trainer are not normalized by the number of training
        // samples.
        params = exp(params) * (cv_folds-1.0)/cv_folds;
        svm_c_linear_dcd_trainer<sparse_linear_kernel<sparse_vector_type> > trainer;
        trainer.set_c_class1(params(0));
        trainer.set_c_class2(params(1));
        cout << "using parameters of: " << trans(params);
        cout << "now doing training..." << endl;
        binary_relation_detector bd;
        bd.df = trainer.train(samples, labels);
        bd.relation_type = relation_name;
        bd.total_word_feature_extractor_fingerprint = tfe.get_fingerprint();

        cout << "test on train: " << test_binary_decision_function(bd.df, samples, labels) << endl;
        return bd;
    }

// ----------------------------------------------------------------------------------------

}

