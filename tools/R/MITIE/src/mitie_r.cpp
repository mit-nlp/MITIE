#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <sstream>

#include <mitie.h>
#include <mitie/binary_relation_detector.h>
#include <mitie/binary_relation_detector_trainer.h>
#include <mitie/conll_tokenizer.h>
#include <mitie/named_entity_extractor.h>
#include <mitie/ner_trainer.h>

#include <Rcpp.h>

using dlib::deserialize;
using dlib::serialize;
using std::pair;
using std::string;
using std::vector;

using namespace mitie;

// TODO : make mitie_tokenize_with_offsets function?
// TODO : constants for token_start, token_end, tag, and any others

// ------------------------------------------------------------------------------------------------
// global functions
// ------------------------------------------------------------------------------------------------

RcppExport SEXP mitie_load_entire_file_R (
    SEXP _filename
)
{
    BEGIN_RCPP
    std::string filename = Rcpp::as<std::string>(_filename);
    std::ifstream fin(filename.c_str());
    std::stringstream buffer;
    buffer << fin.rdbuf();
    return Rcpp::wrap(buffer.str());
    VOID_END_RCPP
}

RcppExport SEXP mitie_tokenize_R (
    SEXP _text
)
{
    BEGIN_RCPP
    string text = Rcpp::as<string>(_text);
    istringstream sin(text);
    conll_tokenizer tok(sin);
    vector<string> tokens;
    string token;
    while (tok(token))
    {
        tokens.push_back(token);
    }
    return Rcpp::wrap<vector<string> >(tokens);
    VOID_END_RCPP
}

RcppExport SEXP mitie_tokenize_with_offsets_R (
    SEXP _text
)
{
    BEGIN_RCPP
    string text = Rcpp::as<string>(_text);
    istringstream sin(text);
    conll_tokenizer tok(sin);
    vector<string> tokens;
    vector<unsigned long> offsets;
    string token;
    unsigned long offset;
    Rcpp::List list;
    while (tok(token, offset))
    {
        // add 1 to offset since R lists are 1-based and MITIE offsets are 0-based
        Rcpp::List element = Rcpp::List::create(
            Rcpp::Named("text") = token,
            Rcpp::Named("offset") = offset + 1
        );
        list.push_back(element);
    }
    return list;
    VOID_END_RCPP
}

pair<unsigned long, unsigned long> range(
    Rcpp::List entity)
{
    BEGIN_RCPP
    // convert entity = list(start = x, end = y) to a std::pair
    // NOTE: R is 1-based and slices are inclusive, whereas MITIE is 0-based and ranges are exclusive
    // - so R list(start = 3, end = 4) becomes MITIE (2, 4)
    
    unsigned long start = Rcpp::as<unsigned long>(entity["start"]) - 1;
    unsigned long end = Rcpp::as<unsigned long>(entity["end"]);
    
    return pair<unsigned long, unsigned long>(start, end);
    VOID_END_RCPP
}

pair<unsigned long, unsigned long> range(
    SEXP _entity
)
{
    BEGIN_RCPP
    return range(Rcpp::List(_entity));
    VOID_END_RCPP
}

// ------------------------------------------------------------------------------------------------
// named entity recognition
// ------------------------------------------------------------------------------------------------

RcppExport SEXP mitie_load_named_entity_extractor_R (
    SEXP _filename
)
{
    BEGIN_RCPP
    string ner_model_path = Rcpp::as<string>(_filename);
    named_entity_extractor* ner = new named_entity_extractor;
    string classname;
    deserialize(ner_model_path) >> classname >> *ner;
    return Rcpp::XPtr<named_entity_extractor>(ner, true);
    VOID_END_RCPP
}

RcppExport SEXP mitie_get_num_possible_ner_tags_R (
    SEXP _ner
)
{
    BEGIN_RCPP
    named_entity_extractor* ner = Rcpp::XPtr<named_entity_extractor>(_ner);
    unsigned long num_tags = ner->get_tag_name_strings().size();
    return Rcpp::wrap(num_tags);
    VOID_END_RCPP
}

