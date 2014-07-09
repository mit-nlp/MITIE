
#include <mitie/binary_relation_detector_trainer.h>
#include <iostream>



using namespace dlib;
using namespace std;
using namespace mitie;

// ----------------------------------------------------------------------------------------

struct relation 
{
    string relation_type;
    std::vector<std::string> tokens;
    std::pair<unsigned long, unsigned long> arg1;
    std::pair<unsigned long, unsigned long> arg2;
};

// ----------------------------------------------------------------------------------------

std::vector<relation> load_relation_data (
    const std::string& filename
)
{
    ifstream fin(filename.c_str());
    if (!fin)
        throw error("Unable to open file " + filename);
    string line;
    relation rel;
    std::vector<relation> rels;

    while (getline(fin, line))
    {
        istringstream sin(line);
        sin >> rel.relation_type;
        sin >> rel.arg1.first;
        sin >> rel.arg1.second;
        sin >> rel.arg2.first;
        sin >> rel.arg2.second;

        // discard the next \t character.
        sin.get();
        if (!sin)
            throw error("error loading relation data");

        rel.tokens.clear();
        while (getline(sin,line,'\t'))
            rel.tokens.push_back(line);

        if (!(rel.arg1.first < rel.arg1.second && 
              rel.arg2.first < rel.arg2.second &&
              rel.arg1.second <= rel.tokens.size() &&
              rel.arg2.second <= rel.tokens.size()))
        {
            throw ("Invalid range in relation data");
        }

        rels.push_back(rel);
    }
    return rels;
}

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        cout << "You must give three arguments on the command line.  The first is a MITIE NER model file." << endl;
        cout << "The second is the freebase_wikipedia_binary_relation_training_data folder, and the third" << endl;
        cout << "is the name of the freebase relation you want to train.  For example: " << endl;
        cout << "./train_relation_detector MITIE-models/english/ner_model.dat ~/freebase_wikipedia_binary_relation_training_data people.person.parents" << endl;
        return 1;
    }

    const string ner_model = argv[1];
    const string freebase_wikipedia_data = argv[2];
    const string relname = argv[3];

    named_entity_extractor ner;
    string classname;
    deserialize(ner_model) >> classname >> ner;

    random_subset_selector<relation> pos_rels, neg_rels;
    pos_rels.set_max_size(15000);
    neg_rels.set_max_size(15000);
    std::vector<relation> rels;

    // load in a bunch of training data
    rels = load_relation_data(freebase_wikipedia_data+"/filtered_freebase_relations.txt");
    for (unsigned long i = 0; i < rels.size(); ++i)
    {
        if (rels[i].relation_type == relname)
            pos_rels.add(rels[i]);
        else
            neg_rels.add(rels[i]);
    }
    rels = load_relation_data(freebase_wikipedia_data+"/unfiltered_freebase_relations.txt");
    for (unsigned long i = 0; i < rels.size(); ++i)
        neg_rels.add(rels[i]);
    rels = load_relation_data(freebase_wikipedia_data+"/not_relations.txt");
    for (unsigned long i = 0; i < rels.size(); ++i)
        neg_rels.add(rels[i]);

    cout << "pos_rels.size(): "<< pos_rels.size() << endl;
    cout << "neg_rels.size(): "<< neg_rels.size() << endl;


    binary_relation_detector_trainer trainer(relname, ner);

    // add training data to trainer
    for (unsigned long i = 0; i < pos_rels.size(); ++i)
    {
        trainer.add_positive_binary_relation(pos_rels[i].tokens, pos_rels[i].arg1, pos_rels[i].arg2);
        // The reverse of the relation is not true (at least for the relation types we got
        // from freebase)
        trainer.add_negative_binary_relation(pos_rels[i].tokens, pos_rels[i].arg2, pos_rels[i].arg1);
    }
    for (unsigned long i = 0; i < neg_rels.size(); ++i)
    {
        trainer.add_negative_binary_relation(neg_rels[i].tokens, neg_rels[i].arg1, neg_rels[i].arg2);
    }

    // train and save detector
    binary_relation_detector bd = trainer.train();
    const string outfilename = "rel_classifier_" + relname + ".svm";
    cout << "saving classifier to file: " << outfilename << endl;
    serialize(outfilename) << "mitie::binary_relation_detector" << bd;
}

// ----------------------------------------------------------------------------------------

