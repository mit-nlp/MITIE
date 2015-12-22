/*
    This example shows how to use the MITIE C++ API to train a named_entity_extractor.
*/

#include <mitie/ner_trainer.h>
#include <iostream>

using namespace dlib;
using namespace std;
using namespace mitie;

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    /*
        When you train a named_entity_extractor you need to get a dataset of sentences (or
        sentence or paragraph length chunks of text) where each sentence is annotated with
        the entities you want to find.  For example, if we wanted to find all the names of
        people and organizations then we would need to get a bunch of sentences with
        examples of person names and organizations in them.  Here is an example:
            My name is Davis King and I work for MIT.
        "Davis King" is a person name and "MIT" is an organization.  

        You then give MITIE these example sentences with their entity annotations and it will
        learn to detect them.  That is what we do below.  
    */

    // So let's make the first training example.  We use the sentence above.  Note that the
    // training API takes tokenized sentences.  It is up to you how you tokenize them, you
    // can use the default conll_tokenizer that comes with MITIE or any other method you like.  
    std::vector<std::string> sentence;
    sentence.push_back("My");
    sentence.push_back("name");
    sentence.push_back("is");
    sentence.push_back("Davis");
    sentence.push_back("King");
    sentence.push_back("and");
    sentence.push_back("I");
    sentence.push_back("work");
    sentence.push_back("for");
    sentence.push_back("MIT");
    sentence.push_back(".");
    // Now that we have the tokens stored, we add the entity annotations.  The first
    // annotation indicates that the token at index 3 and consisting of 2 tokens is a
    // person.  I.e. "Davis King" is a person name.  Note that you can use any strings as
    // the labels.  Here we use "person" and "org" but you could use any labels you like. 
    ner_training_instance sample(sentence);
    sample.add_entity(3,2,"person");
    sample.add_entity(9,1,"org");

    std::vector<std::string> sentence2;
    sentence2.push_back("The");
    sentence2.push_back("other");
    sentence2.push_back("day");
    sentence2.push_back("at");
    sentence2.push_back("work");
    sentence2.push_back("I");
    sentence2.push_back("saw");
    sentence2.push_back("Brian");
    sentence2.push_back("Smith");
    sentence2.push_back("from");
    sentence2.push_back("CMU");
    sentence2.push_back(".");
    ner_training_instance sample2(sentence2);
    sample2.add_entity(7,2,"person");
    sample2.add_entity(10,1,"org");


    // Now that we have some annotated example sentences we can create the object that does
    // the actual training, the ner_trainer.  The constructor for this object takes a string 
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
    if (argc != 2)
    {
        cout << "You must give the path to the MITIE English total_word_feature_extractor.dat file." << endl;
        cout << "So run this program with a command like: " << endl;
        cout << "./train_ner_example ../../../MITIE-models/english/total_word_feature_extractor.dat" << endl;
        return 1;
    }
    ner_trainer trainer(argv[1]);
    // Don't forget to add the training data.  Here we have only two examples, but for real
    // uses you need to have thousands.  
    trainer.add(sample);
    trainer.add(sample2);

    // The trainer can take advantage of a multi-core CPU.  So set the number of threads
    // equal to the number of processing cores for maximum training speed.
    trainer.set_num_threads(4);
    // This function does the work of training.  Note that it can take a long time to run
    // when using larger training datasets.  So be patient.
    named_entity_extractor ner = trainer.train();

    // Now that training is done we can save the ner object to disk like so.  This will
    // allow you to load the model back in using mitie_load_named_entity_extractor("new_ner_model.dat").
    serialize("new_ner_model.dat") << "mitie::named_entity_extractor" << ner;


    // But now let's try out the ner object.  It was only trained on a small dataset but it
    // has still learned a little.  So let's give it a whirl.  But first, print a list of
    // possible tags.  In this case, it is just "person" and "org".
    const std::vector<string> tagstr = ner.get_tag_name_strings();
    cout << "The tagger supports "<< tagstr.size() <<" tags:" << endl;
    for (unsigned int i = 0; i < tagstr.size(); ++i)
        cout << "   " << tagstr[i] << endl;

    // Now let's make up a test sentence
    std::vector<std::string> sentence3;
    sentence3.push_back("I");
    sentence3.push_back("met");
    sentence3.push_back("with");
    sentence3.push_back("John");
    sentence3.push_back("Becker");
    sentence3.push_back("at");
    sentence3.push_back("HBU");
    sentence3.push_back(".");

    // Then ask our new ner object to detect the entities.
    std::vector<pair<unsigned long, unsigned long> > chunks;
    std::vector<unsigned long> chunk_tags;
    ner(sentence3, chunks, chunk_tags);
    // Happily, it found the correct answers, "John Becker" and "HBU" in this case which we
    // print out below.
    cout << "\nNumber of named entities detected: " << chunks.size() << endl;
    for (unsigned int i = 0; i < chunks.size(); ++i)
    {
        cout << "   Tag " << chunk_tags[i] << ":" << tagstr[chunk_tags[i]] << ": ";
        // chunks[i] defines a half open range in sentence3 that contains the entity.
        for (unsigned long j = chunks[i].first; j < chunks[i].second; ++j)
            cout << sentence3[j] << " ";
        cout << endl;
    }
}

// ----------------------------------------------------------------------------------------

