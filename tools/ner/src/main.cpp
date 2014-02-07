


#include <map>
#include <iostream>
#include <dlib/cmd_line_parser.h>

#include "conll_parser.h"

#include <dlib/svm_threaded.h>
#include <mitie/total_word_feature_extractor.h>
#include <mitie/stemmer.h>
#include <mitie/unigram_tokenizer.h>



using namespace dlib;
using namespace std;
using namespace mitie;

// ----------------------------------------------------------------------------------------

void train_chunker(const command_line_parser& parser);
void test_chunker(const command_line_parser& parser);
void train_id(const command_line_parser& parser);
void test_id(const command_line_parser& parser);
void tag_file(const command_line_parser& parser);

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    try
    {
        command_line_parser parser;
        parser.add_option("h", "Display this help information.");
        parser.add_option("train-chunker", "train NER chunker on conll data.");
        parser.add_option("test-chunker", "test NER chunker on conll data.");
        parser.add_option("train-id", "train NER ID/classification on conll data.");
        parser.add_option("test-id", "test NER ID/classification on conll data.");
        parser.add_option("C", "Set SVM C parameter to <arg> (default 100.0).",1);
        parser.add_option("eps", "Set SVM stopping epsilon parameter to <arg> (default 0.1).",1);
        parser.add_option("threads", "Use <arg> threads when doing training (default: 4).",1);
        parser.add_option("cache-size", "Set the max cutting plane cache size to <arg> (default: 5).",1);

        parser.add_option("tag-file", "Read in a text file and tag it with the ner model in file <arg>.",1);

        parser.parse(argc,argv);
        parser.check_option_arg_range("C", 1e-9, 1e9);
        parser.check_option_arg_range("threads", 1, 64);
        parser.check_option_arg_range("cache-size", 0, 500);

        const char* training_ops[] = {"train-chunker", "train-id"};
        const char* training_subops[] = {"C", "eps", "threads", "cache-size"};
        parser.check_sub_options(training_ops, training_subops);

        if (parser.option("h"))
        {
            cout << "Usage: ner [options]\n";
            parser.print_options(); 
            return 0;
        }

        if (parser.option("tag-file"))
        {
            tag_file(parser);
            return 0;
        }

        if (parser.option("train-chunker"))
        {
            train_chunker(parser);
            return 0;
        }

        if (parser.option("test-chunker"))
        {
            test_chunker(parser);
            return 0;
        }

        if (parser.option("train-id"))
        {
            train_id(parser);
            return 0;
        }

        if (parser.option("test-id"))
        {
            test_id(parser);
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

class ner_feature_extractor
{

public:
    typedef std::vector<matrix<float,0,1> > sequence_type;

    ner_feature_extractor() :num_feats(0) {}

    ner_feature_extractor (
        unsigned long num_feats_
    ) :
        num_feats(num_feats_)
    {}

    unsigned long num_feats;

    const static bool use_BIO_model           = false;
    const static bool use_high_order_features = false;
    const static bool allow_negative_weights  = true;

    unsigned long window_size()  const { return 3; }

    unsigned long num_features() const { return num_feats; }

    template <typename feature_setter>
    void get_features (
        feature_setter& set_feature,
        const sequence_type& sentence,
        unsigned long position
    ) const
    {
        const matrix<float,0,1>& feats = sentence[position];
        for (long i = 0; i < feats.size(); ++i)
            set_feature(i, feats(i));
    }
};

void serialize(const ner_feature_extractor& item, std::ostream& out) 
{
    dlib::serialize(item.num_feats, out);
}
void deserialize(ner_feature_extractor& item, std::istream& in) 
{
    dlib::deserialize(item.num_feats, in);
}

// ----------------------------------------------------------------------------------------

std::vector<matrix<float,0,1> > sentence_to_feats (
    const total_word_feature_extractor& fe,
    const std::vector<std::string>& sentence
)
{
    std::vector<matrix<float,0,1> > temp;
    temp.resize(sentence.size());
    for (unsigned long i = 0; i < sentence.size(); ++i)
        fe.get_feature_vector(sentence[i], temp[i]);
    return temp;
}

// ----------------------------------------------------------------------------------------

std::string get_mitie_models_path()
{
    const char* models = getenv("MITIE_MODELS");
    if (models==0)
        throw dlib::error("MITIE_MODELS environment variable not set.  It should contain the path to the MITIE-models repository.");
    return models;
}

void train_chunker(const command_line_parser& parser)
{
    std::vector<std::vector<std::string> > sentences;
    std::vector<std::vector<std::pair<unsigned long, unsigned long> > > chunks;
    std::vector<std::vector<unsigned long> > chunk_labels;
    parse_conll_data(parser[0], sentences, chunks, chunk_labels);
    cout << "number of sentences loaded: "<< sentences.size() << endl;

    const std::string models_path = get_mitie_models_path();

    total_word_feature_extractor fe;
    std::ifstream fin((models_path + "/total_word_feature_extractor.dat").c_str(), ios::binary);
    deserialize(fe, fin);

    cout << "words in dictionary: " << fe.get_num_words_in_dictionary() << endl;
    cout << "num features: " << fe.get_num_dimensions() << endl;


    // do the feature extraction for all the sentences
    std::vector<std::vector<matrix<float,0,1> > > samples;
    samples.reserve(sentences.size());
    for (unsigned long i = 0; i < sentences.size(); ++i)
    {
        samples.push_back(sentence_to_feats(fe, sentences[i]));
    }

    cout << "now do training" << endl;

    ner_feature_extractor nfe(fe.get_num_dimensions());
    structural_sequence_segmentation_trainer<ner_feature_extractor> trainer(nfe);

    const double C = get_option(parser, "C", 15.0);
    const double eps = get_option(parser, "eps", 0.01);
    const unsigned long num_threads = get_option(parser, "threads", 4);
    const unsigned long cache_size = get_option(parser, "cache-size", 5);
    cout << "C:           "<< C << endl;
    cout << "epsilon:     "<< eps << endl;
    cout << "num threads: "<< num_threads << endl;
    cout << "cache size:  "<< cache_size << endl;
    trainer.set_c(C);
    trainer.set_epsilon(eps);
    trainer.set_num_threads(num_threads);
    trainer.set_max_cache_size(cache_size);
    trainer.be_verbose();

    sequence_segmenter<ner_feature_extractor> segmenter = trainer.train(samples, chunks);

    cout << "num feats in chunker model: "<< segmenter.get_weights().size() << endl;
    cout << "precision, recall, f1-score: "<< test_sequence_segmenter(segmenter, samples, chunks) << endl;

    ofstream fout("trained_segmenter.dat", ios::binary);
    serialize(fe, fout);
    serialize(segmenter, fout);
}

// ----------------------------------------------------------------------------------------

void test_chunker(const command_line_parser& parser)
{
    std::vector<std::vector<std::string> > sentences;
    std::vector<std::vector<std::pair<unsigned long, unsigned long> > > chunks;
    std::vector<std::vector<unsigned long> > chunk_labels;
    parse_conll_data(parser[0], sentences, chunks, chunk_labels);
    cout << "number of sentences loaded: "<< sentences.size() << endl;

    ifstream fin("trained_segmenter.dat", ios::binary);
    total_word_feature_extractor fe;
    sequence_segmenter<ner_feature_extractor> segmenter;
    deserialize(fe, fin);
    deserialize(segmenter, fin);

    std::vector<std::vector<matrix<float,0,1> > > samples;
    samples.reserve(sentences.size());
    for (unsigned long i = 0; i < sentences.size(); ++i)
    {
        samples.push_back(sentence_to_feats(fe, sentences[i]));
    }

    cout << "precision, recall, f1-score: "<< test_sequence_segmenter(segmenter, samples, chunks) << endl;
}

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

const unsigned long max_feat = 500000;
inline std::pair<dlib::uint32,double> make_feat (
    const std::pair<uint64,uint64>& hash
)
{
    const double feat_weight = 1.5;
    const double rand_sign = (hash.first&1) ? 1 : -1;
    return std::make_pair(hash.second%max_feat, rand_sign*feat_weight);
}

inline std::pair<uint64,uint64> shash ( 
    const std::string& word,
    const uint32 seed 
)
{
    if (word.size() == 0)
        return make_pair(0,0);
    return murmur_hash3_128bit(&word[0], word.size(), seed);
}

inline std::pair<uint64,uint64> prefix ( 
    const std::string& word,
    const uint32 seed 
)
{
    if (word.size() == 0)
        return make_pair(0,0);
    dlib::uint32 l1 = 0, l2 = 0, l3 = 0;
    if (word.size() > 0)
        l1 = word[0];
    if (word.size() > 1)
        l2 = word[1];
    if (word.size() > 2)
        l3 = word[2];
    return murmur_hash3_128bit(l1,l2,l3,seed);
}

inline std::pair<uint64,uint64> suffix ( 
    const std::string& word,
    const uint32 seed 
)
{
    if (word.size() == 0)
        return make_pair(0,0);
    dlib::uint32 l1 = 0, l2 = 0, l3 = 0;
    if (word.size() > 0)
        l1 = word[word.size()-1];
    if (word.size() > 1)
        l2 = word[word.size()-2];
    if (word.size() > 2)
        l3 = word[word.size()-3];
    return murmur_hash3_128bit(l1,l2,l3,seed);
}

inline std::pair<uint64,uint64> ifeat ( 
    const uint32 seed 
)
{
    return murmur_hash3_128bit_3(seed,0,0);
}

// ----------------------------------------------------------------------------------------

bool is_caps ( const std::string& word)  
{
    return (word.size() != 0 && 'A' <= word[0] && word[0] <= 'Z');
}

bool is_all_caps ( const std::string& word)  
{
    for (unsigned long i = 0; i < word.size(); ++i)
    {
        if (!('A' <= word[i] && word[i] <= 'Z'))
            return false;
    }
    return true;
}

bool contains_numbers ( const std::string& word)  
{
    for (unsigned long i = 0; i < word.size(); ++i)
    {
        if ('0' <= word[i] && word[i] <= '9')
            return true;
    }
    return false;
}

bool contains_letters ( const std::string& word)  
{
    for (unsigned long i = 0; i < word.size(); ++i)
    {
        if ('a' <= word[i] && word[i] <= 'z')
            return true;
        if ('A' <= word[i] && word[i] <= 'Z')
            return true;
    }
    return false;
}

bool contains_letters_and_numbers ( const std::string& word)  
{
    return contains_letters(word) && contains_numbers(word);
}

bool is_all_numbers ( const std::string& word)  
{
    for (unsigned long i = 0; i < word.size(); ++i)
    {
        if (!('0' <= word[i] && word[i] <= '9'))
            return false;
    }
    return true;
}

bool contains_hyphen ( const std::string& word)  
{
    for (unsigned long i = 0; i < word.size(); ++i)
    {
        if (word[i] == '-')
            return true;
    }
    return false;
}

inline std::pair<uint64,uint64> caps_pattern ( 
    const std::vector<std::string>& words,
    const std::pair<unsigned long, unsigned long>& pos
) 
{
    unsigned long val = 0;
    if (pos.first != 0 && is_caps(words[pos.first-1])) 
        val |= 1;
    if (is_caps(words[pos.first])) 
        val |= 1;
    if (is_caps(words[pos.second-1])) 
        val |= 1;
    if (pos.second < words.size() && is_caps(words[pos.second]))
        val |= 1;

    return murmur_hash3_128bit_3(val,12345,5739453);
}


//typedef dlib::matrix<double,0,1> sample_type;
typedef std::vector<std::pair<dlib::uint32,double> > sample_type;

sample_type extract_chunk_features (
    const std::vector<std::string>& words,
    const std::vector<matrix<float,0,1> >& sentence,
    const std::pair<unsigned long, unsigned long>& pos
)
{
    DLIB_CASSERT(words.size() == sentence.size(), "range can't be empty");
    DLIB_CASSERT(pos.first != pos.second, "range can't be empty");


    sample_type result;
    result.reserve(1000);


    matrix<float,0,1> all_sum;
    for (unsigned long i = pos.first; i < pos.second; ++i)
    {
        all_sum += sentence[i];
        result.push_back(make_feat(shash(words[i],0)));
        result.push_back(make_feat(shash(stem_word(words[i]),10)));

        if (is_caps(words[i]))                      result.push_back(make_feat(ifeat(21)));
        if (is_all_caps(words[i]))                  result.push_back(make_feat(ifeat(22)));
        if (contains_numbers(words[i]))             result.push_back(make_feat(ifeat(23)));
        if (contains_letters(words[i]))             result.push_back(make_feat(ifeat(24)));
        if (contains_letters_and_numbers(words[i])) result.push_back(make_feat(ifeat(25)));
        if (is_all_numbers(words[i]))               result.push_back(make_feat(ifeat(26)));
        if (contains_hyphen(words[i]))              result.push_back(make_feat(ifeat(27)));

        result.push_back(make_feat(prefix(words[i],50)));
        result.push_back(make_feat(suffix(words[i],51)));

    }
    all_sum /= pos.second-pos.first;

    result.push_back(make_feat(caps_pattern(words, pos)));

    matrix<float,0,1> first = sentence[pos.first];
    matrix<float,0,1> last = sentence[pos.second-1];

    result.push_back(make_feat(shash(words[pos.first],1)));
    result.push_back(make_feat(shash(words[pos.second-1],2)));
    result.push_back(make_feat(shash(stem_word(words[pos.first]),11)));
    result.push_back(make_feat(shash(stem_word(words[pos.second-1]),12)));
    result.push_back(make_feat(prefix(words[pos.first],52)));
    result.push_back(make_feat(suffix(words[pos.first],53)));
    result.push_back(make_feat(prefix(words[pos.second-1],54)));
    result.push_back(make_feat(suffix(words[pos.second-1],55)));

    if (is_caps(words[pos.first]))                      result.push_back(make_feat(ifeat(27)));
    if (is_all_caps(words[pos.first]))                  result.push_back(make_feat(ifeat(28)));
    if (contains_numbers(words[pos.first]))             result.push_back(make_feat(ifeat(29)));
    if (contains_letters(words[pos.first]))             result.push_back(make_feat(ifeat(30)));
    if (contains_letters_and_numbers(words[pos.first])) result.push_back(make_feat(ifeat(31)));
    if (is_all_numbers(words[pos.first]))               result.push_back(make_feat(ifeat(32)));
    if (contains_hyphen(words[pos.first]))              result.push_back(make_feat(ifeat(33)));

    if (is_caps(words[pos.second-1]))                      result.push_back(make_feat(ifeat(34)));
    if (is_all_caps(words[pos.second-1]))                  result.push_back(make_feat(ifeat(35)));
    if (contains_numbers(words[pos.second-1]))             result.push_back(make_feat(ifeat(36)));
    if (contains_letters(words[pos.second-1]))             result.push_back(make_feat(ifeat(37)));
    if (contains_letters_and_numbers(words[pos.second-1])) result.push_back(make_feat(ifeat(38)));
    if (is_all_numbers(words[pos.second-1]))               result.push_back(make_feat(ifeat(39)));
    if (contains_hyphen(words[pos.second-1]))              result.push_back(make_feat(ifeat(40)));

    matrix<float,0,1> before, after;
    if (pos.first != 0)
    {
        before = sentence[pos.first-1];
        result.push_back(make_feat(shash(words[pos.first-1],3)));
        result.push_back(make_feat(shash(stem_word(words[pos.first-1]),13)));

        result.push_back(make_feat(prefix(words[pos.first-1],56)));
        result.push_back(make_feat(suffix(words[pos.first-1],57)));

        if (is_caps(words[pos.first-1]))                      result.push_back(make_feat(ifeat(60)));
        if (is_all_caps(words[pos.first-1]))                  result.push_back(make_feat(ifeat(61)));
        if (contains_numbers(words[pos.first-1]))             result.push_back(make_feat(ifeat(62)));
        if (contains_letters(words[pos.first-1]))             result.push_back(make_feat(ifeat(63)));
        if (contains_letters_and_numbers(words[pos.first-1])) result.push_back(make_feat(ifeat(64)));
        if (is_all_numbers(words[pos.first-1]))               result.push_back(make_feat(ifeat(65)));
        if (contains_hyphen(words[pos.first-1]))              result.push_back(make_feat(ifeat(66)));
    }
    else
    {
        before = zeros_matrix<float>(first.size(),1);
    }

    if (pos.first > 1)
    {
        result.push_back(make_feat(shash(words[pos.first-2],103)));
        result.push_back(make_feat(shash(stem_word(words[pos.first-2]),113)));

        result.push_back(make_feat(prefix(words[pos.first-2],156)));
        result.push_back(make_feat(suffix(words[pos.first-2],157)));

        if (is_caps(words[pos.first-2]))                      result.push_back(make_feat(ifeat(160)));
        if (is_all_caps(words[pos.first-2]))                  result.push_back(make_feat(ifeat(161)));
        if (contains_numbers(words[pos.first-2]))             result.push_back(make_feat(ifeat(162)));
        if (contains_letters(words[pos.first-2]))             result.push_back(make_feat(ifeat(163)));
        if (contains_letters_and_numbers(words[pos.first-2])) result.push_back(make_feat(ifeat(164)));
        if (is_all_numbers(words[pos.first-2]))               result.push_back(make_feat(ifeat(165)));
        if (contains_hyphen(words[pos.first-2]))              result.push_back(make_feat(ifeat(166)));
    }

    if (pos.second+1 < sentence.size())
    {
        result.push_back(make_feat(shash(words[pos.second+1],104)));
        result.push_back(make_feat(shash(stem_word(words[pos.second+1]),114)));

        result.push_back(make_feat(prefix(words[pos.second+1],158)));
        result.push_back(make_feat(suffix(words[pos.second+1],159)));

        if (is_caps(words[pos.second+1]))                      result.push_back(make_feat(ifeat(167)));
        if (is_all_caps(words[pos.second+1]))                  result.push_back(make_feat(ifeat(168)));
        if (contains_numbers(words[pos.second+1]))             result.push_back(make_feat(ifeat(169)));
        if (contains_letters(words[pos.second+1]))             result.push_back(make_feat(ifeat(170)));
        if (contains_letters_and_numbers(words[pos.second+1])) result.push_back(make_feat(ifeat(171)));
        if (is_all_numbers(words[pos.second+1]))               result.push_back(make_feat(ifeat(172)));
        if (contains_hyphen(words[pos.second+1]))              result.push_back(make_feat(ifeat(173)));
    }

    if (pos.second < sentence.size())
    {
        after = sentence[pos.second];
        result.push_back(make_feat(shash(words[pos.second],4)));
        result.push_back(make_feat(shash(stem_word(words[pos.second]),14)));

        result.push_back(make_feat(prefix(words[pos.second],58)));
        result.push_back(make_feat(suffix(words[pos.second],59)));

        if (is_caps(words[pos.second]))                      result.push_back(make_feat(ifeat(67)));
        if (is_all_caps(words[pos.second]))                  result.push_back(make_feat(ifeat(68)));
        if (contains_numbers(words[pos.second]))             result.push_back(make_feat(ifeat(69)));
        if (contains_letters(words[pos.second]))             result.push_back(make_feat(ifeat(70)));
        if (contains_letters_and_numbers(words[pos.second])) result.push_back(make_feat(ifeat(71)));
        if (is_all_numbers(words[pos.second]))               result.push_back(make_feat(ifeat(72)));
        if (contains_hyphen(words[pos.second]))              result.push_back(make_feat(ifeat(73)));
    }
    else
    {
        after = zeros_matrix<float>(first.size(),1);
    }

    const double lnorm = 0.5;
    first /= lnorm*length(first)+1e-10;
    last /= lnorm*length(last)+1e-10;
    all_sum /= lnorm*length(all_sum)+1e-10;
    before /= lnorm*length(before)+1e-10;
    after /= lnorm*length(after)+1e-10;

    matrix<double,0,1> temp = matrix_cast<double>(join_cols(join_cols(join_cols(join_cols(first,last),all_sum),before),after));

    make_sparse_vector_inplace(result);
    // append on the dense part of the feature space
    for (long i = 0; i < temp.size(); ++i)
        result.push_back(make_pair(i+max_feat, temp(i)));

    return result;
}

// ----------------------------------------------------------------------------------------

unsigned long get_label (
    const std::vector<std::pair<unsigned long, unsigned long> >& chunks,
    const std::vector<unsigned long>& chunk_labels,
    const std::pair<unsigned long, unsigned long>& range
)
/*!
    requires
        - chunks.size() == chunk_labels.size()
    ensures
        - This function checks if any of the elements of chunks are equal to range.  If so,
          then the corresponding chunk label is returned.  Otherwise a value of NOT_ENTITY
          is returned.
!*/
{
    for (unsigned long i = 0; i < chunks.size(); ++i)
    {
        if (range == chunks[i])
            return chunk_labels[i];
    }
    return NOT_ENTITY;
}

// ----------------------------------------------------------------------------------------

namespace mitie
{
    class named_entity_extractor
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
        !*/
    public:

        named_entity_extractor(){}

        named_entity_extractor(
            const std::map<unsigned long, std::string>& possible_tags_,
            const total_word_feature_extractor& fe_,
            const dlib::sequence_segmenter<ner_feature_extractor>& segmenter_,
            const dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<sample_type>,unsigned long>& df_
        ) : possible_tags(possible_tags_), fe(fe_), segmenter(segmenter_), df(df_) 
        {
            DLIB_CASSERT(df.number_of_classes() == possible_tags.size()+1,"invalid inputs");
        }

        void operator() (
            const std::vector<std::string>& sentence,
            std::vector<std::pair<unsigned long, unsigned long> >& chunks,
            std::vector<unsigned long>& chunk_tags
        ) const
        {
            const std::vector<matrix<float,0,1> >& sent = sentence_to_feats(fe, sentence);
            segmenter.segment_sequence(sent, chunks);

            const unsigned long NOT_AN_ENTITY = df.number_of_classes()-1;

            chunk_tags.resize(chunks.size());
            // now label each chunk
            for (unsigned long j = 0; j < chunks.size(); )
            {
                chunk_tags[j] = df(extract_chunk_features(sentence, sent, chunks[j]));

                // if this chunk is predicted to not be an entity then erase it
                if (chunk_tags[j] == NOT_AN_ENTITY)
                {
                    std::swap(chunk_tags[j], chunk_tags.back());
                    std::swap(chunks[j], chunks.back());
                    chunk_tags.resize(chunk_tags.size()-1);
                    chunks.resize(chunks.size()-1);
                }
                else
                {
                    ++j;
                }
            }
        }

        const std::map<unsigned long, std::string>& get_possible_tags (
        ) const { return possible_tags; }

        friend void serialize(const named_entity_extractor& item, std::ostream& out)
        {
            int version = 1;
            dlib::serialize(version, out);
            dlib::serialize(item.possible_tags, out);
            serialize(item.fe, out);
            serialize(item.segmenter, out);
            serialize(item.df, out);
        }

        friend void deserialize(named_entity_extractor& item, std::istream& in)
        {
            int version = 0;
            dlib::deserialize(version, in);
            if (version != 1)
                throw dlib::serialization_error("Unexpected version found while deserializing mitie::named_entity_extractor.");
            dlib::deserialize(item.possible_tags, in);
            deserialize(item.fe, in);
            deserialize(item.segmenter, in);
            deserialize(item.df, in);
        }

    private:
        std::map<unsigned long, std::string> possible_tags;
        total_word_feature_extractor fe;
        dlib::sequence_segmenter<ner_feature_extractor> segmenter;
        dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<sample_type>,unsigned long> df;
    };
}

// ----------------------------------------------------------------------------------------

void train_id(const command_line_parser& parser)
{
    std::vector<std::vector<std::string> > sentences;
    std::vector<std::vector<std::pair<unsigned long, unsigned long> > > chunks;
    std::vector<std::vector<unsigned long> > chunk_labels;
    parse_conll_data(parser[0], sentences, chunks, chunk_labels);
    cout << "number of sentences loaded: "<< sentences.size() << endl;


    ifstream fin("trained_segmenter.dat", ios::binary);
    total_word_feature_extractor fe;
    sequence_segmenter<ner_feature_extractor> segmenter;
    deserialize(fe, fin);
    deserialize(segmenter, fin);

    std::vector<sample_type> samples;
    std::vector<unsigned long> labels;
    for (unsigned long i = 0; i < sentences.size(); ++i)
    {
        const std::vector<matrix<float,0,1> >& sent = sentence_to_feats(fe, sentences[i]);
        std::set<std::pair<unsigned long, unsigned long> > ranges;
        // put all the true chunks into ranges
        ranges.insert(chunks[i].begin(), chunks[i].end());

        // now get all the chunks our segmenter finds
        std::vector<std::pair<unsigned long, unsigned long> > temp;
        temp = segmenter(sent);
        ranges.insert(temp.begin(), temp.end());

        // now go over all the chunks we found and label them with their appropriate NER
        // types and also do feature extraction for each.
        std::set<std::pair<unsigned long,unsigned long> >::const_iterator j;
        for (j = ranges.begin(); j != ranges.end(); ++j)
        {
            samples.push_back(extract_chunk_features(sentences[i], sent, *j));
            labels.push_back(get_label(chunks[i], chunk_labels[i], *j));
        }
    }

    cout << "now do training" << endl;
    cout << "num training samples: " << samples.size() << endl;

    svm_multiclass_linear_trainer<sparse_linear_kernel<sample_type>,unsigned long> trainer;

    const double C = get_option(parser, "C", 450.0);
    const double eps = get_option(parser, "eps", 0.001);
    const unsigned long num_threads = get_option(parser, "threads", 4);
    cout << "C:           "<< C << endl;
    cout << "epsilon:     "<< eps << endl;
    cout << "num_threads: "<< num_threads << endl;
    trainer.set_c(C);
    trainer.set_epsilon(eps);
    trainer.be_verbose();
    trainer.set_num_threads(num_threads);

    randomize_samples(samples, labels);
    /*
    matrix<double> res = cross_validate_multiclass_trainer(trainer, samples, labels, 5);
    cout << "5-fold cross-validation: \n" << res << endl;
    cout << "overall accuracy: "<< sum(diag(res))/sum(res) << endl;
    */

    multiclass_linear_decision_function<sparse_linear_kernel<sample_type>,unsigned long> df;
    df = trainer.train(samples, labels);
    matrix<double> res = test_multiclass_decision_function(df, samples, labels);
    cout << "test on train: \n" << res << endl;
    cout << "overall accuracy: "<< sum(diag(res))/sum(res) << endl;

    cout << "C:           "<< C << endl;
    cout << "epsilon:     "<< eps << endl;
    cout << "num_threads: "<< num_threads << endl;

    std::map<unsigned long,std::string> ner_labels;
    ner_labels[PER] = "PERSON";
    ner_labels[LOC] = "LOCATION";
    ner_labels[ORG] = "ORGANIZATION";
    ner_labels[MISC] = "MISC";
    named_entity_extractor ner(ner_labels, fe, segmenter, df);
    ofstream fout("ner_model.dat", ios::binary);
    serialize(ner, fout);
}

// ----------------------------------------------------------------------------------------

void test_id(const command_line_parser& parser)
{
    named_entity_extractor ner;
    ifstream fin("ner_model.dat", ios::binary);
    deserialize(ner, fin);

    std::vector<std::vector<std::string> > sentences;
    std::vector<std::vector<std::pair<unsigned long, unsigned long> > > chunks;
    std::vector<std::vector<unsigned long> > chunk_labels;
    parse_conll_data(parser[0], sentences, chunks, chunk_labels);
    cout << "number of sentences loaded: "<< sentences.size() << endl;


    const unsigned long num_labels = ner.get_possible_tags().size();
    std::vector<double> num_targets(num_labels);
    std::vector<double> num_dets(num_labels);
    std::vector<double> num_true_dets(num_labels);

    std::vector<std::pair<unsigned long, unsigned long> > ranges;
    std::vector<unsigned long> predicted_labels;

    for (unsigned long i = 0; i < sentences.size(); ++i)
    {
        ner(sentences[i], ranges, predicted_labels);

        for (unsigned long j = 0; j < ranges.size(); ++j)
        {
            const unsigned long predicted_label = predicted_labels[j];
            const unsigned long true_label = get_label(chunks[i], chunk_labels[i], ranges[j]);

            num_dets[predicted_label]++;
            if (predicted_label == true_label)
                num_true_dets[true_label]++;
        }
        for (unsigned long j = 0; j < chunk_labels[i].size(); ++j)
        {
            num_targets[chunk_labels[i][j]]++;
        }
    }

    cout << "results: "<< endl;
    for (unsigned long i = 0; i < num_targets.size(); ++i)
    {
        cout << "label: "<< i << endl;
        double prec = num_true_dets[i]/num_dets[i];
        double recall = num_true_dets[i]/num_targets[i];
        cout << "   precision: "<< prec << endl;
        cout << "   recall:    "<< recall << endl;
        cout << "   f1:        "<< 2*prec*recall/(prec+recall) << endl;
        cout << endl;
    }

    cout << "total: " << endl;
    double prec = sum(mat(num_true_dets))/sum(mat(num_dets));
    double recall = sum(mat(num_true_dets))/sum(mat(num_targets));
    cout << "   precision: "<< prec << endl;
    cout << "   recall:    "<< recall << endl;
    cout << "   f1:        "<< 2*prec*recall/(prec+recall) << endl;

}

// ----------------------------------------------------------------------------------------

void tag_file(const command_line_parser& parser)
{
    string ner_model = parser.option("tag-file").argument();
    ifstream fin(ner_model.c_str(), ios::binary);
    named_entity_extractor ner;
    deserialize(ner, fin);

    fin.close();
    fin.open(parser[0].c_str());

    unigram_tokenizer tok(fin);

    std::vector<std::string> words;
    string word;
    while(tok(word))
        words.push_back(word);

    std::vector<std::pair<unsigned long, unsigned long> > ranges;
    std::vector<unsigned long> predicted_labels;
    std::map<unsigned long, std::string> tags = ner.get_possible_tags();
    tags[tags.size()] = "O";
    ner(words, ranges, predicted_labels);

    std::vector<unsigned long> word_tags(words.size(), tags.size()-1);
    for (unsigned long i = 0; i < ranges.size(); ++i)
    {
        for (unsigned long j = ranges[i].first; j < ranges[i].second; ++j)
        {
            word_tags[j] = predicted_labels[i];
        }
    }

    for (unsigned long i = 0; i < words.size(); ++i)
    {
        cout << words[i] << "/" << tags[word_tags[i]] << " ";
    }

}

// ----------------------------------------------------------------------------------------

