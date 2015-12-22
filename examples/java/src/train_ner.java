/*
    This example shows how to use the MITIE Java API to perform named entity recognition
    and also how to run a binary relation detector on top of the named entity recognition
    outputs.
*/
import edu.mit.ll.mitie.*;

public class train_ner
{
    public static void main(String args[])
    {
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

        NerTrainer nerTrainer = new NerTrainer("../../MITIE-models/english/total_word_feature_extractor.dat");
        nerTrainer.add(nerTrainingInstance.getImpl());
        nerTrainer.add(nerTrainingInstance1.getImpl());

        nerTrainer.setThreadNum(4);

        nerTrainer.train("test_ner_model.dat");

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


