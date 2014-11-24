// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include <iostream>
#include <dlib/cmd_line_parser.h>
#include <mitie/gigaword_reader.h>
#include <dlib/dir_nav.h>
#include <mitie/count_min_sketch.h>
#include <mitie/group_tokenizer.h>
#include <mitie/unigram_tokenizer.h>
#include "basic_morph.h"
#include "word_vects.h"
#include "cca_morph.h"
#include "doc_vects.h"

#include <mitie/total_word_feature_extractor.h>

#include <dlib/graph_utils_threaded.h>
#include <dlib/clustering.h>

using namespace std;
using namespace dlib;
using namespace mitie;


// ----------------------------------------------------------------------------------------

template <typename tokenizer_type>
void get_top_word_counts (
    tokenizer_type& tok,
    std::map<std::string, unsigned long>& top_words,
    const unsigned long max_top_words
)
{
    count_min_sketch counts(5000000);

    // get counts on all the words first
    std::string token;
    while (tok(token))
    {
        counts.increment(token);
    }

    std::priority_queue<std::pair<long, std::string> > best_words;

    dlib::set<std::string>::kernel_1a cur_words;

    // now pass over the data again and pick out only words with high counts
    tok.reset();
    while (tok(token))
    {
        if (cur_words.is_member(token))
            continue;

        const long hits = counts.get_count(token);
        if (best_words.size() < max_top_words || hits > -best_words.top().first)
        {
            if (best_words.size() >= max_top_words)
            {
                cur_words.destroy(best_words.top().second);
                best_words.pop();
            }

            best_words.push(std::make_pair(-hits, token));
            cur_words.add(token);
        }
    }

    // now put the best words into top_words
    top_words.clear();
    while (best_words.size() != 0)
    {
        top_words[best_words.top().second] = -best_words.top().first;
        best_words.pop();
    }
}

// ----------------------------------------------------------------------------------------

void count_words(const command_line_parser& parser);
void test(const command_line_parser& parser);
void cluster_words(const command_line_parser& parser);

