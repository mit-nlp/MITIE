// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include <mitie/ner_trainer.h>
#include <dlib/svm_threaded.h>
#include <dlib/optimization.h>
#include <dlib/misc_api.h>

using namespace std;
using namespace dlib;

namespace mitie
{

// ----------------------------------------------------------------------------------------

    namespace
    {
        static bool entities_overlap (
            const std::pair<unsigned long, unsigned long>& arg1,
            const std::pair<unsigned long, unsigned long>& arg2
        ) 
        {
            // find intersection range
            const unsigned long left = std::max(arg1.first, arg2.first);
            const unsigned long right = std::min(arg1.second, arg2.second);
            // if the range is not empty
            if (left < right)
                return true;
            else
                return false;
        }
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    ner_training_instance::
    ner_training_instance (
        const std::vector<std::string>& tokens_
    ) : tokens(tokens_)
    {
    }

// ----------------------------------------------------------------------------------------

    unsigned long ner_training_instance::
    num_tokens(
    ) const 
    { 
        return tokens.size(); 
    }

// ----------------------------------------------------------------------------------------

    unsigned long ner_training_instance::
    num_entities(
    ) const 
    { 
        return chunks.size(); 
    }

// ----------------------------------------------------------------------------------------

    bool ner_training_instance::
    overlaps_any_entity (
        unsigned long start,
        unsigned long length
    ) const
    {
        const std::pair<unsigned long, unsigned long> p(start, start+length);
        for (unsigned long i = 0; i < chunks.size(); ++i)
        {
            if (entities_overlap(p, chunks[i]))
                return true;
        }
        return false;
    }

// ----------------------------------------------------------------------------------------

    void ner_training_instance::
    add_entity (
        const std::pair<unsigned long,unsigned long>& range,
        const std::string& label
    )
    {
        // TODO, make just DLIB_ASSERT 
        DLIB_CASSERT(overlaps_any_entity(range.first, range.second-range.first) == false, "Invalid Inputs");
        DLIB_CASSERT(range.first < range.second && range.second <= num_tokens(), "Invalid Inputs");

        chunks.push_back(range);
        chunk_labels.push_back(label);
    }

// ----------------------------------------------------------------------------------------

