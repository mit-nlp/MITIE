// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#include <mitie.h>

#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <dlib/vectorstream.h>
#include <mitie/named_entity_extractor.h>
#include <mitie/conll_tokenizer.h>
#include <mitie/binary_relation_detector.h>
#include <mitie/ner_trainer.h>
#include <mitie/binary_relation_detector_trainer.h>

using namespace mitie;

namespace 
{

    /*
        The point of this memory allocation complexity here is to allow us to implement
        mitie_free().  That is, mitie_free() should know how to delete anything allocated
        by any function in the MITIE API.  To accomplish this, we put a header in front of
        each allocation that tells us what kind of object it is so we can appropriately
        destruct it.
    */

    enum mitie_object_type
    {
        MITIE_NOT_A_MITIE_OBJECT = 0,
        MITIE_NAMED_ENTITY_EXTRACTOR = 1234,
        MITIE_NAMED_ENTITY_DETECTIONS,
        MITIE_RAW_MEMORY,
        MITIE_BINARY_RELATION_DETECTOR,
        MITIE_BINARY_RELATION,
        MITIE_BINARY_RELATION_TRAINER,
        MITIE_NER_TRAINING_INSTANCE,
        MITIE_NER_TRAINER
    };

    template <typename T>
    struct allocatable_types;

    // types that allocate() can create
    template <> struct allocatable_types<named_entity_extractor>        { const static mitie_object_type type = MITIE_NAMED_ENTITY_EXTRACTOR; };
    template <> struct allocatable_types<mitie_named_entity_detections> { const static mitie_object_type type = MITIE_NAMED_ENTITY_DETECTIONS; };
    template <> struct allocatable_types<binary_relation_detector>      { const static mitie_object_type type = MITIE_BINARY_RELATION_DETECTOR; };
    template <> struct allocatable_types<binary_relation_detector_trainer>{ const static mitie_object_type type = MITIE_BINARY_RELATION_TRAINER; };
    template <> struct allocatable_types<binary_relation>               { const static mitie_object_type type = MITIE_BINARY_RELATION; };
    template <> struct allocatable_types<ner_training_instance>         { const static mitie_object_type type = MITIE_NER_TRAINING_INSTANCE; };
    template <> struct allocatable_types<ner_trainer>                   { const static mitie_object_type type = MITIE_NER_TRAINER; };

// ----------------------------------------------------------------------------------------

    const int min_alignment = 16;
    int memory_block_type (const void* ptr)
    {
        return *((int*)((char*)ptr-min_alignment));
    }

    template <typename T>
    T& checked_cast(void* ptr) 
    {
        assert(ptr);
        assert(memory_block_type(ptr) == allocatable_types<T>::type);
        return *static_cast<T*>(ptr);
    }

    template <typename T>
    const T& checked_cast(const void* ptr) 
    {
        assert(ptr);
        assert(memory_block_type(ptr) == allocatable_types<T>::type);
        return *static_cast<const T*>(ptr);
    }

// ----------------------------------------------------------------------------------------

    template <typename T>
    void destroy(void* object)
    /*!
        ensures
            - frees an object allocated by the allocate() or allocate_bytes() method defined below.
    !*/
    {
        ((T*)object)->~T();
        *((int*)((char*)object-min_alignment)) = MITIE_NOT_A_MITIE_OBJECT;
        free(((char*)object)-min_alignment);
    }

    template <typename T>
    T* allocate()
    {
        const mitie_object_type type = allocatable_types<T>::type;
        void* temp = malloc(sizeof(T)+min_alignment);
        if (temp == 0)
            throw std::bad_alloc();

        *((int*)temp) = type;

        try
        {
            return new((char*)temp+min_alignment) T();
        }
        catch (...)
        {
            free(temp);
            throw std::bad_alloc();
        }
    }

    template <typename T, typename A1>
    T* allocate(const A1& arg1)
    /*!
        This function is just like allocate() except it passes arg1 to T's constructor.
    !*/
    {
        const mitie_object_type type = allocatable_types<T>::type;
        void* temp = malloc(sizeof(T)+min_alignment);
        if (temp == 0)
            throw std::bad_alloc();

        *((int*)temp) = type;

        try
        {
            return new((char*)temp+min_alignment) T(arg1);
        }
        catch (...)
        {
            free(temp);
            throw std::bad_alloc();
        }
    }

