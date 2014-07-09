/*
    This example shows how to use the MITIE C API to perform named entity
    recognition and also how to run a binary relation detector on top of the
    named entity recognition outputs.
*/
#include <stdio.h>
#include <stdlib.h>
#include <mitie.h>

// ----------------------------------------------------------------------------------------

void print_relation (char** tokens, const mitie_named_entity_detections* dets,
                     unsigned long idx1, unsigned long idx2);

int detect_relation (const mitie_binary_relation_detector* relation_detector,
    const mitie_named_entity_extractor* ner, char** tokens, const
    mitie_named_entity_detections* dets, unsigned long idx1, unsigned long
    idx2, double* score);

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    mitie_named_entity_extractor* ner = 0;
    mitie_named_entity_detections* dets = 0;
    mitie_binary_relation_detector* relation_detector = 0;
    unsigned long num_dets = 0;
    unsigned long i = 0;
    char** tokens = 0;
    int return_code = EXIT_FAILURE;
    double score = 0;

    if (argc != 4)
    {
        printf("To run this program you must give NER model and binary\n");
        printf("relation detector files as input, as well as a text file\n");
        printf("to evaluate.  For example:\n");
        printf("./relation_extraction_example MITIE-models/english/ner_model.dat MITIE-models/english/binary_relations/rel_classifier_location.location.contains.svm sample_text.txt\n");
        goto cleanup;
    }

    ner = mitie_load_named_entity_extractor(argv[1]);
    if (!ner)
    {
        printf("Unable to load model file\n");
        goto cleanup;
    }

    // Now get some text and turn it into an array of tokens.
    tokens = mitie_tokenize_file(argv[3]);
    if (!tokens)
    {
        printf("Unable to tokenize file.\n");
        goto cleanup;
    }

    // Now detect all the entities in the text file we loaded and print them to the screen.
    dets = mitie_extract_entities(ner, tokens);
    if (!dets) 
    { 
        printf("Unable to allocate list of MITIE entities.");
        goto cleanup; 
    }

    num_dets = mitie_ner_get_num_detections(dets);
    printf("\nNumber of named entities detected: %lu\n", num_dets);



    printf("\nNow look for binary relations.\n");
    relation_detector = mitie_load_binary_relation_detector(argv[2]);
    if (!relation_detector) 
    { 
        printf("Unable to load MITIE binary relation detector.");
        goto cleanup; 
    }
    // Print the type of relations this detector looks for.
    printf("relation type: %s\n", mitie_binary_relation_detector_name_string(relation_detector));
    // Now let's scan along the entities and ask the relation detector which pairs of
    // entities are instances of the type of relation we are looking for.  
    for (i = 0; i+1 < num_dets; ++i)
    {
        if (detect_relation(relation_detector, ner, tokens, dets, i, i+1, &score))
            goto cleanup; 
        if (score > 0)
            print_relation(tokens, dets, i, i+1);

        // Relations have an ordering to their arguments.  So even if the above
        // relation check failed we still might have a valid relation if we try
        // swapping the two arguments.  So that's what we do here.
        if (detect_relation(relation_detector, ner, tokens, dets, i+1, i, &score))
            goto cleanup; 
        if (score > 0)
            print_relation(tokens, dets, i+1, i);
    }




    return_code = EXIT_SUCCESS;
cleanup:
    mitie_free(tokens);
    mitie_free(dets);
    mitie_free(ner);
    mitie_free(relation_detector);

    return return_code;
}

// ----------------------------------------------------------------------------------------

void print_relation (
    char** tokens,
    const mitie_named_entity_detections* dets,
    unsigned long idx1,
    unsigned long idx2
)
{
    unsigned long pos, len;

    // print the first argument to the relation
    pos = mitie_ner_get_detection_position(dets, idx1);
    len = mitie_ner_get_detection_length(dets, idx1);
    while(len > 0)
    {
        printf("%s ", tokens[pos++]);
        --len;
    }

    printf("   #   ");

    // Now print the second argument to the relation
    pos = mitie_ner_get_detection_position(dets, idx2);
    len = mitie_ner_get_detection_length(dets, idx2);
    while(len > 0)
    {
        printf("%s ", tokens[pos++]);
        --len;
    }
    printf("\n");
}

// ----------------------------------------------------------------------------------------

int detect_relation (
    const mitie_binary_relation_detector* relation_detector,
    const mitie_named_entity_extractor* ner,
    char** tokens,
    const mitie_named_entity_detections* dets,
    unsigned long idx1,
    unsigned long idx2,
    double* score
)
{
    mitie_binary_relation* relation = 0;
    // The relation detection process in MITIE has two steps.  First you extract a set of
    // "features" that describe a particular relation mention.  Then you call
    // mitie_classify_binary_relation() on those features and see if it is an instance of a
    // particular kind of relation.  The reason we have this two step process is because,
    // in many applications, you will have a large set of relation detectors you need to
    // evaluate for each possible relation instance and it is more efficient to perform the
    // feature extraction once and then reuse the results for multiple calls to
    // mitie_classify_binary_relation().  However, in this case, we are simply running one
    // type of relation detector.
    relation = mitie_extract_binary_relation(ner, tokens, 
                                             mitie_ner_get_detection_position(dets,idx1),
                                             mitie_ner_get_detection_length(dets,idx1),
                                             mitie_ner_get_detection_position(dets,idx2),
                                             mitie_ner_get_detection_length(dets,idx2));
    if (!relation) 
    { 
        printf("Unable to allocate binary relation object.");
        return 1;
    }
    // Calling this function runs the relation detector on the relation and stores the
    // output into score.  If score is > 0 then the detector is indicating that this
    // relation mention is an example of the type of relation this detector is looking for.
    // Moreover, the larger score the more confident the detector is that it is that this
    // is a correct relation detection. 
    if (mitie_classify_binary_relation(relation_detector, relation, score))
    {
        // When you train a relation detector it uses features derived from a MITIE NER
        // object as part of its processing.  This is also evident in the interface of
        // mitie_extract_binary_relation() which requires a NER object to perform feature
        // extraction.  Because of this, every relation detector depends on a NER object
        // and, moreover, it is important that you use the same NER object which was used
        // during training when you run the relation detector.  If you don't use the same
        // NER object instance the mitie_classify_binary_relation() routine will return an
        // error.
        printf("An incompatible NER object was used with the relation detector.");
        mitie_free(relation);
        return 1;
    }
    mitie_free(relation);
    return 0;
}