RcppExport SEXP mitie_get_tag_name_strings_R (
    SEXP _ner
)
{
    BEGIN_RCPP
    named_entity_extractor* ner = Rcpp::XPtr<named_entity_extractor>(_ner);
    return Rcpp::wrap(ner->get_tag_name_strings());
    VOID_END_RCPP
}

RcppExport SEXP mitie_extract_entities_R (
    SEXP _ner,
    SEXP _tokens
)
{
    BEGIN_RCPP
    named_entity_extractor* ner = Rcpp::XPtr<named_entity_extractor>(_ner);
    vector<string> tokens = Rcpp::as<vector<string> >(_tokens);

    vector<pair<unsigned long, unsigned long> > ranges;
    vector<unsigned long> predicted_labels;

    (*ner)(tokens, ranges, predicted_labels);

    const unsigned long num_detections = ranges.size();
    Rcpp::List list(num_detections);

    vector<string> tags = ner->get_tag_name_strings();
    tags.push_back("O");
    
    for (unsigned long i = 0; i < num_detections; ++i) {
        // create R list object for detection
        // add 1 to first index since R indexing is 1-based
        // leave second index alone to allow tokens[first:second] in R
        Rcpp::List detection = Rcpp::List::create(
            Rcpp::Named("start") = ranges[i].first + 1,
            Rcpp::Named("end") = ranges[i].second,
            Rcpp::Named("tag") = predicted_labels[i] + 1
        );
        list[i] = detection;
    }
    
    return list;
    VOID_END_RCPP
}

// ------------------------------------------------------------------------------------------------
// binary relation detection
// ------------------------------------------------------------------------------------------------

RcppExport SEXP mitie_load_binary_relation_detector_R (
    SEXP _filename
)
{
    BEGIN_RCPP
    string filename = Rcpp::as<string>(_filename);
    binary_relation_detector* brd = new binary_relation_detector;
    string classname;
    deserialize(filename) >> classname >> *brd;
    return Rcpp::XPtr<binary_relation_detector>(brd, true);
    VOID_END_RCPP
}

RcppExport SEXP mitie_extract_binary_relation_R (
    SEXP _ner,
    SEXP _tokens,
    SEXP _arg1,
    SEXP _arg2
)
{
        BEGIN_RCPP
    named_entity_extractor* ner = Rcpp::XPtr<named_entity_extractor>(_ner);
    vector<string> tokens = Rcpp::as<vector<string> >(_tokens);
    
    binary_relation temp = extract_binary_relation(tokens, range(_arg1), range(_arg2), ner->get_total_word_feature_extractor());
    // copy to object allocated on heap so can wrap as external pointer
    binary_relation* br = new binary_relation(temp);
    
    return Rcpp::XPtr<binary_relation>(br, true);
    VOID_END_RCPP
}

RcppExport SEXP mitie_get_binary_relation_name_R (
    SEXP _brd
)
{
    BEGIN_RCPP
    binary_relation_detector* brd = Rcpp::XPtr<binary_relation_detector>(_brd);
    return Rcpp::wrap(brd->relation_type);
    VOID_END_RCPP
}

RcppExport SEXP mitie_score_binary_relation_R (
    SEXP _brd,
    SEXP _br
)
{
    BEGIN_RCPP
    binary_relation_detector* brd = Rcpp::XPtr<binary_relation_detector>(_brd);
    binary_relation* br = Rcpp::XPtr<binary_relation>(_br);
    double score = (*brd)(*br);
    return Rcpp::wrap(score);
    VOID_END_RCPP
}

RcppExport SEXP mitie_save_binary_relation_detector_R (
    SEXP _brd,
    SEXP _filename
)
{
    BEGIN_RCPP
    binary_relation_detector* brd = Rcpp::XPtr<binary_relation_detector>(_brd);
    string filename = Rcpp::as<string>(_filename);
    serialize(filename) << "mitie::binary_relation_detector" << (*brd);
    return Rcpp::wrap(true);
    VOID_END_RCPP
}

