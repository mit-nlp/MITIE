// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#include <mitie.h>

#include <mitie/ner_trainer.h>
#include <map>
#include <iostream>
#include <dlib/cmd_line_parser.h>
#include <mitie/conll_parser.h>
#include <mitie/total_word_feature_extractor.h>



using namespace dlib;
using namespace std;
using namespace mitie;

// ----------------------------------------------------------------------------------------

void train(const command_line_parser& parser);
void test(const command_line_parser& parser);
void tag_conll_file(const command_line_parser& parser);

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    try
    {
        command_line_parser parser;
        parser.add_option("h", "Display this help information.");
        parser.add_option("train", "train named_entity_extractor on CoNLL data.");
        parser.add_option("test", "test named_entity_extractor on CoNLL data.");
        parser.add_option("threads", "Use <arg> threads when doing training (default: 4).",1);
        parser.add_option("tag-conll-file", "Read in a CoNLL annotation file and output a copy that is tagged with a MITIE NER model.");

        parser.parse(argc,argv);
        parser.check_option_arg_range("threads", 1, 1000);
        parser.check_sub_option("train", "threads");

        if (parser.option("h"))
        {
            cout << "Usage: ner [options]\n";
            parser.print_options(); 
            return 0;
        }

        if (parser.option("tag-conll-file"))
        {
            tag_conll_file(parser);
            return 0;
        }

        if (parser.option("train"))
        {
            train(parser);
            return 0;
        }

        if (parser.option("test"))
        {
            test(parser);
            return 0;
        }

        return 0;
    }
    catch (std::exception& e)
    {
        cout << e.what() << endl;
        return 1;
    }
}

// ----------------------------------------------------------------------------------------

void train(const command_line_parser& parser)
{
    if (parser.number_of_arguments() != 2)
    {
        throw dlib::error("You must give a CoNLL formatted data file followed by a saved total_word_feature_extractor object.");
    }

    const unsigned long num_threads = get_option(parser, "threads", 4);

    std::vector<std::vector<std::string> > sentences;
    std::vector<std::vector<std::pair<unsigned long, unsigned long> > > chunks;
    std::vector<std::vector<std::string> > chunk_labels;

    parse_conll_data(parser[0], sentences, chunks, chunk_labels);
    ner_trainer trainer(parser[1]);
    trainer.set_num_threads(num_threads);
    trainer.add(sentences, chunks, chunk_labels);
    named_entity_extractor ner = trainer.train();
    cout << "Saving learned named_entity_extractor to ner_model.dat" << endl;
    serialize("ner_model.dat") << "mitie::named_entity_extractor" << ner;
}

// ----------------------------------------------------------------------------------------

void test(const command_line_parser& parser)
{
    if (parser.number_of_arguments() != 2)
    {
        throw dlib::error("You must give a CoNLL formatted data file followed by a saved named_entity_extractor object.");
    }

    string classname;
    named_entity_extractor ner;
    deserialize(parser[1]) >> classname >> ner;

    std::vector<std::vector<std::string> > sentences;
    std::vector<std::vector<std::pair<unsigned long, unsigned long> > > chunks;
    std::vector<std::vector<std::string> > chunk_labels;
    parse_conll_data(parser[0], sentences, chunks, chunk_labels);

    cout << evaluate_named_entity_recognizer(ner, sentences, chunks, chunk_labels) << endl;
}

// ----------------------------------------------------------------------------------------

void tag_conll_file(const command_line_parser& parser)
{
    if (parser.number_of_arguments() != 2)
    {
        throw dlib::error("You must give a CoNLL formatted data file followed by a saved named_entity_extractor object.");
    }
    string classname;
    named_entity_extractor ner;
    deserialize(parser[1]) >> classname >> ner;


    std::vector<labeled_sentence> conll_data = parse_conll_data (parser[0]);
    std::vector<std::vector<std::string> > tokens;
    std::vector<std::vector<BIO_label> > labels;
    separate_labels_from_tokens(conll_data, tokens, labels);

    const std::vector<std::string> tags = ner.get_tag_name_strings();

    std::vector<std::pair<unsigned long, unsigned long> > ranges;
    std::vector<unsigned long> predicted_labels;
    for (unsigned long i = 0; i < tokens.size(); ++i)
    {
        ner(tokens[i], ranges, predicted_labels);
        labels[i].assign(labels[i].size(),O);
        for (unsigned long j = 0; j < ranges.size(); ++j)
        {
            for (unsigned long k = ranges[j].first; k < ranges[j].second; ++k)
            {
                if (j > 0 && ranges[j].first == ranges[j-1].second && predicted_labels[j] == predicted_labels[j-1])
                {
                    if (tags[predicted_labels[j]] == "PERSON")
                        labels[i][k] = B_PER; 
                    else if (tags[predicted_labels[j]] == "ORGANIZATION")
                        labels[i][k] = B_ORG; 
                    else if (tags[predicted_labels[j]] == "LOCATION")
                        labels[i][k] = B_LOC; 
                    else if (tags[predicted_labels[j]] == "MISC")
                        labels[i][k] = B_MISC; 
                }
                else
                {
                    if (tags[predicted_labels[j]] == "PERSON")
                        labels[i][k] = I_PER; 
                    else if (tags[predicted_labels[j]] == "ORGANIZATION")
                        labels[i][k] = I_ORG; 
                    else if (tags[predicted_labels[j]] == "LOCATION")
                        labels[i][k] = I_LOC; 
                    else if (tags[predicted_labels[j]] == "MISC")
                        labels[i][k] = I_MISC; 
                }
            }
        }
    }

    print_conll_data(conll_data, labels);
}

// ----------------------------------------------------------------------------------------

