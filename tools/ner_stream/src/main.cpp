// Copyright (C) 2014 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)


#include <iostream>
#include <fstream>
#include <sstream>
#include <mitie/named_entity_extractor.h>
#include <mitie/conll_tokenizer.h>
#include <dlib/time_this.h>
#include <dlib/cmd_line_parser.h>
#include <dlib/serialize.h>

using namespace std;
using namespace dlib;
using namespace mitie;

std::vector<std::string> tokenize (
    const std::string& line
)
{
    istringstream sin(line);
    conll_tokenizer tok(sin);
    std::vector<std::string> words;
    string word;
    while(tok(word))
        words.push_back(word);

    return words;
}

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    try
    {
        command_line_parser parser;
        parser.add_option("h", "Display this help information.");
        parser.add_option("o", "Output the results to a file named <arg>.  The contents will be saved "
            "using dlib's serialization format. ",1);

        parser.parse(argc, argv);
        const char* one_time_ops[] = {"o", "h"};
        parser.check_one_time_options(one_time_ops);
        if (parser.option("h"))
        {
            cout << "Usage: cat input_file.txt | ner_stream <options> MITIE-models/english/ner_model.dat" << endl;
            parser.print_options();
            return 0;
        }
        if (parser.number_of_arguments() != 1)
        {
            cerr << "Error, you must give a MITIE ner model file as the first argument to this program!" << endl;
            return 1;
        }


        string classname;
        named_entity_extractor ner;
        cerr << "Loading MITIE NER model file..." << endl;
        // All the models saved in the MITIE-models folder contain a serialized string that
        // indicates the name of the class saved in the file (e.g. "mitie::named_entity_extractor")
        // and then the instance of that class.  So here we read those two things from the
        // given model file.
        TIME_THIS_TO(deserialize(parser[0]) >> classname >> ner, cerr);

        cerr << "Now running NER tool..." << endl;

        if (parser.option("o"))
        {
            const string filename = parser.option("o").argument();
            cerr << "saving results to file " << filename << endl;
            ofstream fout(filename.c_str(), ios::binary);
            std::string line;
            while (getline(cin, line))
            {
                std::vector<std::pair<unsigned long, unsigned long> > chunks;
                std::vector<unsigned long> chunk_tags;

                ner(tokenize(line), chunks, chunk_tags);
                dlib::serialize(chunks, fout);
                dlib::serialize(chunk_tags, fout);
            }
        }
        else
        {
            const std::vector<std::string> tags = ner.get_tag_name_strings();

            string line;
            while (getline(cin, line))
            {
                std::vector<std::string> tokens = tokenize(line);
                std::vector<std::pair<unsigned long, unsigned long> > chunks;
                std::vector<unsigned long> chunk_tags;
                ner(tokens, chunks, chunk_tags);

                // Push an empty chunk onto the end so we can avoid complicated bounds checking in
                // the following loop.
                chunks.push_back(make_pair(tokens.size()+1, tokens.size()+1));

                unsigned long next = 0;
                for (unsigned long i = 0; i <= tokens.size(); ++i)
                {
                    if (i == chunks[next].second)
                    {
                        cout << "] ";
                        ++next;
                    }
                    if (i == tokens.size())
                        break;

                    if (i == chunks[next].first)
                        cout << "[" << tags[chunk_tags[next]] << " ";
                    cout << tokens[i];
                    if (i+1 != chunks[next].second)
                        cout << " ";
                }
                cout << endl;
            }
        }

    }
    catch (std::exception& e)
    {
        cout << e.what() << endl;
        return 1;
    }
}