// ------------------------------------------------------------------------------------------------
// train named entity recognizer
// ------------------------------------------------------------------------------------------------

RcppExport SEXP mitie_create_ner_trainer_R (
    SEXP _total_word_feature_extractor_path
)
{
    BEGIN_RCPP
    string total_word_feature_extractor_path = Rcpp::as<string>(_total_word_feature_extractor_path);
    ner_trainer* trainer = new ner_trainer(total_word_feature_extractor_path);
    return Rcpp::XPtr<ner_trainer>(trainer, true);
    VOID_END_RCPP
}

RcppExport SEXP mitie_add_ner_training_instance_R (
    SEXP _trainer,
    SEXP _tokens,
    SEXP _entities
)
{
    BEGIN_RCPP
    ner_trainer* trainer = Rcpp::XPtr<ner_trainer>(_trainer);
    vector<string> tokens = Rcpp::as<vector<string> >(_tokens);
    Rcpp::List entities(_entities);
    
    ner_training_instance sample(tokens);
 
    // TODO : check overlaps_any_entity() ?
    for (unsigned long i = 0; i < entities.size(); i++) {
        Rcpp::List entity(entities[i]);
        string tag = Rcpp::as<string>(entity["tag"]);
        sample.add_entity(range(entity), tag);
    }

    trainer->add(sample);
    
    // return R_NilValue;
    return Rcpp::wrap(true);
    VOID_END_RCPP
}

RcppExport SEXP mitie_ner_trainer_get_size_R (
    SEXP _trainer
)
{
    BEGIN_RCPP
    ner_trainer* trainer = Rcpp::XPtr<ner_trainer>(_trainer);
    return Rcpp::wrap(trainer->size());
    VOID_END_RCPP
}

RcppExport SEXP mitie_ner_trainer_get_num_threads_R (
    SEXP _trainer
)
{
    BEGIN_RCPP
    ner_trainer* trainer = Rcpp::XPtr<ner_trainer>(_trainer);
    return Rcpp::wrap(trainer->get_num_threads());
    VOID_END_RCPP
}

RcppExport SEXP mitie_ner_trainer_set_num_threads_R (
    SEXP _trainer,
    SEXP _num_threads
)
{
    BEGIN_RCPP
    ner_trainer* trainer = Rcpp::XPtr<ner_trainer>(_trainer);
    unsigned long num_threads = Rcpp::as<unsigned long>(_num_threads);
    trainer->set_num_threads(num_threads);
    return Rcpp::wrap(num_threads);
    VOID_END_RCPP
}

RcppExport SEXP mitie_ner_trainer_get_beta_R (
    SEXP _trainer
)
{
    BEGIN_RCPP
    ner_trainer* trainer = Rcpp::XPtr<ner_trainer>(_trainer);
    return Rcpp::wrap(trainer->get_beta());
    VOID_END_RCPP
}

RcppExport SEXP mitie_ner_trainer_set_beta_R (
    SEXP _trainer,
    SEXP _beta
)
{
    BEGIN_RCPP
    ner_trainer* trainer = Rcpp::XPtr<ner_trainer>(_trainer);
    double beta = Rcpp::as<double>(_beta);
    trainer->set_beta(beta);
    return Rcpp::wrap(beta);
    VOID_END_RCPP
}

RcppExport SEXP mitie_train_named_entity_extractor_R (
    SEXP _trainer
)
{
    BEGIN_RCPP
    ner_trainer* trainer = Rcpp::XPtr<ner_trainer>(_trainer);
    
    // copy to object allocated on heap so can wrap as external pointer
    named_entity_extractor temp = trainer->train();
    named_entity_extractor* ner = new named_entity_extractor(temp);
    return Rcpp::XPtr<named_entity_extractor>(ner);
    VOID_END_RCPP
}

