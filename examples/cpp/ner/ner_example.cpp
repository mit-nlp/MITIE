/*
    This example shows how to use the MITIE C++ API to perform named entity
    recognition. 
*/

#include <mitie/named_entity_extractor.h>
#include <mitie/conll_tokenizer.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

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

int main(int argc, char** argv)
{
    try
    {
        if (argc != 3)
        {
            printf("You must give a MITIE ner model file as the first command line argument\n");
            printf("followed by a text file to process.\n");
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

        // Print out what kind of tags this tagger can predict.
        const std::vector<string> tagstr = ner.get_tag_name_strings();
        cout << "The tagger supports "<< tagstr.size() <<" tags:" << endl;
        for (unsigned int i = 0; i < tagstr.size(); ++i)
            cout << "   " << tagstr[i] << endl;


        // Before we can try out the tagger we need to load some data.
        std::vector<string> tokens = tokenize_file(argv[2]);

        std::vector<pair<unsigned long, unsigned long> > chunks;
        std::vector<unsigned long> chunk_tags;
        std::vector<double> chunk_scores;

        // Now detect all the entities in the text file we loaded and print them to the screen.
        // The output of this function is a set of "chunks" of tokens, each a named entity.
        // Additionally, if it is useful for your application a confidence score for each "chunk"
        // is available by using the predict() method.  The larger the score the more
        // confident MITIE is in the tag.
        ner.predict(tokens, chunks, chunk_tags, chunk_scores);

        // If a confidence score is not necessary for your application you can detect entities
        // using the operator() method as shown in the following line.
        //ner(tokens, chunks, chunk_tags);

        cout << "\nNumber of named entities detected: " << chunks.size() << endl;
        for (unsigned int i = 0; i < chunks.size(); ++i)
        {
            cout << "   Tag " << chunk_tags[i] << ": ";
            cout << "Score: " << fixed << setprecision(3) << chunk_scores[i] << ": ";
            cout << tagstr[chunk_tags[i]] << ": ";
            // chunks[i] defines a half open range in tokens that contains the entity.
            for (unsigned long j = chunks[i].first; j < chunks[i].second; ++j)
                cout << tokens[j] << " ";
            cout << endl;
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

