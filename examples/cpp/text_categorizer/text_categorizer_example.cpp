/*
    This example shows how to use the MITIE C++ API to perform text categorizer.
*/

#include <mitie/conll_tokenizer.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <mitie/text_categorizer.h>

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
            printf("You must give a MITIE text categorizer model file as the first command line argument\n");
            printf("followed by a text file to process.\n");
            return EXIT_FAILURE;
        }

        // Load MITIE's text categorizer from disk.  Each file in the MITIE-models
        // folder begins with a string containing the name of the serialized class.  In
        // this case classname contains "mitie::text_categorizer".  It can be used to
        // identify what is in any particular file.  However, in this example we don't need
        // it so it is just ignored.
        string classname;
        text_categorizer categorizer;
        dlib::deserialize(argv[1]) >> classname >> categorizer;

        // Print out what kind of labels this categorizer can predict.
        const std::vector<string> tagstr = categorizer.get_tag_name_strings();
        cout << "The categorizer supports "<< tagstr.size() <<" labels:" << endl;
        for (unsigned int i = 0; i < tagstr.size(); ++i)
            cout << "   " << tagstr[i] << endl;

        // Before we can try out the categorizer, we need to load some testing data.
        std::vector<string> tokens = tokenize_file(argv[2]);

        string text_tag;
        double text_score;

        // Now detect the label of the text file we loaded and print them to the screen.
        // The output of this function is the detected label.
        // Additionally, if it is useful for your application a confidence score for this label,
        // is available by using the predict() method.  The larger the score the more
        // confident MITIE is in the label.
        categorizer.predict(tokens, text_tag, text_score);
        cout << "The label is " << text_tag << ", with the confidence score as " << text_score << endl;

        // If a confidence score is not necessary for your application you can detect document type
        // using the operator() method as shown in the following line.
        // text_tag = categorizer(tokens);
        // cout << "The label is " << text_tag << endl;

        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        cout << e.what() << endl;
        return EXIT_FAILURE;
    }
}

// ----------------------------------------------------------------------------------------

