/*
    This example shows how to use the MITIE Java API to perform named entity recognition
    and also how to run a binary relation detector on top of the named entity recognition
    outputs.
*/
import edu.mit.ll.mitie.*;

public class TrainNerExample 
{
    public static void main(String args[])
    {
        /*
            When you train a NamedEntityExtractor you need to get a dataset of sentences (or
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
        // can use the default tokenizer that comes with MITIE or any other method you like.  
        StringVector stringVector = new StringVector();
        stringVector.add("My");
        stringVector.add("name");
        stringVector.add("is");
        stringVector.add("Davis");
        stringVector.add("King");
        stringVector.add("and");
        stringVector.add("I");
        stringVector.add("work");
        stringVector.add("for");
        stringVector.add("MIT");
        stringVector.add(".");
        
        // Now that we have the tokens stored, we add the entity annotations.  The first
        // annotation indicates that the token at index 3 and consisting of 2 tokens is a
        // person.  I.e. "Davis King" is a person name.  Note that you can use any strings
        // as the labels.  Here we use "person" and "org" but you could use any labels you
        // like. 
        NerTrainingInstance nerTrainingInstance = new NerTrainingInstance(stringVector);
        nerTrainingInstance.addEntity(3, 2, "person");
        nerTrainingInstance.addEntity(9, 1, "org");

        StringVector stringVector12 = new StringVector();
        stringVector12.add("The");
        stringVector12.add("other");
        stringVector12.add("day");
        stringVector12.add("at");
        stringVector12.add("work");
        stringVector12.add("I");
        stringVector12.add("saw");
        stringVector12.add("Brian");
        stringVector12.add("Smith");
        stringVector12.add("from");
        stringVector12.add("CMU");
        stringVector12.add(".");

        NerTrainingInstance nerTrainingInstance1 = new NerTrainingInstance(stringVector12);
        nerTrainingInstance1.addEntity(7, 2, "person");
        nerTrainingInstance1.addEntity(10, 1, "org");

        // Now that we have some annotated example sentences we can create the object that does
        // the actual training, the NerTrainer.  The constructor for this object takes a string 
        // that should contain the file name for a saved mitie::total_word_feature_extractor C++ object.
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
        NerTrainer nerTrainer = new NerTrainer("../../MITIE-models/english/total_word_feature_extractor.dat");
        // Don't forget to add the training data.  Here we have only two examples, but for real
        // uses you need to have thousands.  
        nerTrainer.add(nerTrainingInstance);
        nerTrainer.add(nerTrainingInstance1);

        // The trainer can take advantage of a multi-core CPU.  So set the number of threads
        // equal to the number of processing cores for maximum training speed.
        nerTrainer.setThreadNum(4);

        // This function does the work of training.  Note that it can take a long time to run
        // when using larger training datasets.  So be patient.  When it finishes it will
        // save the resulting model to the test_ner_model.dat file.
        nerTrainer.train("test_ner_model.dat");


        // Finally, lets test out our new model on an example sentence
        StringVector testStringVector = new StringVector();
        testStringVector.add("I");
        testStringVector.add("met");
        testStringVector.add("with");
        testStringVector.add("John");
        testStringVector.add("Becker");
        testStringVector.add("at");
        testStringVector.add("HBU");
        testStringVector.add(".");

        NamedEntityExtractor ner = new NamedEntityExtractor("test_ner_model.dat");
        System.out.println("Tags output by this NER model are: ");
        StringVector possibleTags = ner.getPossibleNerTags();
        for (int i = 0; i < possibleTags.size(); ++i)
            System.out.println(possibleTags.get(i));

        // Now ask MITIE to find all the named entities in the file we just loaded.
        EntityMentionVector entities = ner.extractEntities(testStringVector);
        System.out.println("Number of entities found: " + entities.size());

        // Now print out all the named entities and their tags
        for (int i = 0; i < entities.size(); ++i)
        {
            EntityMention entity = entities.get(i);
            String tag = possibleTags.get(entity.getTag());
            Double score = entity.getScore();
            String scoreStr = String.format("%1$,.3f",score);
            System.out.print("   Score: " + scoreStr + ": " + tag + ":");
            NerExample.printEntity(testStringVector, entity);
        }


    }
}


