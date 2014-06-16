
#include <mitie/named_entity_extractor.h>
#include <mitie/conll_tokenizer.h>
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;
using namespace mitie;

// ----------------------------------------------------------------------------------------

vector<string> tokenize_file (
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
    vector<string> tokens;
    string token;
    // Read the tokens out of the file one at a time and store into tokens.
    while(tok(token))
        tokens.push_back(token);

    return tokens;
}

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("You must give a MITIE ner model file as the first command line argument\n");
        printf("followed by a text file to process.\n");
        return EXIT_FAILURE;
    }

    // Load MITIE's named entity extractor from disk.
    ifstream fin(argv[1], ios::binary);
    named_entity_extractor ner;
    deserialize(ner, fin);

    // Print out what kind of tags this tagger can predict.
    const vector<string> tagstr = ner.get_tag_name_strings();
    cout << "The tagger supports "<< tagstr.size() <<" tags:" << endl;
    for (unsigned int i = 0; i < tagstr.size(); ++i)
        cout << "   " << tagstr[i] << endl;


    // Before we can try out the tagger we need to load some data.
    vector<string> tokens = tokenize_file(argv[2]);

    vector<pair<unsigned long, unsigned long> > chunks;
    vector<unsigned long> chunk_tags;
    // Now detect all the entities in the text file we loaded and print them to the screen.
    // The output of this function is a set of "chunks" of tokens, each a named entity.
    ner(tokens, chunks, chunk_tags);
    cout << "\nNumber of named entities detected: " << chunks.size() << endl;
    for (unsigned int i = 0; i < chunks.size(); ++i)
    {
        cout << "   Tag " << chunk_tags[i] << ":" << tagstr[chunk_tags[i]] << ": ";
        // chunks[i] defines a half open range in tokens that contains the entity.
        for (unsigned long j = chunks[i].first; j < chunks[i].second; ++j)
            cout << tokens[j] << " ";
        cout << endl;
    }

    return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------------------

