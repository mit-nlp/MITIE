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

        THREAD SAFETY
            Unless otherwise specified, you must not touch the MITIE objects returned from
            this API from multiple threads without serializing access to them via a mutex
            or some other kind of synchronization that prevents concurrent accesses.
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
            - If something prevents this function from succeeding then a NULL is returned.
    !*/

    MITIE_EXPORT char** mitie_tokenize_file (
        const char* filename
    );
    /*!
        requires
            - filename == a valid pointer to a NULL terminated C string
        ensures
            - This function is identical to calling mitie_tokenize(mitie_load_entire_file(filename));
              (except that there is no memory leak)
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_EXPORT mitie_named_entity_extractor* mitie_load_named_entity_extractor (
        const char* filename
    );
    /*!
        requires
            - filename == a valid pointer to a NULL terminated C string
        ensures
            - Reads a saved MITIE named entity extractor from disk and returns a pointer to
              the entity extractor object.
            - The returned object MUST BE FREED by a call to mitie_free().
            - If the object can't be created then this function returns NULL.
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
            - The entities detected by MITIE will never overlap.  That is, adjacent
              entities never contain the same tokens as each other.
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

    typedef struct mitie_binary_relation_detector mitie_binary_relation_detector;
    typedef struct mitie_binary_relation mitie_binary_relation;

    MITIE_EXPORT mitie_binary_relation_detector* mitie_load_binary_relation_detector (
        const char* filename
    );
    /*!
        requires
            - filename == a valid pointer to a NULL terminated C string
        ensures
            - Reads a saved MITIE binary relation detector object from disk and returns a
              pointer to the relation detector.
            - The returned object MUST BE FREED by a call to mitie_free().
            - If the object can't be created then this function returns NULL.
    !*/

    MITIE_EXPORT const char* mitie_binary_relation_detector_name_string (
        const mitie_binary_relation_detector* detector
    );
    /*!
        requires
            - detector != NULL
        ensures
            - returns a null terminated C string that identifies which relation type this
              detector is designed to detect.
            - The returned pointer is valid until mitie_free(detector) is called.
    !*/

    MITIE_EXPORT int mitie_entities_overlap (
        unsigned long arg1_start,
        unsigned long arg1_length,
        unsigned long arg2_start,
        unsigned long arg2_length
    );
    /*!
        ensures
            - returns 1 if the given entity ranges overlap and 0 otherwise.  That is, we
              interpret the arguments as defining two ranges.  One from arg1_start to
              arg1_start+length-1 inclusive and the other from arg2_start to
              arg2_start+arg2_length-1 inclusive.  This function checks if these ranges
              overlap and returns 1 if they do.
    !*/

    MITIE_EXPORT mitie_binary_relation* mitie_extract_binary_relation (
        const mitie_named_entity_extractor* ner,
        char** tokens,
        unsigned long arg1_start,
        unsigned long arg1_length,
        unsigned long arg2_start,
        unsigned long arg2_length
    );
    /*!
        requires
            - ner != NULL
            - tokens == An array of NULL terminated C strings.  The end of the array must
              be indicated by a NULL value (i.e. exactly how mitie_tokenize() defines an
              array of tokens).  
            - arg1_length > 0
            - arg2_length > 0
            - The arg indices reference valid elements of the tokens array.  That is,
              the following expressions evaluate to valid C-strings:
                - tokens[arg1_start]
                - tokens[arg1_start+arg1_length-1]
                - tokens[arg2_start]
                - tokens[arg2_start+arg1_length-1]
            - mitie_entities_overlap(arg1_start,arg1_length,arg2_start,arg2_length) == 0
        ensures
            - This function converts a raw relation mention pair into an object that you
              can feed into a binary relation detector for classification.  In particular,
              you can pass the output of this function to mitie_classify_binary_relation()
              and it will tell you if the pair of relations indicated by
              arg1_start/arg1_length and arg2_start/arg2_length are an instance of a valid
              binary relation within tokens.
            - Each binary relation is a relation between two entities.  The arg index
              ranges indicate which part of tokens comprises each of the two entities.  For
              example, if you have the "PERSON born_in PLACE" relation then the arg1 index
              range indicates the location of the PERSON entity and the arg2 indices
              indicate the PLACE entity.  In particular, the PERSON entity would be
              composed of the tokens tokens[arg1_start] through
              tokens[arg1_start+arg1_length-1] and similarly for the PLACE entity.
            - The returned object MUST BE FREED by a call to mitie_free().
            - returns NULL if the object could not be created.
    !*/

    MITIE_EXPORT int mitie_classify_binary_relation (
        const mitie_binary_relation_detector* detector,
        const mitie_binary_relation* relation,
        double* score
    );
    /*!
        requires
            - detector != NULL
            - relation != NULL
            - score != NULL
        ensures
            - returns 0 upon success and a non-zero value on failure.  Failure happens if
              the detector is incompatible with the ner object used to extract the
              relation.
            - if (this function returns 0) then
                - *score == the confidence that the given relation is an instance of the
                  type of binary relation identified by the given detector.  In particular,
                  if *score > 0 then the detector is predicting that the relation is a
                  valid instance of the relation and if *score <= 0 then it is predicting
                  that it is NOT a valid instance.  Moreover, the larger *score the more
                  confident it is that the relation is a valid relation.
    !*/

// ----------------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // MITLL_MITIe_H__

