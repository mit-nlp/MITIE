/*
    This example shows how to use the MITIE C++ API to perform named entity
    recognition and also how to run a binary relation detector on top of the
    named entity recognition outputs.
*/

#include <mitie/binary_relation_detector.h>
#include <mitie/named_entity_extractor.h>
#include <mitie/conll_tokenizer.h>
#include <iostream>
#include <fstream>
#include <cstdlib>


using namespace dlib;
using namespace std;
using namespace mitie;

// ----------------------------------------------------------------------------------------

std::vector<string> tokenize_file (
    const string& filename
)
{
    ifstream fin(filename.c_str());
    if (!fin)
    {
        cout << "Unable to load input text file" << endl;
        exit(EXIT_FAILURE);
    }
    // The conll_tokenizer splits the contents of an istream into a bunch of words and is
    // MITIE's default tokenization method. 
    conll_tokenizer tok(fin);
    std::vector<string> tokens;
    string token;
    // Read the tokens out of the file one at a time and store into tokens.
    while(tok(token))
        tokens.push_back(token);

    return tokens;
}

// ----------------------------------------------------------------------------------------

string cat_tokens (
    const std::vector<std::string>& tokens,
    const std::pair<unsigned long, unsigned long>& range
)
{
    // Note that the entities are identified using half open ranges.  So we loop over all
    // the elements of the supplied half open range and concatenate them.
    string temp;
    for (unsigned long i = range.first; i < range.second; ++i)
    {
        temp += tokens[i];
        if (i+1 < range.second)
            temp += " ";
    }
    return temp;
}

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    try
    {
        if (argc != 4)
        {
            cout << "To run this program you must give NER model and binary" << endl;
            cout << "relation detector files as input, as well as a text file" << endl;
            cout << "to evaluate.  For example:" << endl;
            cout << "./relation_extraction_example MITIE-models/english/ner_model.dat MITIE-models/english/binary_relations/rel_classifier_location.location.contains.svm sample_text.txt" << endl;
            return EXIT_FAILURE;
        }

        // Load MITIE's named entity extractor from disk.  Each file in the MITIE-models
        // folder begins with a string containing the name of the serialized class.  In
        // this case classname contains "mitie::named_entity_extractor".  It can be used to
        // identify what is in any particular file.  However, in this example we don't need
        // it so it is just ignored.
        string classname;
        named_entity_extractor ner;
        dlib::deserialize(argv[1]) >> classname >> ner;

        // Before we can try out the tagger we need to load some data.
        std::vector<string> tokens = tokenize_file(argv[3]);

        std::vector<pair<unsigned long, unsigned long> > chunks;
        std::vector<unsigned long> chunk_tags;
        // Now detect all the entities in the text file we loaded and print them to the screen.
        // The output of this function is a set of "chunks" of tokens, each a named entity.
        ner(tokens, chunks, chunk_tags);
        cout << "\nNumber of named entities detected: " << chunks.size() << endl;




        cout << "now look for binary relations" << endl;
        binary_relation_detector bd;
        // But first, load a binary relation detector from disk.
        deserialize(argv[2]) >> classname >> bd;
        // Print the type of relations this detector looks for.
        cout << "relation type: "<< bd.relation_type << endl;
        // Now let's scan along the entities and ask the relation detector which pairs of
        // entities are instances of the type of relation we are looking for.  
        for (unsigned long i = 0; i+1 < chunks.size(); ++i)
        {
            // Calling this function runs the relation detector and returns a score value.
            // If the score is > 0 then the detector is indicating that this relation
            // mention is an example of the type of relation this detector is looking for.
            // Moreover, the larger the score the more confident the detector is that this
            // is a correct relation detection.  Here we print anything with a score > 0.
            if (bd(extract_binary_relation(tokens, chunks[i], chunks[i+1], ner.get_total_word_feature_extractor())) > 0)
                cout << cat_tokens(tokens, chunks[i]) << "   #   " << cat_tokens(tokens, chunks[i+1]) << endl;

            // Relations have an ordering to their arguments.  So even if the above
            // relation check failed we still might have a valid relation if we try
            // swapping the two arguments.  So that's what we do here.
            if (bd(extract_binary_relation(tokens, chunks[i+1], chunks[i], ner.get_total_word_feature_extractor())) > 0)
                cout << cat_tokens(tokens, chunks[i+1]) << "   #   " << cat_tokens(tokens, chunks[i]) << endl;


            // Finally, note that when you train a relation detector it uses features
            // derived from a MITIE NER object as part of its processing (specifically, the
            // total word feature extractor part).  Because of this, every relation
            // detector depends on a NER object and, moreover, it is important that you use
            // the same NER object which was used during training when you run the relation
            // detector.  If you don't use the same NER object instance the
            // binary_relation_detector with throw an exception.
        }

        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        cout << e.what() << endl;
        return EXIT_FAILURE;
    }
}

// ----------------------------------------------------------------------------------------

