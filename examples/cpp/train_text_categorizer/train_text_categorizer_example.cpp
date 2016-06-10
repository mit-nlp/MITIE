/*
    This example shows how to use the MITIE C++ API to train a text_categorizer.
*/

#include <mitie/ner_trainer.h>
#include <mitie/text_categorizer_trainer.h>
#include <iostream>

using namespace dlib;
using namespace std;
using namespace mitie;

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    /*
        When you train a text_categorizer you need to get a dataset of documents (or
        sentence or paragraph length chunks of text) where each document is annotated with
        the label you want to find.  For example, if we wanted to detect the sentiment
        (positive or negative) of a sentence, then we would need to get a bunch of sentences
        with examples of both positive and negative ones.  Here is an example:
            Positive expression: It is so happy and exciting to make this
            Negative expression: What a black and bad day

        You then give MITIE these example sentences with their labels and it will
        learn to detect them.  That is what we do below.  
    */

    // So let's make the first training example.  We use the sentence above.  Note that the
    // training API takes tokenized sentences.  It is up to you how you tokenize them, you
    // can use the default conll_tokenizer that comes with MITIE or any other method you like.  
    std::vector<std::string> sentence;
    sentence.push_back("I");
    sentence.push_back("am");
    sentence.push_back("so");
    sentence.push_back("happy");
    sentence.push_back("and");
    sentence.push_back("exciting");
    sentence.push_back("to");
    sentence.push_back("make");
    sentence.push_back("this");

    std::vector<std::string> sentence2;
    sentence2.push_back("What");
    sentence2.push_back("a");
    sentence2.push_back("black");
    sentence2.push_back("and");
    sentence2.push_back("bad");
    sentence2.push_back("day");

    // Now that we have some annotated example sentences we can create the object that does
    // the actual training, the text_categorizer_trainer.  The constructor for this object takes a string
    // that should contain the file name for a saved mitie::total_word_feature_extractor.
    // The total_word_feature_extractor is MITIE's primary method for analyzing words and
    // is created by the tool in the MITIE/tools/wordrep folder.  The wordrep tool analyzes
    // a large document corpus, learns important word statistics, and then outputs a
    // total_word_feature_extractor that is knowledgeable about a particular language (e.g.
    // English).  MITIE comes with a total_word_feature_extractor for English so that is
    // what we use here.  But if you need to make your own you do so using a command line 
    // statement like:
    //    wordrep -e a_folder_containing_only_text_files
    // and wordrep will create a total_word_feature_extractor.dat based on the supplied
    // text files.  Note that wordrep can take a long time to run or require a lot of RAM
    // if a large text dataset is given.  So use a powerful machine and be patient.
    if (argc != 2) {
        cout << "You must give the path to the MITIE English total_word_feature_extractor.dat file." << endl;
        cout << "So run this program with a command like: " << endl;
        cout << "./train_text_categorizer_example ../../../MITIE-models/english/total_word_feature_extractor.dat" << endl;
        return 1;
    }
    text_categorizer_trainer trainer(argv[1]);
    // Don't forget to add the training data.  Here we have only two examples, but for real
    // uses you need to have thousands.  
    trainer.add(sentence, "positive");
    trainer.add(sentence2, "negative");

    // The trainer can take advantage of a multi-core CPU.  So set the number of threads
    // equal to the number of processing cores for maximum training speed.
    trainer.set_num_threads(4);
    // This function does the work of training.  Note that it can take a long time to run
    // when using larger training datasets.  So be patient.
    text_categorizer categorizer = trainer.train();

    // Now that training is done we can save the categorizer object to disk like so.  This will
    // allow you to load the model back in using the following codes
    // string classname;
    // text_categorizer categorizer;
    // dlib::deserialize("new_text_categorizer_model.dat") >> classname >> categorizer;
    serialize("new_text_categorizer_model.dat") << "mitie::text_categorizer" << categorizer;

    // But now let's try out the categorizer.  It was only trained on a small dataset but it
    // has still learned a little.  So let's give it a whirl.  But first, print a list of
    // possible labels.  In this case, it is just "positive" and "negative".
    const std::vector<string> tagstr = categorizer.get_tag_name_strings();
    cout << "The tagger supports " << tagstr.size() << " tags:" << endl;
    for (unsigned int i = 0; i < tagstr.size(); ++i)
        cout << "   " << tagstr[i] << endl;

    // Now let's make up a test sentence
    std::vector<std::string> sentence3;
    sentence3.push_back("It");
    sentence3.push_back("is");
    sentence3.push_back("really");
    sentence3.push_back("exciting");

    string text_tag;
    double text_score;
    categorizer.predict(sentence3, text_tag, text_score);
    // Happily, it found the correct answers, "Positive", in this case which we
    // print out below.

    cout << "This is a " << text_tag << " text, with score as " << text_score << endl;
}

// ----------------------------------------------------------------------------------------

