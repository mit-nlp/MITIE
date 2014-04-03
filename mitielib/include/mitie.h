// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis.king@ll.mit.edu)
#ifndef MITLL_MITIe_H__
#define MITLL_MITIe_H__

#if defined(_WIN32)
#define MITIE_EXPORT __declspec(dllexport)
#else
#define MITIE_EXPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    /*!
        MITIE RESOURCE MANAGEMENT POLICY
            Unless explicitly noted, you do NOT need to call mitie_free() on the pointers
            returned from MITIE API calls.  That is, if it is the caller's responsibility
            to free an object created by a MITIE API call then the documentation for that
            routine will explicitly say the caller needs to free the object.

            Moreover, all resources allocated by MITIE should always be freed by calling
            mitie_free().  So never call free() on a MITIE object.
    !*/

// ----------------------------------------------------------------------------------------

    typedef struct mitie_named_entity_extractor  mitie_named_entity_extractor;
    typedef struct mitie_named_entity_detections mitie_named_entity_detections;

    MITIE_EXPORT void mitie_free (
        void* object 
    );
    /*!
        requires
            - object is either NULL or a pointer to an object from the MITIE API.
        ensures
            - Frees the resources associated with any MITIE object.
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_EXPORT char* mitie_load_entire_file (
        const char* filename
    );
    /*!
        requires
            - filename == a valid pointer to a NULL terminated C string
        ensures
            - Reads in the entire contents of the file with the given name and returns it
              as a NULL terminated C string.
            - If the file can't be loaded or read then this function returns NULL.
            - It is the responsibility of the caller to free the returned string.  You free
              it by calling mitie_free() on the pointer to the string.
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_EXPORT char** mitie_tokenize (
        const char* text
    );
    /*!
        requires
            - text == a valid pointer to a NULL terminated C string
        ensures
            - returns an array that contains a tokenized copy of the input text.  
            - The returned array is an array of pointers to NULL terminated C strings.  The
              array itself is terminated with a NULL.  So for example, if text was "some text" 
              then the returned array, TOK, would contain:
                - TOK[0] == "some"
                - TOK[1] == "text"
                - TOK[2] == NULL
            - It is the responsibility of the caller to free the returned array.  You free
              it by calling mitie_free() once on the entire array.  So to use the above
              nomenclature, you call mitie_free(TOK).  DO NOT CALL FREE ON ELEMENTS OF TOK.
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_EXPORT mitie_named_entity_extractor* mitie_load_named_entity_extractor (
        const char* filename
    );
    /*!
        requires
            - filename == a valid pointer to a NULL terminated C string
        ensures
            - The returned object MUST BE FREED by a call to mitie_free().
            - If the object can't be created then this function returns NULL
    !*/

    MITIE_EXPORT unsigned long mitie_get_num_possible_ner_tags (
        const mitie_named_entity_extractor* ner
    );
    /*!
        requires
            - ner != NULL
        ensures
            - A named entity extractor tags each entity with a tag.  This function returns
              the number of different tags which can be produced by the given named entity
              extractor.  Moreover, each tag is uniquely identified by a numeric ID which
              is just the index of the tag.  For example, if there are 4 possible tags then
              the numeric IDs are just 0, 1, 2, and 3.  
    !*/

    MITIE_EXPORT const char* mitie_get_named_entity_tagstr (
        const mitie_named_entity_extractor* ner,
        unsigned long idx
    );
    /*!
        requires
            - ner != NULL
            - idx < mitie_get_num_possible_ner_tags(ner)
        ensures
            - Each named entity tag, in addition to having a numeric ID which uniquely
              identifies it, has a text string name.  For example, if a named entity tag
              logically identifies a person then the tag string might be "PERSON". 
            - This function takes a tag ID number and returns the tag string for that tag.
            - The returned pointer is valid until mitie_free(ner) is called.
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_EXPORT mitie_named_entity_detections* mitie_extract_entities (
        const mitie_named_entity_extractor* ner,
        char** tokens 
    );
    /*!
        requires
            - ner != NULL
            - tokens == An array of NULL terminated C strings.  The end of the array must
              be indicated by a NULL value (i.e. exactly how mitie_tokenize() defines an
              array of tokens).  
        ensures
            - The returned object MUST BE FREED by a call to mitie_free().
            - Runs the supplied named entity extractor on the tokenized text and returns a
              set of named entity detections.
            - If the object can't be created then this function returns NULL
    !*/

    MITIE_EXPORT unsigned long mitie_ner_get_num_detections (
        const mitie_named_entity_detections* dets
    );
    /*!
        requires
            - dets != NULL
        ensures
            - returns the number of named entity detections inside the dets object.
    !*/

    MITIE_EXPORT unsigned long mitie_ner_get_detection_position (
        const mitie_named_entity_detections* dets,
        unsigned long idx
    );
    /*!
        requires
            - dets != NULL
            - idx < mitie_ner_get_num_detections(dets)
        ensures
            - This function returns the position of the idx-th named entity within the
              input text.  That is, if dets was created by calling
              mitie_extract_entities(ner, TOKENS) then the return value of
              mitie_ner_get_detection_position() is an index I such that TOKENS[I] is the
              first token in the input text that is part of the named entity.
            - The named entity detections are stored in the order they appeared in the
              input text.  That is, for all valid IDX it is true that:
                - mitie_ner_get_detection_position(dets,IDX) < mitie_ner_get_detection_position(dets,IDX+1)
    !*/

    MITIE_EXPORT unsigned long mitie_ner_get_detection_length (
        const mitie_named_entity_detections* dets,
        unsigned long idx
    );
    /*!
        requires
            - dets != NULL
            - idx < mitie_ner_get_num_detections(dets)
        ensures
            - returns the length of the idx-th named entity.  That is, this function
              returns the number of tokens from the input text which comprise the idx-th
              named entity detection.  
    !*/

    MITIE_EXPORT unsigned long mitie_ner_get_detection_tag (
        const mitie_named_entity_detections* dets,
        unsigned long idx
    );
    /*!
        requires
            - dets != NULL
            - idx < mitie_ner_get_num_detections(dets)
        ensures
            - returns a numeric value that identifies the type of the idx-th named entity.
    !*/

    MITIE_EXPORT const char* mitie_ner_get_detection_tagstr (
        const mitie_named_entity_detections* dets,
        unsigned long idx
    );
    /*!
        requires
            - dets != NULL
            - idx < mitie_ner_get_num_detections(dets)
        ensures
            - returns a NULL terminated C string that identifies the type of the idx-th
              named entity. 
            - The returned pointer is valid until mitie_free(dets) is called.
    !*/

// ----------------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // MITLL_MITIe_H__