RcppExport SEXP mitie_save_named_entity_extractor_R (
    SEXP _ner,
    SEXP _filename
)
{
    BEGIN_RCPP
    named_entity_extractor* ner = Rcpp::XPtr<named_entity_extractor>(_ner);
    string filename = Rcpp::as<string>(_filename);
    serialize(filename) << "mitie::named_entity_extractor" << (*ner);
    return Rcpp::wrap(true);
    VOID_END_RCPP
}

// ------------------------------------------------------------------------------------------------
// train binary relation detector
// ------------------------------------------------------------------------------------------------

RcppExport SEXP mitie_create_binary_relation_trainer_R (
    SEXP _relation_name,
    SEXP _ner
)
{
    BEGIN_RCPP
    string relation_name = Rcpp::as<string>(_relation_name);
    named_entity_extractor* ner = Rcpp::XPtr<named_entity_extractor>(_ner);
    binary_relation_detector_trainer* trainer = new binary_relation_detector_trainer(relation_name, *ner);
    return Rcpp::XPtr<binary_relation_detector_trainer>(trainer);
    VOID_END_RCPP
}

RcppExport SEXP mitie_add_positive_binary_relation_R (
    SEXP _trainer,
    SEXP _tokens,
    SEXP _entity1,
    SEXP _entity2
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    vector<string> tokens = Rcpp::as<vector<string> >(_tokens);
    trainer->add_positive_binary_relation(tokens, range(_entity1), range(_entity2));
    return Rcpp::wrap(true);
    VOID_END_RCPP
}

RcppExport SEXP mitie_add_negative_binary_relation_R (
    SEXP _trainer,
    SEXP _tokens,
    SEXP _entity1,
    SEXP _entity2
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    vector<string> tokens = Rcpp::as<vector<string> >(_tokens);
    trainer->add_negative_binary_relation(tokens, range(_entity1), range(_entity2));
    return Rcpp::wrap(true);
    VOID_END_RCPP
}

RcppExport SEXP mitie_binary_relation_trainer_get_relation_name_R ( 
    SEXP _trainer
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    return Rcpp::wrap(trainer->get_relation_name());
    VOID_END_RCPP
}

RcppExport SEXP mitie_binary_relation_trainer_get_num_positive_examples_R (
    SEXP _trainer
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    return Rcpp::wrap(trainer->num_positive_examples());
    VOID_END_RCPP
}

RcppExport SEXP mitie_binary_relation_trainer_get_num_negative_examples_R (
    SEXP _trainer
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    return Rcpp::wrap(trainer->num_negative_examples());
    VOID_END_RCPP
}

RcppExport SEXP mitie_binary_relation_trainer_get_num_threads_R (
    SEXP _trainer
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    return Rcpp::wrap(trainer->get_num_threads());
    VOID_END_RCPP
}

RcppExport SEXP mitie_binary_relation_trainer_set_num_threads_R (
    SEXP _trainer,
    SEXP _num_threads
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    unsigned long num_threads = Rcpp::as<unsigned long>(_num_threads);
    trainer->set_num_threads(num_threads);
    return Rcpp::wrap(num_threads);
    VOID_END_RCPP
}

RcppExport SEXP mitie_binary_relation_trainer_get_beta_R (
    SEXP _trainer
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    return Rcpp::wrap(trainer->get_beta());
    VOID_END_RCPP
}

RcppExport SEXP mitie_binary_relation_trainer_set_beta_R (
    SEXP _trainer,
    SEXP _beta
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    double beta = Rcpp::as<double>(_beta);
    trainer->set_beta(beta);
    return Rcpp::wrap(beta);
    VOID_END_RCPP
}

RcppExport SEXP mitie_train_binary_relation_detector_R (
    SEXP _trainer
)
{
    BEGIN_RCPP
    binary_relation_detector_trainer* trainer = Rcpp::XPtr<binary_relation_detector_trainer>(_trainer);
    binary_relation_detector temp = trainer->train();
    binary_relation_detector* brd = new binary_relation_detector(temp);
    return Rcpp::XPtr<binary_relation_detector>(brd, true);
    VOID_END_RCPP
}