    template <typename T, typename A1, typename A2>
    T* allocate(const A1& arg1, const A2& arg2)
    /*!
        This function is just like allocate() except it passes arg1 and arg2 to T's constructor.
    !*/
    {
        const mitie_object_type type = allocatable_types<T>::type;
        void* temp = malloc(sizeof(T)+min_alignment);
        if (temp == 0)
            throw std::bad_alloc();

        *((int*)temp) = type;

        try
        {
            return new((char*)temp+min_alignment) T(arg1,arg2);
        }
        catch (...)
        {
            free(temp);
            throw std::bad_alloc();
        }
    }

    void* allocate_bytes (size_t num)
    /*!
        ensures
            - allocate a block of num bytes and return a pointer to the first byte.
    !*/
    {
        void* temp = malloc(num + min_alignment);
        if (temp == 0)
            throw std::bad_alloc();
        *((int*)temp) = MITIE_RAW_MEMORY;
        return (char*)temp + min_alignment;
    }
}

extern "C"
{

// ----------------------------------------------------------------------------------------

    char* mitie_load_entire_file (
        const char* filename
    )
    {
        try
        {
            ifstream fin(filename);
            if (!fin)
                return 0;
            std::vector<char> buf;
            dlib::vectorstream out(buf);
            out << fin.rdbuf();

            char* final_buf = (char*)allocate_bytes(buf.size()+1);
            if (!final_buf)
                return 0;
            memcpy(final_buf, &buf[0], buf.size());
            final_buf[buf.size()] = '\0';
            return final_buf;
        }
        catch (...)
        {
            return 0;
        }
    }

// ----------------------------------------------------------------------------------------

    static char** std_vector_to_double_ptr (
        const std::vector<std::string>& words
    )
    {
        size_t data_size = 0;
        for (unsigned long i = 0; i < words.size(); ++i)
            data_size += words[i].size() + 1; // +1 for the NULL terminator

        // account for the size of the char** array.  +1 for the NULL terminator
        const size_t array_size = sizeof(char*)*(words.size() + 1);

        // now allocate enough space for the result
        char* buf = (char*)allocate_bytes(array_size + data_size);

        char** array = (char**)buf;
        char* next = buf+array_size;
        for (unsigned long i = 0; i < words.size(); ++i)
        {
            array[i] = next;
            strcpy(next, words[i].c_str());
            next += words[i].size()+1;
        }
        array[words.size()] = 0;

        return array;
    }

// ----------------------------------------------------------------------------------------

    char** mitie_tokenize (
        const char* text
    )
    {
        assert(text);
        try
        {
            // first tokenize the text
            istringstream sin(text);
            conll_tokenizer tok(sin);
            std::vector<std::string> words;
            string word;
            while(tok(word))
                words.push_back(word);

            return std_vector_to_double_ptr(words);
        }
        catch (...)
        {
            return NULL;
        }
    }

// ----------------------------------------------------------------------------------------

    char** mitie_tokenize_with_offsets (
        const char* text,
        unsigned long** token_offsets
    )
    {
        assert(text);
        assert(token_offsets);
        char** tokens = NULL;
        try
        {
            // first tokenize the text
            istringstream sin(text);
            conll_tokenizer tok(sin);
            std::vector<std::string> words;
            std::vector<unsigned long> offsets;
            string word;
            unsigned long offset;
            while(tok(word,offset))
            {
                words.push_back(word);
                offsets.push_back(offset);
            }

            tokens = std_vector_to_double_ptr(words);

            (*token_offsets) = (unsigned long*)allocate_bytes(offsets.size()*sizeof(unsigned long));
            for (unsigned long i = 0; i < offsets.size(); ++i)
                (*token_offsets)[i] = offsets[i];

            return tokens;
        }
        catch (...)
        {
            mitie_free(tokens);
            return NULL;
        }
    }

// ----------------------------------------------------------------------------------------

    char** mitie_tokenize_file (
        const char* filename
    )
    {
        char* text = mitie_load_entire_file(filename);
        if (!text)
            return 0;

        char** tokens = mitie_tokenize(text);
        mitie_free(text);
        return tokens;
    }

// ----------------------------------------------------------------------------------------

    struct mitie_named_entity_detections
    {
        std::vector<std::pair<unsigned long, unsigned long> > ranges;
        std::vector<unsigned long> predicted_labels;
        std::vector<double> predicted_scores;
        std::vector<std::string> tags;
    };


    void mitie_free (
        void* object 
    )
    {
        if (object == 0)
            return;

        switch(memory_block_type(object))
        {
            case MITIE_NAMED_ENTITY_EXTRACTOR:
                destroy<named_entity_extractor>(object);
                break;
            case MITIE_NAMED_ENTITY_DETECTIONS:
                destroy<mitie_named_entity_detections>(object);
                break;
            case MITIE_RAW_MEMORY:
                destroy<char>(object);
                break;
            case MITIE_BINARY_RELATION_DETECTOR:
                destroy<binary_relation_detector>(object);
                break;
            case MITIE_BINARY_RELATION:
                destroy<binary_relation>(object);
                break;
            case MITIE_BINARY_RELATION_TRAINER:
                destroy<binary_relation_detector_trainer>(object);
                break;
            case MITIE_NER_TRAINING_INSTANCE:
                destroy<ner_training_instance>(object);
                break;
            case MITIE_NER_TRAINER:
                destroy<ner_trainer>(object);
                break;
            default:
                std::cerr << "ERROR, mitie_free() called on non-MITIE object or called twice." << std::endl;
                assert(false);
                abort();

        }

    }

// ----------------------------------------------------------------------------------------

    mitie_named_entity_extractor* mitie_load_named_entity_extractor (
        const char* filename
    )
    {
        assert(filename != NULL);

        named_entity_extractor* impl = 0;
        try
        {
            string classname;
            impl = allocate<named_entity_extractor>();
            dlib::deserialize(filename) >> classname;
            if (classname != "mitie::named_entity_extractor")
                throw dlib::error("This file does not contain a mitie::named_entity_extractor. Contained: " + classname);
            dlib::deserialize(filename) >> classname >> *impl;
            return (mitie_named_entity_extractor*)impl;
        }
        catch(std::exception& e)
        {
#ifndef NDEBUG
            cerr << "Error loading MITIE model file: " << filename << "\n" << e.what() << endl;
#endif
            mitie_free(impl);
            return NULL;
        }
        catch(...)
        {
            mitie_free(impl);
            return NULL;
        }
    }

    unsigned long mitie_get_num_possible_ner_tags (
        const mitie_named_entity_extractor* ner_
    )
    {
        return checked_cast<named_entity_extractor>(ner_).get_tag_name_strings().size();
    }

    const char* mitie_get_named_entity_tagstr (
        const mitie_named_entity_extractor* ner_,
        unsigned long idx
    )
    {
        assert(idx < mitie_get_num_possible_ner_tags(ner_));
        return checked_cast<named_entity_extractor>(ner_).get_tag_name_strings()[idx].c_str();
    }

// ----------------------------------------------------------------------------------------

    mitie_named_entity_detections* mitie_extract_entities (
        const mitie_named_entity_extractor* ner_,
        char** tokens 
    )
    {
        const named_entity_extractor& ner = checked_cast<named_entity_extractor>(ner_);

        assert(tokens != NULL);

        mitie_named_entity_detections* impl = 0;

        try
        {
            impl = allocate<mitie_named_entity_detections>();

            std::vector<std::string> words;
            for (unsigned long i = 0; tokens[i]; ++i)
                words.push_back(tokens[i]);

            ner.predict(words, impl->ranges, impl->predicted_labels, impl->predicted_scores);
            impl->tags = ner.get_tag_name_strings();
            return impl;
        }
        catch(...)
        {
            mitie_free(impl);
            return NULL;
        }

    }

    unsigned long mitie_ner_get_num_detections (
        const mitie_named_entity_detections* dets
    )
    {
        assert(dets);
        return dets->ranges.size();
    }

    unsigned long mitie_ner_get_detection_position (
        const mitie_named_entity_detections* dets,
        unsigned long idx
    )
    {
        assert(dets);
        assert(idx < mitie_ner_get_num_detections(dets));
        return dets->ranges[idx].first;
    }

    unsigned long mitie_ner_get_detection_length (
        const mitie_named_entity_detections* dets,
        unsigned long idx
    )
    {
        assert(dets);
        assert(idx < mitie_ner_get_num_detections(dets));
        return dets->ranges[idx].second - dets->ranges[idx].first;
    }

    unsigned long mitie_ner_get_detection_tag (
        const mitie_named_entity_detections* dets,
        unsigned long idx
    )
    {
        assert(dets);
        assert(idx < mitie_ner_get_num_detections(dets));
        return dets->predicted_labels[idx];
    }

    double mitie_ner_get_detection_score (
        const mitie_named_entity_detections* dets,
        unsigned long idx
    )
    {
        assert(dets);
        assert(idx < mitie_ner_get_num_detections(dets));
        return dets->predicted_scores[idx];
    }

    const char* mitie_ner_get_detection_tagstr (
        const mitie_named_entity_detections* dets,
        unsigned long idx
    )
    {
        assert(dets);
        assert(idx < mitie_ner_get_num_detections(dets));
        unsigned long tag = dets->predicted_labels[idx];
        return dets->tags[tag].c_str();
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    mitie_binary_relation_detector* mitie_load_binary_relation_detector (
        const char* filename
    )
    {
        assert(filename != NULL);

        binary_relation_detector* impl = 0;
        try
        {
            string classname;
            impl = allocate<binary_relation_detector>();
            dlib::deserialize(filename) >> classname;
            if (classname != "mitie::binary_relation_detector")
                throw dlib::error("This file does not contain a mitie::binary_relation_detector. Contained: " + classname);
            dlib::deserialize(filename) >> classname >> *impl;
            return (mitie_binary_relation_detector*)impl;
        }
        catch(std::exception& e)
        {
#ifndef NDEBUG
            cerr << "Error loading MITIE model file: " << filename << "\n" << e.what() << endl;
#endif
            mitie_free(impl);
            return NULL;
        }
        catch(...)
        {
            mitie_free(impl);
            return NULL;
        }
    }

// ----------------------------------------------------------------------------------------

    const char* mitie_binary_relation_detector_name_string (
        const mitie_binary_relation_detector* detector_
    )
    {
        return checked_cast<binary_relation_detector>(detector_).relation_type.c_str();
    }

// ----------------------------------------------------------------------------------------

    int mitie_entities_overlap (
        unsigned long arg1_start,
        unsigned long arg1_length,
        unsigned long arg2_start,
        unsigned long arg2_length
    )
    {
        // find intersection range
        const unsigned long left = std::max(arg1_start, arg2_start);
        const unsigned long right = std::min(arg1_start+arg1_length, arg2_start+arg2_length);
        // if the range is not empty
        if (left < right)
            return 1;
        else
            return 0;
    }

// ----------------------------------------------------------------------------------------

    mitie_binary_relation* mitie_extract_binary_relation (
        const mitie_named_entity_extractor* ner_,
        char** tokens,
        unsigned long arg1_start,
        unsigned long arg1_length,
        unsigned long arg2_start,
        unsigned long arg2_length
    )
    {
        const named_entity_extractor& ner = checked_cast<named_entity_extractor>(ner_);
        assert(arg1_length > 0);
        assert(arg2_length > 0);
        assert(mitie_entities_overlap(arg1_start,arg1_length,arg2_start,arg2_length) == 0);

        binary_relation* br = NULL;
        try
        {
            // crop out tokens in a window around the two arguments and store them into a
            // std::vector.
            const unsigned long window_size = 5;
            unsigned long begin = std::min(arg1_start, arg2_start);
            if (begin > window_size)
                begin -= window_size;
            else 
                begin = 0;

            const unsigned long end = std::max(arg1_start+arg1_length, arg2_start+arg2_length)+window_size;

            std::vector<std::string> words;
            for (unsigned long i = begin; tokens[i] && i < end; ++i)
                words.push_back(tokens[i]);

            // adjust the argument indices since we clipped out only part of the tokens
            arg1_start -= begin;
            arg2_start -= begin;

            br = allocate<binary_relation>();
            *br = extract_binary_relation(words, std::make_pair(arg1_start,arg1_start+arg1_length),
                                                 std::make_pair(arg2_start,arg2_start+arg2_length),
                                                 ner.get_total_word_feature_extractor());
            return (mitie_binary_relation*)br;
        }
        catch (std::exception& e)
        {
#ifndef NDEBUG
            cerr << e.what() << endl;
#endif
            mitie_free(br);
            return NULL;
        }
        catch (...)
        {
            mitie_free(br);
            return NULL;
        }
    }

    int mitie_classify_binary_relation (
        const mitie_binary_relation_detector* detector_,
        const mitie_binary_relation* relation_,
        double* score
    )
    {
        const binary_relation_detector& detector = checked_cast<binary_relation_detector>(detector_);
        const binary_relation& relation = checked_cast<binary_relation>(relation_);

        assert(score);

        try
        {
            *score = detector(relation);
            return 0;
        }
        catch (std::exception& e)
        {
#ifndef NDEBUG
            cerr << e.what() << endl;
#endif
            return 1;
        }
        catch (...)
        {
            return 1;
        }
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
//                                      TRAINING ROUTINES
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    int mitie_save_named_entity_extractor (
        const char* filename,
        const mitie_named_entity_extractor* ner_
    )
    {
        const named_entity_extractor& ner = checked_cast<named_entity_extractor>(ner_);
        assert(filename);

        try
        {
            dlib::serialize(filename) << "mitie::named_entity_extractor" << ner;
            return 0;
        }
        catch (std::exception& e)
        {
#ifndef NDEBUG
            cerr << "Error saving MITIE model file: " << filename << "\n" << e.what() << endl;
#endif
            return 1;
        }
        catch (...)
        {
#ifndef NDEBUG
            cerr << "Error saving MITIE model file: " << filename << endl;
#endif
            return 1;
        }
    }

// ----------------------------------------------------------------------------------------

    int mitie_save_binary_relation_detector (
        const char* filename,
        const mitie_binary_relation_detector* detector_ 
    )
    {
        const binary_relation_detector& detector = checked_cast<binary_relation_detector>(detector_);
        assert(filename);

        try
        {
            dlib::serialize(filename) << "mitie::binary_relation_detector" << detector;
            return 0;
        }
        catch (std::exception& e)
        {
#ifndef NDEBUG
            cerr << "Error saving MITIE model file: " << filename << "\n" << e.what() << endl;
#endif
            return 1;
        }
        catch (...)
        {
#ifndef NDEBUG
            cerr << "Error saving MITIE model file: " << filename << endl;
#endif
            return 1;
        }
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    mitie_ner_training_instance* mitie_create_ner_training_instance (
        char** tokens
    )
    {
        assert(tokens != NULL);

        try
        {
            std::vector<std::string> words;
            for (unsigned long i = 0; tokens[i]; ++i)
                words.push_back(tokens[i]);

            return (mitie_ner_training_instance*)allocate<ner_training_instance>(words);
        }
        catch(...)
        {
            return NULL;
        }
    }

// ----------------------------------------------------------------------------------------

    unsigned long mitie_ner_training_instance_num_tokens (
        const mitie_ner_training_instance* instance_
    )
    {
        return checked_cast<ner_training_instance>(instance_).num_tokens();
    }

// ----------------------------------------------------------------------------------------

    unsigned long mitie_ner_training_instance_num_entities (
        const mitie_ner_training_instance* instance_
    )
    {
        return checked_cast<ner_training_instance>(instance_).num_entities();
    }

// ----------------------------------------------------------------------------------------

    int mitie_overlaps_any_entity (
        mitie_ner_training_instance* instance_,
        unsigned long start,
        unsigned long length
    )
    {
        const ner_training_instance& instance = checked_cast<ner_training_instance>(instance_);
        assert(length > 0);
        assert(start+length <= mitie_ner_training_instance_num_tokens(instance_));
        if (instance.overlaps_any_entity(start, length))
            return 1;
        else
            return 0;
    }

// ----------------------------------------------------------------------------------------

    int mitie_add_ner_training_entity (
        mitie_ner_training_instance* instance_,
        unsigned long start,
        unsigned long length,
        const char* label
    )
    {
        ner_training_instance& instance = checked_cast<ner_training_instance>(instance_);
        assert(label);
        assert(length > 0);
        assert(start+length <= mitie_ner_training_instance_num_tokens(instance_));
        assert(mitie_overlaps_any_entity(instance_, start, length) == 0);

        try
        {
            instance.add_entity(start, length, label);
            return 0;
        }
        catch(...)
        {
            return 1;
        }
    }

// ----------------------------------------------------------------------------------------

    mitie_ner_trainer* mitie_create_ner_trainer (
        const char* filename
    )
    {
        assert(filename != NULL);

        try
        {
            return (mitie_ner_trainer*)allocate<ner_trainer>(filename);
        }
        catch(...)
        {
            return NULL;
        }
    }

// ----------------------------------------------------------------------------------------

    unsigned long mitie_ner_trainer_size (
        const mitie_ner_trainer* trainer_
    )
    {
        return checked_cast<ner_trainer>(trainer_).size();
    }

// ----------------------------------------------------------------------------------------

    int mitie_add_ner_training_instance(
        mitie_ner_trainer* trainer_,
        const mitie_ner_training_instance* instance_
    )
    {
        ner_trainer& trainer = checked_cast<ner_trainer>(trainer_);
        const ner_training_instance& instance = checked_cast<ner_training_instance>(instance_);
        try
        {
            trainer.add(instance);
            return 0;
        }
        catch (...)
        {
            return 1;
        }
    }

// ----------------------------------------------------------------------------------------

    void mitie_ner_trainer_set_beta (
        mitie_ner_trainer* trainer_,
        double beta
    )
    {
        assert(beta >= 0);
        checked_cast<ner_trainer>(trainer_).set_beta(beta);
    }

// ----------------------------------------------------------------------------------------

    double mitie_ner_trainer_get_beta (
        const mitie_ner_trainer* trainer_
    )
    {
        return checked_cast<ner_trainer>(trainer_).get_beta();
    }

// ----------------------------------------------------------------------------------------

    void mitie_ner_trainer_set_num_threads (
        mitie_ner_trainer* trainer_,
        unsigned long num_threads 
    )
    {
        checked_cast<ner_trainer>(trainer_).set_num_threads(num_threads);
    }

// ----------------------------------------------------------------------------------------

    unsigned long mitie_ner_trainer_get_num_threads (
        const mitie_ner_trainer* trainer_
    )
    {
        return checked_cast<ner_trainer>(trainer_).get_num_threads();
    }

// ----------------------------------------------------------------------------------------

    mitie_named_entity_extractor* mitie_train_named_entity_extractor (
        const mitie_ner_trainer* trainer_
    )
    {
        const ner_trainer& trainer = checked_cast<ner_trainer>(trainer_);
        assert(mitie_ner_trainer_size(trainer_) > 0);
        try
        {
            return (mitie_named_entity_extractor*)allocate<named_entity_extractor>(trainer.train());
        }
        catch(...)
        {
            return 0;
        }
    }
    
// ----------------------------------------------------------------------------------------

    mitie_binary_relation_trainer* mitie_create_binary_relation_trainer (
        const char* relation_name,
        const mitie_named_entity_extractor* ner_
    )
    {
        const named_entity_extractor& ner = checked_cast<named_entity_extractor>(ner_);
        assert(relation_name);

        try
        {
            return (mitie_binary_relation_trainer*)allocate<binary_relation_detector_trainer>(relation_name,ner);
        }
        catch(...)
        {
            return NULL;
        }
    }

// ----------------------------------------------------------------------------------------

    unsigned long mitie_binary_relation_trainer_num_positive_examples (
        const mitie_binary_relation_trainer* trainer_
    )
    {
        return checked_cast<binary_relation_detector_trainer>(trainer_).num_positive_examples();
    }

// ----------------------------------------------------------------------------------------

    unsigned long mitie_binary_relation_trainer_num_negative_examples (
        const mitie_binary_relation_trainer* trainer_
    )
    {
        return checked_cast<binary_relation_detector_trainer>(trainer_).num_negative_examples();
    }

// ----------------------------------------------------------------------------------------

    int mitie_add_positive_binary_relation (
        mitie_binary_relation_trainer* trainer_,
        char** tokens,
        unsigned long arg1_start,
        unsigned long arg1_length,
        unsigned long arg2_start,
        unsigned long arg2_length
    )
    {
        binary_relation_detector_trainer& trainer =  checked_cast<binary_relation_detector_trainer>(trainer_);
        assert(tokens);
        assert(arg1_length > 0);
        assert(arg2_length > 0);
        assert(mitie_entities_overlap(arg1_start,arg1_length,arg2_start,arg2_length) == 0);
        try
        {
            std::vector<std::string> words;
            while(*tokens)
                words.push_back(*tokens++);
            trainer.add_positive_binary_relation(words, arg1_start, arg1_length, arg2_start, arg2_length);
            return 0;
        }
        catch(...)
        {
            return 1;
        }
    }

// ----------------------------------------------------------------------------------------

    int mitie_add_negative_binary_relation (
        mitie_binary_relation_trainer* trainer_,
        char** tokens,
        unsigned long arg1_start,
        unsigned long arg1_length,
        unsigned long arg2_start,
        unsigned long arg2_length
    )
    {
        binary_relation_detector_trainer& trainer =  checked_cast<binary_relation_detector_trainer>(trainer_);
        assert(tokens);
        assert(arg1_length > 0);
        assert(arg2_length > 0);
        assert(mitie_entities_overlap(arg1_start,arg1_length,arg2_start,arg2_length) == 0);
        try
        {
            std::vector<std::string> words;
            while(*tokens)
                words.push_back(*tokens++);
            trainer.add_negative_binary_relation(words, arg1_start, arg1_length, arg2_start, arg2_length);
            return 0;
        }
        catch(...)
        {
            return 1;
        }
    }

// ----------------------------------------------------------------------------------------

    void mitie_binary_relation_trainer_set_beta (
        mitie_binary_relation_trainer* trainer_,
        double beta
    )
    {
        assert(beta >= 0);
        checked_cast<binary_relation_detector_trainer>(trainer_).set_beta(beta);
    }

// ----------------------------------------------------------------------------------------

    double mitie_binary_relation_trainer_get_beta (
        const mitie_binary_relation_trainer* trainer_
    )
    {
        return checked_cast<binary_relation_detector_trainer>(trainer_).get_beta();
    }

// ----------------------------------------------------------------------------------------

    void mitie_binary_relation_trainer_set_num_threads (
        mitie_binary_relation_trainer* trainer_,
        unsigned long num_threads 
    )
    {
        checked_cast<binary_relation_detector_trainer>(trainer_).set_num_threads(num_threads);
    }

// ----------------------------------------------------------------------------------------

    unsigned long mitie_binary_relation_trainer_get_num_threads (
        const mitie_binary_relation_trainer* trainer_
    )
    {
        return checked_cast<binary_relation_detector_trainer>(trainer_).get_num_threads();
    }

// ----------------------------------------------------------------------------------------

    mitie_binary_relation_detector* mitie_train_binary_relation_detector (
        const mitie_binary_relation_trainer* trainer_
    )
    {
        assert(mitie_binary_relation_trainer_num_positive_examples(trainer_) > 0);
        assert(mitie_binary_relation_trainer_num_negative_examples(trainer_) > 0);
        const binary_relation_detector_trainer& trainer =  checked_cast<binary_relation_detector_trainer>(trainer_);
        try
        {
            return (mitie_binary_relation_detector*)allocate<binary_relation_detector>(trainer.train());
        }
        catch(...)
        {
            return NULL;
        }
    }

// ----------------------------------------------------------------------------------------

}

