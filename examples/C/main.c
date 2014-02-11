
#include <stdio.h>
#include <stdlib.h>
#include <mitie.h>

int main(int argc, char** argv)
{
    mitie_named_entity_extractor* ner = 0;
    mitie_named_entity_detections* dets = 0;
    unsigned long num_tags = 0;
    unsigned long num_dets = 0;
    unsigned long i = 0;
    unsigned long begin, len;
    char* text_data = 0;

    if (argc != 3)
    {
        printf("You must give a MITIE ner model file as the first command line argument\n");
        printf("followed by a text file to process.\n");
        return EXIT_FAILURE;
    }

    ner = mitie_load_named_entity_extractor(argv[1]);
    if (!ner)
    {
        printf("Unable to load model file\n");
        return EXIT_FAILURE;
    }

    // Print out what kind of tags this tagger can predict.
    num_tags = mitie_get_num_possible_ner_tags(ner);
    printf("The tagger supports %lu tags:\n", num_tags);
    for (i = 0; i < num_tags; ++i)
        printf("   %s\n", mitie_get_named_entity_tagstr(ner, i));

    text_data = mitie_load_entire_file(argv[2]);
    if (!text_data)
    {
        printf("Unable to load input text file.\n");
        mitie_free(ner);
        return EXIT_FAILURE;
    }

    // Now detect all the entities in the text file we loaded and print them to the screen.
    dets = mitie_extract_entities(ner, text_data);
    num_dets = mitie_ner_get_num_detections(dets);
    printf("\nNumber of named entities detected: %lu\n", num_dets);
    for (i = 0; i < num_dets; ++i)
    {
        begin = mitie_ner_get_detection_position(dets, i);
        len = mitie_ner_get_detection_length(dets, i);
        // Print the label for each named entity and also the text of the named entity
        // itself.
        printf("   Tag %lu:%s: %.*s\n", 
            mitie_ner_get_detection_tag(dets,i),
            mitie_ner_get_detection_tagstr(dets,i),
            len, text_data+begin);
    }


    free(text_data);
    mitie_free(dets);
    mitie_free(ner);

    return EXIT_SUCCESS;
}