int main(int argc, char** argv)
{
    try
    {
        command_line_parser parser;
        parser.add_option("h","Display this help message.");

        parser.add_option("e", "Make a total_word_feature_extractor from a folder of text files.   This option is a shortcut for executing"
                               " the following options together --count-words 200000 --word-vects --basic-morph --cca-morph.");

        parser.set_group_name("Other Options");
        parser.add_option("convert-gigaword", "Take a folder of gigaword XML documents and convert them "
            "into a regular ASCII file named <arg>.",1);
        parser.add_option("count-words", "Make a file containing the top <arg> most common words and their occurrence counts.",1);
        parser.add_option("basic-morph", "Make a word morphology extractor.");
        parser.add_option("cca-morph", "Make a CCA based word morphology extractor object as well as a total word "
                                        "feature extractor.");
        parser.add_option("word-vects", "Use CCA to create distributional word vectors.");
        parser.add_option("test", "Print out the feature vectors for the word given on the command line.");
        parser.add_option("cluster-words", "Generate word clusters based on a saved total_word_feature_extractor.");

        parser.set_group_name("Document Vector Level Features");
        parser.add_option("doc-vects", "Generate CCA based word features where we assume the important thing about a "
            "word is what other words it shows up with in the same document.  For this, we take a folder of gigaword XML files "
            "as input.");
        parser.add_option("dims", "When doing --doc-vects, make the output vectors have <arg> dimensions (default: 500).",1);


        parser.parse(argc,argv);
        parser.check_option_arg_range("count-words", 1, 1000000000);
        parser.check_option_arg_range("dims", 1, 100000);
        parser.check_sub_option("doc-vects", "dims");
        parser.check_incompatible_options("e", "word-vects");
        parser.check_incompatible_options("e", "count-words");
        parser.check_incompatible_options("e", "basic-morph");
        parser.check_incompatible_options("e", "cca-morph");

        if (parser.option("h"))
        {
            cout << "Main Usage: wordrep -e <folder of text files>\n";
            parser.print_options(); 
            cout << endl;
            return 0;
        }

        if (parser.option("convert-gigaword"))
        {
            ofstream fout(parser.option("convert-gigaword").argument().c_str());
            std::vector<dlib::file> files = get_files_in_directory_tree(directory(parser[0]), match_all());
            cout << "number of gigaword files found: " << files.size() << endl;
            mitie::gigaword_reader reader(files);
            std::string data;
            while (reader(data))
                fout << data << "\n\n";

        }

        if (parser.option("e"))
        {
            count_words(parser);
            word_vects(parser);
            basic_morph(parser);
            cca_morph(parser);
            return 0;
        }

        if (parser.option("count-words"))
        {
            count_words(parser);
        }

        if (parser.option("word-vects"))
        {
            word_vects(parser);
        }

        if (parser.option("basic-morph"))
        {
            basic_morph(parser);
        }

        if (parser.option("cca-morph"))
        {
            cca_morph(parser);
        }

        if (parser.option("test"))
        {
            test(parser);
        }

        if (parser.option("cluster-words"))
        {
            cluster_words(parser);
        }

        if (parser.option("doc-vects"))
        {
            make_doc_vects(parser);
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

void cluster_words(const command_line_parser& parser)
{
    string classname;
    total_word_feature_extractor fe;
    deserialize("total_word_feature_extractor.dat") >> classname >> fe;

    cout << "words in dictionary: " << fe.get_num_words_in_dictionary() << endl;
    cout << "num features: " << fe.get_num_dimensions() << endl;
    const std::vector<std::string>& words = fe.get_words_in_dictionary();
    std::vector<matrix<float,0,1> > vects;
    vects.resize(words.size());
    for (unsigned long i = 0; i < vects.size(); ++i)
        fe.get_feature_vector(words[i], vects[i]);

    cout << "Making graph" << endl;
    std::vector<sample_pair> edges;
    find_k_nearest_neighbors_lsh(vects, cosine_distance(), hash_similar_angles_256(), 100, 4, edges);
    cout << "edges.size(): "<< edges.size() << endl;

    // change all the edge weights to use the formula from the unsupervised POS paper.
    for (unsigned long i = 0; i < edges.size(); ++i)
    {
        const double dist = edges[i].distance();
        double edge_weight;
        if (dist <= 0.1)
            edge_weight = 10;
        else 
            edge_weight = 1.0/dist;
        edges[i] = sample_pair(edges[i].index1(), edges[i].index2(), edge_weight);
    }
    remove_short_edges(edges, 3.0);
    //remove_short_edges(edges, 5.0);
    cout << "edges.size(): "<< edges.size() << endl;

    std::vector<unsigned long> labels;
    const unsigned long num_clusters = chinese_whispers(edges, labels);
    //const unsigned long num_clusters = newman_cluster(edges, labels);
    cout << "num_clusters: "<< num_clusters << endl;
    cout << "labels.size(): "<< labels.size() << endl;

    std::vector<std::vector<std::string> > groups(num_clusters);
    for (unsigned long i = 0; i < labels.size(); ++i)
    {
        groups[labels[i]].push_back(words[i]);
    }

    ofstream fout("word_clusters.txt");
    for (unsigned long i = 0; i < groups.size(); ++i)
    {
        for (unsigned long j = 0; j < groups[i].size(); ++j)
        {
            fout << groups[i][j] << " ";
        }
        fout << "\n\n********************************************************************************\n\n";
    }

    // also save a more machine readable copy of the clusters
    ofstream fout2("word_clusters.dat", ios::binary);
    serialize(groups, fout2);
}

// ----------------------------------------------------------------------------------------

void count_words(const command_line_parser& parser)
{
    const unsigned long num_top_words = get_option(parser, "count-words", 200000);

    std::vector<dlib::file> files = get_files_in_directory_tree(directory(parser[0]), match_all());
    cout << "number of raw ASCII files found: " << files.size() << endl;

    group_tokenizer<unigram_tokenizer> tok(files);
    std::map<std::string, unsigned long> words;
    get_top_word_counts(tok, words, num_top_words);

    cout << "num words: "<< words.size() << endl;
    cout << "saving word counts to top_word_counts.dat" << endl;
    ofstream fout("top_word_counts.dat", ios::binary);
    serialize(words, fout);

    // lets also save it as a .txt file
    std::vector<std::pair<unsigned long, std::string> > temp;
    for (std::map<std::string,unsigned long>::iterator i = words.begin(); i != words.end(); ++i)
        temp.push_back(make_pair(i->second, i->first));
    std::sort(temp.begin(), temp.end());
    fout.close(); fout.clear();
    fout.open("top_words.txt");
    for (unsigned long i = 0; i < temp.size(); ++i)
    {
        fout << temp[i].first << " \t" << temp[i].second << "\n";
    }

}

// ----------------------------------------------------------------------------------------

void test(const command_line_parser& parser)
{
    string classname;
    total_word_feature_extractor fe;
    deserialize("total_word_feature_extractor.dat") >> classname >> fe;

    cout << "words in dictionary: " << fe.get_num_words_in_dictionary() << endl;
    cout << "num features: " << fe.get_num_dimensions() << endl;

    string word = parser[0];

    matrix<float,0,1> feats;
    fe.get_feature_vector(word,feats);
    cout << "feature vector: "<< trans(feats) << endl;
}

// ----------------------------------------------------------------------------------------

