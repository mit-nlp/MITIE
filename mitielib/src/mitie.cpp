// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis.king@ll.mit.edu)
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
        MITIE_RAW_MEMORY
    };

    const int min_alignment = 16;
    int memory_block_type (void* ptr)
    {
        return *((int*)((char*)ptr-min_alignment));
    }

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
    T* allocate(mitie_object_type type)
    {
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
            std::vector<char> buf;
            dlib::vectorstream out(buf);
            out << fin.rdbuf();

            char* final_buf = (char*)allocate_bytes(buf.size()+1);
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

    char** mitie_tokenize (
        const char* text
    )
    {
        try
        {
            // first tokenize the text
            istringstream sin(text);
            conll_tokenizer tok(sin);
            std::vector<std::string> words;
            string word;
            size_t data_size = 0;
            while(tok(word))
            {
                words.push_back(word);
                data_size += word.size() + 1; // +1 for the NULL terminator
            }

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
        catch (...)
        {
            return 0;
        }
    }

// ----------------------------------------------------------------------------------------

    struct mitie_named_entity_detections
    {
        std::vector<std::pair<unsigned long, unsigned long> > ranges;
        std::vector<unsigned long> predicted_labels;
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
            impl = allocate<named_entity_extractor>(MITIE_NAMED_ENTITY_EXTRACTOR);
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
        const named_entity_extractor* ner = (named_entity_extractor*)ner_;
        assert(ner != NULL);
        return ner->get_tag_name_strings().size();
    }

    const char* mitie_get_named_entity_tagstr (
        const mitie_named_entity_extractor* ner_,
        unsigned long idx
    )
    {
        const named_entity_extractor* ner = (named_entity_extractor*)ner_;
        assert(ner != NULL);
        assert(idx < mitie_get_num_possible_ner_tags(ner_));
        return ner->get_tag_name_strings()[idx].c_str();
    }

// ----------------------------------------------------------------------------------------

    mitie_named_entity_detections* mitie_extract_entities (
        const mitie_named_entity_extractor* ner_,
        char** tokens 
    )
    {
        const named_entity_extractor* ner = (named_entity_extractor*)ner_;

        assert(ner != NULL);
        assert(tokens != NULL);

        mitie_named_entity_detections* impl = 0;

        try
        {
            impl = allocate<mitie_named_entity_detections>(MITIE_NAMED_ENTITY_DETECTIONS);

            std::vector<std::string> words;
            for (unsigned long i = 0; tokens[i]; ++i)
                words.push_back(tokens[i]);

            (*ner)(words, impl->ranges, impl->predicted_labels);
            impl->tags = ner->get_tag_name_strings();
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

}

