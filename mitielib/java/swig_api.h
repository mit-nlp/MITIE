// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MITLL_MITIE_JAVA_ApI_H_
#define MITLL_MITIE_JAVA_ApI_H_

// Define some swig type maps that tell swig what to call various instantiations of
// std::vector.
#ifdef SWIG
%include "std_string.i"
%include "std_vector.i"
%template(StringVector)         std::vector<std::string>;
%template(TokenIndexVector)     std::vector<TokenIndexPair>;
%template(EntityMentionVector)  std::vector<EntityMention>;
#endif


#include <string>
#include <sstream>
#include <fstream>
#include <dlib/error.h>
#include <mitie/conll_tokenizer.h>
#include <mitie/binary_relation_detector.h>
#include <mitie/named_entity_extractor.h>
#include <mitie/ner_trainer.h>
#include <mitie/micro_ner.h>
#include <mitie/micro_trainer.h>

// ----------------------------------------------------------------------------------------

inline std::string loadEntireFile (
    const std::string& filename
)
{
    std::ifstream fin(filename.c_str());
    if (!fin)
        throw dlib::error("Unable to open file " + filename);

    std::ostringstream sout;
    sout << fin.rdbuf();
    return sout.str();
}

// ----------------------------------------------------------------------------------------

inline std::vector<std::string> tokenize (
    const std::string& str 
)
{
    using namespace mitie;
    std::istringstream sin(str);
    // The conll_tokenizer splits the contents of an istream into a bunch of words and is
    // MITIE's default tokenization method. 
    conll_tokenizer tok(sin);
    std::vector<std::string> tokens;
    std::string token;
    // Read the tokens out of the file one at a time and store into tokens.
    while(tok(token))
        tokens.push_back(token);

    return tokens;
}

// ----------------------------------------------------------------------------------------

struct TokenIndexPair
{
    unsigned long index;
    std::string token;
};

std::vector<TokenIndexPair> tokenizeWithOffsets (
    const std::string& str 
)
{
    using namespace mitie;
    std::istringstream sin(str);
    // The conll_tokenizer splits the contents of an istream into a bunch of words and is
    // MITIE's default tokenization method. 
    conll_tokenizer tok(sin);
    std::vector<TokenIndexPair> tokens;
    TokenIndexPair p;
    // Read the tokens out of the file one at a time and store into tokens.
    while(tok(p.token, p.index))
    {
        tokens.push_back(p);
    }

    return tokens;
}

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

class EntityMention
{
public:
    EntityMention() : start(0),end(0),tag(0),score(0.0) {}
    EntityMention (int start_, int end_) : start(start_), end(end_), tag(0), score(0.0) {}
    EntityMention (int start_, int end_, int tag_, double score_) : start(start_), end(end_), tag(tag_), score(score_) {}

    int start;
    int end;
    int tag;
    double score;
};

struct BinaryRelation
{
    mitie::binary_relation item;
};

class TotalWordFeatureExtractor {
public:
    TotalWordFeatureExtractor(const std::string& filename)
    {
        string classname;
        dlib::deserialize(filename) >> classname;
        if (classname != "mitie::total_word_feature_extractor")
            throw dlib::error("This file does not contain a mitie::total_word_feature_extractor. Contained: " + classname);
        dlib::deserialize(filename) >> classname >> impl;
    }

private:
    friend class NamedEntityExtractor;
    friend class NerTrainer;
    friend class MicroNER;
    friend class MicroTrainer;
    mitie::total_word_feature_extractor impl;
};

class MicroNER
{
public:    
    MicroNER(const std::string& pureModelName                        
    ) :impl(pureModelName)
    {

    }

    std::vector<std::string> getPossibleNerTags (
    ) const
    {
        return impl.get_tag_name_strings();
    }

    std::vector<EntityMention> extractEntities (
        const TotalWordFeatureExtractor& extractorObject,
        const std::vector<std::string>& tokens
    ) const
    {
        std::vector<std::pair<unsigned long, unsigned long> > ranges;
        std::vector<unsigned long> predicted_labels;
        std::vector<double> predicted_scores;
        impl.predict(extractorObject.impl, tokens, ranges, predicted_labels, predicted_scores);
        std::vector<EntityMention> temp;
        for (unsigned long i = 0; i < ranges.size(); ++i)
            temp.push_back(EntityMention(ranges[i].first, ranges[i].second, predicted_labels[i], predicted_scores[i]));
        return temp;
    }

    std::vector<EntityMention> extractEntities (
        const TotalWordFeatureExtractor& extractorObject,
        const std::vector<TokenIndexPair>& tokens
    ) const
    {
        std::vector<std::string> temp;
        temp.reserve(tokens.size());
        for (unsigned long i = 0; i < tokens.size(); ++i)
        {
            temp.push_back(tokens[i].token);
        }
        return extractEntities(extractorObject, temp);
    }

private:
    mitie::micro_ner impl;
};

class NamedEntityExtractor
{
public:
    NamedEntityExtractor (
        const std::string& filename
    )
    {
        std::string classname;
        dlib::deserialize(filename) >> classname;
        if (classname != "mitie::named_entity_extractor")
            throw dlib::error("This file does not contain a mitie::named_entity_extractor. Contained: " + classname);
        dlib::deserialize(filename) >> classname >> impl;
    }

    NamedEntityExtractor(const std::string& pureModelName,
               const std::string& extractorName
    ) :impl(pureModelName, extractorName)
    {

    }

    NamedEntityExtractor(const std::string& pureModelName,
                         const TotalWordFeatureExtractor& extractorObject
    ) :impl(pureModelName, extractorObject.impl)
    {

    }

