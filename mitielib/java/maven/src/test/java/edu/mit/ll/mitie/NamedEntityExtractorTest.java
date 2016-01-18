package edu.mit.ll.mitie;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import com.google.common.io.ByteStreams;
import com.google.common.io.Resources;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * Created by wihoho on 9/1/16.
 */
public class NamedEntityExtractorTest {

    @Test
    public void testExtractEntities() throws Exception {
        System.out.println("loading NER model...");

        File file = File.createTempFile("model", ".dat");
        file.deleteOnExit();

        FileOutputStream fileOutputStream = new FileOutputStream(file);

        InputStream modelStream = Resources.getResource("models/testModel.dat").openStream();
        ByteStreams.copy(modelStream, fileOutputStream);
        modelStream.close();
        fileOutputStream.close();

        TotalWordFeatureExtractor totalWordFeatureExtractor = TotalWordFeatureExtractor.getEnglishExtractor();

        MicroNER ner = new MicroNER(file.getAbsolutePath());
        MicroNER ner1 = new MicroNER(file.getAbsolutePath());
        //NamedEntityExtractor ner = new NamedEntityExtractor(file.getAbsolutePath(), totalWordFeatureExtractor);
        //NamedEntityExtractor ner2 = new NamedEntityExtractor(file.getAbsolutePath(), totalWordFeatureExtractor);
        //NamedEntityExtractor ner3 = new NamedEntityExtractor(file.getAbsolutePath(), totalWordFeatureExtractor);

        System.out.println("Tags output by this NER model are: ");
        StringVector possibleTags = ner.getPossibleNerTags();

        Set<String> tagSet = new HashSet<>();
        for (int i = 0; i < possibleTags.size(); ++i) {
            System.out.println(possibleTags.get(i));
            tagSet.add(possibleTags.get(i));
        }

        assertTrue(tagSet.contains("person"));
        assertTrue(tagSet.contains("org"));

        // Load a text file and convert it into a list of words.
        StringVector words = global.tokenize("Ted Mosby studies at NTU.");

        // Now ask MITIE to find all the named entities in the file we just loaded.
        //EntityMentionVector entities = ner.extractEntities(words);
        EntityMentionVector entities = ner.extractEntities(totalWordFeatureExtractor, words);
        EntityMentionVector entities1 = ner.extractEntities(totalWordFeatureExtractor, words);
        System.out.println("Number of entities found: " + entities.size());

        // Now print out all the named entities and their tags
        Map<String, String> mapResult = new HashMap<>();
        for (int i = 0; i < entities.size(); ++i)
        {
            // Each EntityMention contains three integers and a double. The start and end
            // define the range of tokens in the words vector that are part of the entity.
            // There is also a tag which indicates which element of possibleTags is
            // associated with the entity. There is also a score which indicates a
            // confidence associated with the predicted tag (larger values mean MITIE is
            // more confident in its prediction). So we can print out all the tagged
            // entities as follows:
            EntityMention entity = entities.get(i);
            String tag = possibleTags.get(entity.getTag());
            Double score = entity.getScore();
            String scoreStr = String.format("%1$,.3f", score);
            System.out.print("   Score: " + scoreStr + ": " + tag + ":");
            String current = printEntity(words, entity);
            mapResult.put(tag, current);
        }


        assertEquals(2, entities.size());
        assertEquals("Ted Mosby ", mapResult.get("person"));
        assertEquals("NTU ", mapResult.get("org"));
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