    void ner_training_instance::
    add_entity (
        unsigned long start,
        unsigned long length,
        const char* label
    )
    {
        // TODO, make just DLIB_ASSERT 
        DLIB_CASSERT(overlaps_any_entity(start, length) == false, "Invalid Inputs");
        DLIB_CASSERT(length > 0 && start+length <= num_tokens(), "Invalid Inputs");
        chunks.push_back(std::make_pair(start, start+length));
        chunk_labels.push_back(label);
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    ner_trainer::
    ner_trainer (
        const std::string& filename
    ) : beta(0.5), num_threads(4)
    {
        string classname;
        dlib::deserialize(filename) >> classname >> tfe;
    }

// ----------------------------------------------------------------------------------------

    unsigned long ner_trainer::
    size() const 
    {
        return sentences.size();
    }

// ----------------------------------------------------------------------------------------

    void ner_trainer::
    add (
        const ner_training_instance& item
    )
    {
        sentences.push_back(item.tokens);
        chunks.push_back(item.chunks);
        std::vector<unsigned long> temp;
        for (unsigned long i = 0; i < item.chunk_labels.size(); ++i)
            temp.push_back(get_label_id(item.chunk_labels[i]));
        chunk_labels.push_back(temp);
    }

// ----------------------------------------------------------------------------------------

    void ner_trainer::
    add (
        const std::vector<std::string>& tokens,
        const std::vector<std::pair<unsigned long,unsigned long> >& ranges,
        const std::vector<std::string>& labels
    ) 
    {
        // TODO, add missing asserts
        DLIB_CASSERT(ranges.size() == labels.size(),"");

        sentences.push_back(tokens);
        chunks.push_back(ranges);
        std::vector<unsigned long> temp;
        for (unsigned long i = 0; i < labels.size(); ++i)
            temp.push_back(get_label_id(labels[i]));
        chunk_labels.push_back(temp);
    }

// ----------------------------------------------------------------------------------------

    void ner_trainer::
    add (
        const std::vector<std::vector<std::string> >& tokens,
        const std::vector<std::vector<std::pair<unsigned long,unsigned long> > >& ranges,
        const std::vector<std::vector<std::string> >& labels
    ) 
    /*!
        requires
            - it must be legal to call add(tokens[i], ranges[i], labels[i]) for all i.
        ensures
            - For all valid i: performs:  add(tokens[i], ranges[i], labels[i]);
                (i.e. This function is just a convenience for adding a bunch of training
                data into a trainer in one call).
    !*/
    {
        for (unsigned long i = 0; i < tokens.size(); ++i)
            add(tokens[i], ranges[i], labels[i]);
    }

// ----------------------------------------------------------------------------------------

    unsigned long ner_trainer::
    get_num_threads (
    ) const { return num_threads; }

// ----------------------------------------------------------------------------------------

    void ner_trainer::
    set_num_threads (
        unsigned long num
    ) { num_threads = num; }

// ----------------------------------------------------------------------------------------

    double ner_trainer::
    get_beta (
    ) const { return beta; }

// ----------------------------------------------------------------------------------------

    void ner_trainer::
    set_beta (
        double new_beta
    )
    {
        DLIB_CASSERT(new_beta >= 0, "Invalid beta");
        beta = new_beta;
    }

// ----------------------------------------------------------------------------------------

    named_entity_extractor ner_trainer::
    train (
    ) const
    /*!
        requires
            - size() > 0
    !*/
    {
        DLIB_CASSERT(size() > 0, "You can't train a named_entity_extractor if you don't give any training data.");

	// timestaper used for printouts
	dlib::timestamper ts;

        // Print out all the labels the user gave to the screen.
        std::vector<std::string> all_labels = get_all_labels();
        cout << "Training to recognize " << all_labels.size() << " labels: ";
        for (unsigned long i = 0; i < all_labels.size(); ++i)
        {
            cout << "'" << all_labels[i] << "'";
            if (i+1 < all_labels.size())
                cout << ", ";
        }
        cout << endl;

        cout << "Part I: train segmenter" << endl;

	dlib::uint64 start = ts.get_timestamp();

        sequence_segmenter<ner_feature_extractor> segmenter;
        train_segmenter(segmenter);

	dlib::uint64 stop = ts.get_timestamp();

	cout << "Part I: elapsed time: " << (stop - start)/1000/1000 << " seconds." << endl << endl;

        std::vector<ner_sample_type> samples;
        std::vector<unsigned long> labels;
        extract_ner_segment_feats(segmenter, samples, labels);

        cout << "Part II: train segment classifier" << endl;

	start = ts.get_timestamp();

        classifier_type df = train_ner_segment_classifier(samples, labels);

	stop = ts.get_timestamp();

	cout << "Part II: elapsed time: " << (stop - start)/1000/1000 << " seconds." << endl;

        cout << "df.number_of_classes(): "<< df.number_of_classes() << endl;

        return named_entity_extractor(get_all_labels(), tfe, segmenter, df);
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    class train_ner_segment_classifier_objective
    {
    public:
        train_ner_segment_classifier_objective (
            const std::vector<ner_sample_type>& samples_,
            const std::vector<unsigned long>& labels_,
            unsigned long num_threads_,
            double beta_,
            unsigned long num_labels_,
            unsigned long max_iterations_
        ) : samples(samples_), labels(labels_), num_threads(num_threads_), beta(beta_), num_labels(num_labels_),
            max_iterations(max_iterations_)
        {}

        double operator() (
            const double C 
        ) const
        {
            svm_multiclass_linear_trainer<sparse_linear_kernel<ner_sample_type>,unsigned long> trainer;

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
        const std::vector<ner_sample_type>& samples;
        const std::vector<unsigned long>& labels;
        const unsigned long num_threads;
        const double beta;
        const unsigned long num_labels;
        const unsigned long max_iterations;
    };

// ----------------------------------------------------------------------------------------

    unsigned long ner_trainer::
    count_of_least_common_label (
        const std::vector<unsigned long>& labels
    ) const
    {
        std::map<unsigned long, unsigned long> counts;
        for (unsigned long i = 0; i < labels.size(); ++i)
        {
            counts[labels[i]]++;
        }
        unsigned long min_count = std::numeric_limits<unsigned long>::max();
        for (std::map<unsigned long,unsigned long>::iterator i = counts.begin(); i != counts.end(); ++i)
        {
            min_count = std::min(min_count, i->second);
        }
        return min_count;
    }

// ----------------------------------------------------------------------------------------

    ner_trainer::classifier_type ner_trainer::
    train_ner_segment_classifier (
        const std::vector<ner_sample_type>& samples,
        const std::vector<unsigned long>& labels
    ) const
    {
        cout << "now do training" << endl;
        cout << "num training samples: " << samples.size() << endl;

        svm_multiclass_linear_trainer<sparse_linear_kernel<ner_sample_type>,unsigned long> trainer;

        trainer.set_c(300);
        trainer.set_num_threads(num_threads);
        trainer.set_epsilon(0.0001);
        trainer.set_max_iterations(2000);
        //trainer.be_verbose();

        if (count_of_least_common_label(labels) > 1)
        {
            train_ner_segment_classifier_objective obj(samples, labels, num_threads, beta, get_all_labels().size(), 2000);

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

    static unsigned long get_label (
        const std::vector<std::pair<unsigned long, unsigned long> >& chunks,
        const std::vector<unsigned long>& chunk_labels,
        const std::pair<unsigned long, unsigned long>& range,
        const unsigned long not_entity
    ) 
    {
        for (unsigned long i = 0; i < chunks.size(); ++i)
        {
            if (range == chunks[i])
                return chunk_labels[i];
        }
        return not_entity;
    }

// ----------------------------------------------------------------------------------------

    void ner_trainer::
    extract_ner_segment_feats (
        const sequence_segmenter<ner_feature_extractor>& segmenter,
        std::vector<ner_sample_type>& samples,
        std::vector<unsigned long>& labels
    ) const
    {
        samples.clear();
        labels.clear();
        const std::vector<std::string> ner_labels = get_all_labels();

        for (unsigned long i = 0; i < sentences.size(); ++i)
        {
            const std::vector<matrix<float,0,1> >& sent = sentence_to_feats(tfe, sentences[i]);
            std::set<std::pair<unsigned long, unsigned long> > ranges;
            // put all the true chunks into ranges
            ranges.insert(chunks[i].begin(), chunks[i].end());

            // now get all the chunks our segmenter finds
            std::vector<std::pair<unsigned long, unsigned long> > temp;
            temp = segmenter(sent);
            ranges.insert(temp.begin(), temp.end());

            // now go over all the chunks we found and label them with their appropriate NER
            // types and also do feature extraction for each.
            std::set<std::pair<unsigned long,unsigned long> >::const_iterator j;
            for (j = ranges.begin(); j != ranges.end(); ++j)
            {
                samples.push_back(extract_ner_chunk_features(sentences[i], sent, *j));
                labels.push_back(get_label(chunks[i], chunk_labels[i], *j, ner_labels.size()));
            }
        }

        randomize_samples(samples, labels);
    }

// ----------------------------------------------------------------------------------------

    const static double LOSS_SCALE = 10;
    class train_segmenter_bobyqa_objective
    {
    public:
        train_segmenter_bobyqa_objective (
            structural_sequence_segmentation_trainer<ner_feature_extractor>& trainer_,
            const std::vector<std::vector<matrix<float,0,1> > >& samples_,
            const std::vector<std::vector<std::pair<unsigned long, unsigned long> > >& local_chunks_
        ) : trainer(trainer_), samples(samples_), local_chunks(local_chunks_)
        {}

        double operator() (
            const matrix<double,2,1>& params
        ) const
        {
            const double C = params(0);
            const double loss = params(1)/LOSS_SCALE;

            trainer.set_c(C);
            trainer.set_loss_per_missed_segment(loss);
            matrix<double> res = cross_validate_sequence_segmenter(trainer, samples, local_chunks, 2);
            double score = res(1); // use the recall as the measure of goodness
            cout << "C: "<< C << "   loss: " << loss << " \t" << score << endl;
            return score;
        }

    private:
        structural_sequence_segmentation_trainer<ner_feature_extractor>& trainer;
        const std::vector<std::vector<matrix<float,0,1> > >& samples;
        const std::vector<std::vector<std::pair<unsigned long, unsigned long> > >& local_chunks;
    };

// ----------------------------------------------------------------------------------------

    void ner_trainer::
    train_segmenter (
        sequence_segmenter<ner_feature_extractor>& segmenter
    ) const
    {
        cout << "words in dictionary: " << tfe.get_num_words_in_dictionary() << endl;
        cout << "num features: " << tfe.get_num_dimensions() << endl;


        // do the feature extraction for all the sentences
        std::vector<std::vector<matrix<float,0,1> > > samples;
        samples.reserve(sentences.size());
        for (unsigned long i = 0; i < sentences.size(); ++i)
            samples.push_back(sentence_to_feats(tfe, sentences[i]));

        std::vector<std::vector<std::pair<unsigned long, unsigned long> > > local_chunks(chunks);
        randomize_samples(samples, local_chunks);

        cout << "now do training" << endl;

        ner_feature_extractor nfe(tfe.get_num_dimensions());
        structural_sequence_segmentation_trainer<ner_feature_extractor> trainer(nfe);

        const double C = 20.0; 
        const double eps = 0.01;
        const unsigned long max_iterations = 2000;
        const double loss_per_missed_segment = 3.0; 
        const unsigned long cache_size = 5; 
        cout << "C:           "<< C << endl;
        cout << "epsilon:     "<< eps << endl;
        cout << "num threads: "<< num_threads << endl;
        cout << "cache size:  "<< cache_size << endl;
        cout << "max iterations: " << max_iterations << endl;
        cout << "loss per missed segment:  "<< loss_per_missed_segment << endl;
        trainer.set_c(C);
        trainer.set_epsilon(eps);
        trainer.set_max_iterations(max_iterations);
        trainer.set_num_threads(num_threads);
        trainer.set_max_cache_size(cache_size);
        trainer.set_loss_per_missed_segment(loss_per_missed_segment);
        //trainer.be_verbose();

        if (samples.size() > 1)
        {
            matrix<double,2,1> params;
            params = C, loss_per_missed_segment*LOSS_SCALE;

            matrix<double,2,1> min_params, max_params;
            min_params = 0.1, 1*LOSS_SCALE;
            max_params = 100, 10*LOSS_SCALE;

            train_segmenter_bobyqa_objective obj(trainer, samples, local_chunks);
            try
            {
                find_max_bobyqa(obj, params, params.size()*2+1, min_params, max_params, 15, 1, 100);
            }
            catch (bobyqa_failure&)
            {
                // if the optimization ran too long then just use the default
                // parameters
                params = C, loss_per_missed_segment*LOSS_SCALE;
            }

            cout << "best C: "<< params(0) << endl;
            cout << "best loss: "<< params(1)/LOSS_SCALE << endl;
            trainer.set_c(params(0));
            trainer.set_loss_per_missed_segment(params(1)/LOSS_SCALE);
        }


        segmenter = trainer.train(samples, local_chunks);

        cout << "num feats in chunker model: "<< segmenter.get_weights().size() << endl;
        cout << "train: precision, recall, f1-score: "<< test_sequence_segmenter(segmenter, samples, local_chunks);
    }

// ----------------------------------------------------------------------------------------

    unsigned long ner_trainer::
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

    std::vector<std::string> ner_trainer::
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
// ----------------------------------------------------------------------------------------

    ner_eval_metrics evaluate_named_entity_recognizer (
        const named_entity_extractor& ner,
        const std::vector<std::vector<std::string> >& sentences,
        const std::vector<std::vector<std::pair<unsigned long, unsigned long> > >& chunks,
        const std::vector<std::vector<std::string> >& text_chunk_labels
    )
    {
        // Make sure the requires clause is not broken.
        DLIB_CASSERT(sentences.size() == chunks.size() && chunks.size() == text_chunk_labels.size(), "Invalid inputs");
        for (unsigned long i = 0; i < chunks.size(); ++i)
        {
            DLIB_CASSERT(chunks[i].size() == text_chunk_labels[i].size(), "Invalid Inputs");
        }

        const std::vector<std::string> tags = ner.get_tag_name_strings();

        // convert text_chunk_labels into integer labels.
        std::vector<std::vector<unsigned long> > chunk_labels(text_chunk_labels.size());
        std::map<std::string,unsigned long> str_to_id;
        for (unsigned long i = 0; i < tags.size(); ++i)
            str_to_id[tags[i]]=i;
        for (unsigned long i = 0; i < chunk_labels.size(); ++i)
        {
            chunk_labels[i].resize(text_chunk_labels[i].size());
            for (unsigned long j = 0; j < chunk_labels[i].size(); ++j)
            {
                if (str_to_id.count(text_chunk_labels[i][j]) == 0)
                    throw dlib::error("NER object does not support the tag "+text_chunk_labels[i][j]+" found in testing dataset.");
                chunk_labels[i][j] = str_to_id[text_chunk_labels[i][j]];
            }
        }

        const unsigned long num_labels = tags.size();
        std::vector<double> num_targets(num_labels);
        std::vector<double> num_dets(num_labels);
        std::vector<double> num_true_dets(num_labels);

        // Now run the ner object over all the sentences and compare it to the truth data.
        for (unsigned long i = 0; i < sentences.size(); ++i)
        {
            std::vector<std::pair<unsigned long, unsigned long> > ranges;
            std::vector<unsigned long> predicted_labels;
            ner(sentences[i], ranges, predicted_labels);

            for (unsigned long j = 0; j < ranges.size(); ++j)
            {
                const unsigned long predicted_label = predicted_labels[j];
                const unsigned long true_label = get_label(chunks[i], chunk_labels[i], ranges[j], num_labels);

                num_dets[predicted_label]++;
                if (predicted_label == true_label)
                {
                    num_true_dets[true_label]++;
                }
            }
            for (unsigned long j = 0; j < chunk_labels[i].size(); ++j)
            {
                num_targets[chunk_labels[i][j]]++;
            }
        }


        ner_eval_metrics mets;
        mets.per_label_metrics.resize(num_targets.size());
        for (unsigned long i = 0; i < num_targets.size(); ++i)
        {
            mets.per_label_metrics[i].precision = num_true_dets[i]/num_dets[i];
            mets.per_label_metrics[i].recall = num_true_dets[i]/num_targets[i];
            mets.per_label_metrics[i].label = tags[i];
        }

        mets.overall_precision = sum(mat(num_true_dets))/sum(mat(num_dets));
        mets.overall_recall = sum(mat(num_true_dets))/sum(mat(num_targets));
        return mets;
    }

// ----------------------------------------------------------------------------------------

    std::ostream& operator<< (std::ostream& out_, const ner_eval_metrics& item)
    {
        unsigned long max_tag_length = 5;
        for (unsigned long i = 0; i < item.per_label_metrics.size(); ++i)
            max_tag_length = std::max<unsigned long>(max_tag_length, item.per_label_metrics[i].label.size());

        std::ostream out(out_.rdbuf());
        out.setf(std::ios_base::fixed, std::ios_base::floatfield);
        for (unsigned long i = 0; i < item.per_label_metrics.size(); ++i)
        {
            out << "label: "<< setw(max_tag_length) << item.per_label_metrics[i].label;
            double prec = item.per_label_metrics[i].precision;
            double recall = item.per_label_metrics[i].recall;
            out << " precision: "<<  setprecision(4) << prec << ",";
            out << " recall: "<<  setprecision(4) <<recall << ",";
            out << " F1: "<< setprecision(4) << 2*prec*recall/(prec+recall) << endl;
        }

        out << "all labels: " << string(max_tag_length-5,' ');
        double prec = item.overall_precision;
        double recall = item.overall_recall;
        out << " precision: "<<  setprecision(4) << prec << ",";
        out << " recall: "<<  setprecision(4) <<recall << ",";
        out << " F1: "<< setprecision(4) << 2*prec*recall/(prec+recall) << endl;

        return out_;
    }

// ----------------------------------------------------------------------------------------

}