    std::vector<std::string> getPossibleNerTags (
    ) const
    {
        return impl.get_tag_name_strings();
    }

    void saveToDisk (
        const std::string& filename
    ) const
    {
        dlib::serialize(filename) << "mitie::named_entity_extractor" << impl;
    }

    std::vector<EntityMention> extractEntities (
        const std::vector<std::string>& tokens
    ) const
    {
        std::vector<std::pair<unsigned long, unsigned long> > ranges;
        std::vector<unsigned long> predicted_labels;
        std::vector<double> predicted_scores;
        impl.predict(tokens, ranges, predicted_labels, predicted_scores);
        std::vector<EntityMention> temp;
        for (unsigned long i = 0; i < ranges.size(); ++i)
            temp.push_back(EntityMention(ranges[i].first, ranges[i].second, predicted_labels[i], predicted_scores[i]));
        return temp;
    }

    std::vector<EntityMention> extractEntities (
        const std::vector<TokenIndexPair>& tokens
    ) const
    {
        std::vector<std::string> temp;
        temp.reserve(tokens.size());
        for (unsigned long i = 0; i < tokens.size(); ++i)
        {
            temp.push_back(tokens[i].token);
        }
        return extractEntities(temp);
    }

    BinaryRelation extractBinaryRelation(
        const std::vector<std::string>& tokens,
        const EntityMention& arg1,
        const EntityMention& arg2
    ) const
    {
        if (!(arg1.start < arg1.end && arg1.end <= tokens.size() &&
              arg2.start < arg2.end && arg2.end <= tokens.size()))
        {
            throw dlib::error("Invalid entity mention ranges given to NamedEntityExtractor.extractBinaryRelation().");
        }
        BinaryRelation temp;
        temp.item = extract_binary_relation(tokens,
                                            std::make_pair(arg1.start,arg1.end),
                                            std::make_pair(arg2.start,arg2.end),
                                            impl.get_total_word_feature_extractor());
        return temp;
    }
private:
    mitie::named_entity_extractor impl;
};

// ----------------------------------------------------------------------------------------

class BinaryRelationDetector
{
public:
    BinaryRelationDetector (
        const std::string& filename
    )
    {
        string classname;
        dlib::deserialize(filename) >> classname;
        if (classname != "mitie::binary_relation_detector")
            throw dlib::error("This file does not contain a mitie::binary_relation_detector. Contained: " + classname);
        dlib::deserialize(filename) >> classname >> impl;
    }

    void saveToDisk (
        const std::string& filename
    ) const
    {
        dlib::serialize(filename) << "mitie::binary_relation_detector" << impl;
    }

    std::string getNameString (
    ) const
    {
        return impl.relation_type;
    }

    double classify (
        const BinaryRelation& rel
    ) const
    {
        return impl(rel.item);
    }

private:
    mitie::binary_relation_detector impl;
};

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

class NerTrainingInstance {
public:
    NerTrainingInstance(std::vector<std::string> &tokens
    ) : impl(tokens)
    {
    }

    void addEntity(unsigned long start,
                   unsigned long length,
                   const char *label) 
    {
        impl.add_entity(start, length, label);
    }

    unsigned long getSize() 
    {
        return impl.num_tokens();
    }

private:
    friend class NerTrainer;
    mitie::ner_training_instance impl;
};

class NerMicroTrainingInstance {
public:
    NerMicroTrainingInstance(std::vector<std::string> &tokens
    ) : impl(tokens)
    {
    }

    void addEntity(unsigned long start,
                   unsigned long length,
                   const char *label) 
    {
        impl.add_entity(start, length, label);
    }

    unsigned long getSize() 
    {
        return impl.num_tokens();
    }

private:
    friend class MicroTrainer;
    mitie::ner_micro_training_instance impl;
};

class MicroTrainer
{
public:
    MicroTrainer() : impl() 
    {
    }

    void add(const NerMicroTrainingInstance& item) 
    {
        impl.add(item.impl);
    }

    void setThreadNum(unsigned long num) 
    {
        impl.set_num_threads(num);
    }

    void trainSeparateModels(
        const TotalWordFeatureExtractor& extractorObject, 
        const std::string& filename
    ) const
    {
        mitie::micro_ner obj = impl.train(extractorObject.impl);
        dlib::serialize(filename)
        << "mitie::named_entity_extractor_pure_model"
        << obj.get_df()
        << obj.get_segmenter()
        << obj.get_tag_name_strings();
    }
private:
    mitie::micro_trainer impl;
};

class NerTrainer
{
public:
    NerTrainer(const std::string& filename) : impl(filename) 
    {
    }

    NerTrainer( const TotalWordFeatureExtractor& extractorObject) : impl(extractorObject.impl)
    {

    }


    void add(const NerTrainingInstance& item) 
    {
        impl.add(item.impl);
    }

    void setThreadNum(unsigned long num) 
    {
        impl.set_num_threads(num);
    }

    void train(const std::string& filename) const
    {
        mitie::named_entity_extractor obj = impl.train();
        dlib::serialize(filename) << "mitie::named_entity_extractor" << obj;
    }

    void trainSeparateModels(const std::string& filename) const
    {
        mitie::named_entity_extractor obj = impl.train();
        dlib::serialize(filename)
        << "mitie::named_entity_extractor_pure_model"
        << obj.get_df()
        << obj.get_segmenter()
        << obj.get_tag_name_strings();
    }
private:
    mitie::ner_trainer impl;
};

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------


#endif // MITLL_MITIE_JAVA_ApI_H_


