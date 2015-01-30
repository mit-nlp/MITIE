/*
    This example shows how to use the MITIE C API to perform named entity
    recognition. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <mitie.h>

// ----------------------------------------------------------------------------------------

void print_entity (char** tokens, const mitie_named_entity_detections* dets, unsigned long i);

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    mitie_named_entity_extractor* ner = 0;
    mitie_named_entity_detections* dets = 0;
    unsigned long num_tags = 0;
    unsigned long num_dets = 0;
    unsigned long i = 0;
    char** tokens = 0;
    int return_code = EXIT_FAILURE;

    if (argc != 3)
    {
        printf("You must give a MITIE ner model file as the first command line argument\n");
        printf("followed by a text file to process. For example:\n");
        printf("./ner_example MITIE-models/english/ner_model.dat sample_text.txt\n");
        return EXIT_FAILURE;
    }

    ner = mitie_load_named_entity_extractor(argv[1]);
    if (!ner)
    {
        printf("Unable to load model file\n");
        goto cleanup;
    }

    // Print out what kind of tags this tagger can predict.
    num_tags = mitie_get_num_possible_ner_tags(ner);
    printf("The tagger supports %lu tags:\n", num_tags);
    for (i = 0; i < num_tags; ++i)
        printf("   %s\n", mitie_get_named_entity_tagstr(ner, i));

    // Now get some text and turn it into an array of tokens.
    tokens = mitie_tokenize_file(argv[2]);
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
    for (i = 0; i < num_dets; ++i)
    {
        print_entity(tokens, dets, i);
    }




    return_code = EXIT_SUCCESS;
cleanup:
    mitie_free(tokens);
    mitie_free(dets);
    mitie_free(ner);

    return return_code;
}

// ----------------------------------------------------------------------------------------

void print_entity (
    char** tokens,
    const mitie_named_entity_detections* dets,
    unsigned long i
)
{
    unsigned long pos, len;

    pos = mitie_ner_get_detection_position(dets, i);
    len = mitie_ner_get_detection_length(dets, i);
    // Print the label and score for each named entity and also the text of the named entity
    // itself.  The larger the score the more confident MITIE is in the tag.
    printf("   Tag %lu: Score: %0.3f: %s: ", mitie_ner_get_detection_tag(dets,i),
                                             mitie_ner_get_detection_score(dets,i),
                                             mitie_ner_get_detection_tagstr(dets,i));
    while(len > 0)
    {
        printf("%s ", tokens[pos++]);
        --len;
    }
    printf("\n");
}

// ----------------------------------------------------------------------------------------

