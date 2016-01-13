package edu.mit.ll.mitie;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * Created by wihoho on 9/1/16.
 */
public class NerTrainerTest {

    @Test
    public void testTrainSeparateModels() throws Exception {
        // train models using the separation API
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

        NerMicroTrainingInstance nerMicroTrainingInstance = new NerMicroTrainingInstance(stringVector);
        nerMicroTrainingInstance.addEntity(3, 2, "person");
        nerMicroTrainingInstance.addEntity(9, 1, "org");

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

        NerMicroTrainingInstance nerMicroTrainingInstance1 = new NerMicroTrainingInstance(stringVector12);
        nerMicroTrainingInstance1.addEntity(7, 2, "person");
        nerMicroTrainingInstance1.addEntity(10, 1, "org");

        TotalWordFeatureExtractor totalWordFeatureExtractor = TotalWordFeatureExtractor.getEnglishExtractor();
        MicroTrainer microTrainer = new MicroTrainer();
        MicroTrainer microTrainer1 = new MicroTrainer();

        microTrainer.add(nerMicroTrainingInstance);
        microTrainer.add(nerMicroTrainingInstance1);

        microTrainer1.add(nerMicroTrainingInstance);
        microTrainer1.add(nerMicroTrainingInstance1);

        // The trainer can take advantage of a multi-core CPU.  So set the number of threads
        // equal to the number of processing cores for maximum training speed.
        microTrainer.setThreadNum(4);

        // This function does the work of training.  Note that it can take a long time to run
        // when using larger training datasets.  So be patient.  When it finishes it will
        // save the resulting pure model
        File file = File.createTempFile( "train", "model.dat");
        file.deleteOnExit();

        microTrainer.trainSeparateModels(totalWordFeatureExtractor, file.getAbsolutePath());

        microTrainer1.trainSeparateModels(totalWordFeatureExtractor, file.getAbsolutePath());

        // restore the model using the pure model and extractor
        MicroNER ner = new MicroNER(file.getAbsolutePath());

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

        System.out.println("Tags output by this NER model are: ");
        StringVector possibleTags = ner.getPossibleNerTags();
        for (int i = 0; i < possibleTags.size(); ++i)
            System.out.println(possibleTags.get(i));

        // Now ask MITIE to find all the named entities in the file we just loaded.
        EntityMentionVector entities = ner.extractEntities(totalWordFeatureExtractor, testStringVector);
        System.out.println("Number of entities found: " + entities.size());

        Map<String, String> mapResult = new HashMap<>();
        // Now print out all the named entities and their tags
        for (int i = 0; i < entities.size(); ++i)
        {
            EntityMention entity = entities.get(i);
            String tag = possibleTags.get(entity.getTag());
            Double score = entity.getScore();
            String scoreStr = String.format("%1$,.3f",score);
            System.out.print("   Score: " + scoreStr + ": " + tag + ":");
            String current = printEntity(testStringVector, entity);
            mapResult.put(tag, current);
        }

        assertEquals(2, entities.size());
        assertEquals("John Becker ", mapResult.get("person"));
        assertEquals("HBU ", mapResult.get("org"));
    }





    public static String printEntity (
            StringVector words,
            EntityMention ent
    )
    {
        StringBuffer sb = new StringBuffer();
        // Print all the words in the range indicated by the entity ent.
        for (int i = ent.getStart(); i < ent.getEnd(); ++i)
        {

            sb.append(words.get(i) + " ");
        }
        sb.append("");
        System.out.println(sb);

        return sb.toString();
    }
}