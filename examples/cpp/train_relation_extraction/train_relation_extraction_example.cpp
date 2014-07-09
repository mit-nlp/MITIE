/*
    This example shows how to use the MITIE C++ API to train a binary_relation_detector.
*/


#include <mitie/binary_relation_detector_trainer.h>
#include <iostream>



using namespace dlib;
using namespace std;
using namespace mitie;

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cout << "You must give the path to the MITIE English ner_model.dat file." << endl;
        cout << "So run this program with a command like: " << endl;
        cout << "./train_relation_extraction_example ../../../MITIE-models/english/ner_model.dat" << endl;
        return 1;
    }
    // The training process for a binary relation detector requires a MITIE NER object as
    // input.  So we load the saved NER model first.
    named_entity_extractor ner;
    string classname;
    deserialize(argv[1]) >> classname >> ner;

    // This object is responsible for doing the training work.  The first argument to the
    // constructor is a string that is used to identify the relation detector.  So you
    // should put some informative string here.  In this case, we use the name of one of
    // the freebase relations.  That is, the "person born-in location" relation.
    binary_relation_detector_trainer trainer("people.person.place_of_birth", ner);

    // When you train this kind of algorithm, you need to create a set of training
    // examples.  This dataset should include examples of the binary relations you would
    // like to detect as well as examples of things that are not what you want to detect.
    // To keep this little tutorial simple, we will use just the sentence "Ben Franklin was born in Boston" 
    // as training data, but note that for real applications you will likely require many
    // thousands of examples to create a high quality relation detector.
    //
    // So here we create a tokenized version of that sentence.  
    std::vector<std::string> sentence;
    sentence.push_back("Ben");
    sentence.push_back("Franklin");
    sentence.push_back("was");
    sentence.push_back("born");
    sentence.push_back("in");
    sentence.push_back("Boston");

    // Tell the trainer that "Ben Franklin" was born in the location "Boston".  
    trainer.add_positive_binary_relation(sentence, 
                                        make_pair(0, 2), // person's name, given as a half open range.
                                        make_pair(5, 6)); // The location, given as a half open range.

    // You should also give some negative examples.  Here we give a single negative where
    // we keep the same sentence but flip the named entity arguments.  So this is telling
    // the trainer that it is not true that Boston was born in Ben Franklin.
    trainer.add_negative_binary_relation(sentence, 
                                        make_pair(5, 6),
                                        make_pair(0, 2)); 

    // Again, note that you need much more training data than this to make high quality
    // relation detectors.  We use just this small amount here to keep the example program
    // simple.


    // This call runs the actual trainer based on all the training data.  It might take a
    // while to run so be patient.
    binary_relation_detector brd = trainer.train();

    // Once finished, we can save the relation detector to disk like so.  This will allow you 
    // to use a function call like mitie_load_binary_relation_detector("rel_classifier.svm") 
    // to read the detector later on.
    serialize("rel_classifier.svm") << "mitie::binary_relation_detector" << brd;



    // Now let's test it out a little bit.  

    // Was Ben Franklin born in Boson?  If the score is > 0 then the
    // binary_relation_detector is predicting that he was.  In this case, the number is
    // positive so the detector made the right decision.
    cout << "detection score: " << brd(extract_binary_relation(sentence, make_pair(0,2), make_pair(5,6), ner.get_total_word_feature_extractor())) << endl;

    // Now let's try a different sentence
    sentence.clear();
    sentence.push_back("Jimmy");
    sentence.push_back("Smith");
    sentence.push_back(",");
    sentence.push_back("a");
    sentence.push_back("guy");
    sentence.push_back("raised");
    sentence.push_back("in");
    sentence.push_back("France");
    // Was Jimmy Smith born in France?  Again, the detector correctly gives a number > 0.
    cout << "detection score: " << brd(extract_binary_relation(sentence, make_pair(0,2), make_pair(7,8), ner.get_total_word_feature_extractor())) << endl;

    // Now let's ask if France was born in Jimmy Smith.  This should be false and happily
    // the detector also correctly predicts a number < 0.
    cout << "detection score: " << brd(extract_binary_relation(sentence, make_pair(7,8), make_pair(0,2), ner.get_total_word_feature_extractor())) << endl;
}

// ----------------------------------------------------------------------------------------

