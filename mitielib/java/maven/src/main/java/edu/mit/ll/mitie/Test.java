package edu.mit.ll.mitie;

import java.io.IOException;

/**
 * Created by wihoho on 4/1/16.
 */
public class Test {

    public static void main(String args[]) throws IOException {
        // Note that, to use MITIE you must have already compiled the MITIE Java API
        // located in mitielib/java.  It will have produced a javamitie.jar file and a
        // shared library or dll.  Moreover, you must make sure that the shared library or
        // dll file is in your system PATH if on Windows or the LD_LIBRARY_PATH environment
        // variable if on a UNIX system.  This example program comes with both .bat and .sh
        // scripts that show how to do this on any system.

        System.out.println("loading NER model...");
        NamedEntityExtractor ner = new NamedEntityExtractor("../../MITIE-models/english/ner_model.dat");

        System.out.println("Tags output by this NER model are: ");
        edu.mit.ll.mitie.StringVector possibleTags = ner.getPossibleNerTags();
        for (int i = 0; i < possibleTags.size(); ++i)
            System.out.println(possibleTags.get(i));


        // Load a text file and convert it into a list of words.
        StringVector words = global.tokenize(global.loadEntireFile("../../sample_text.txt"));


        // Now ask MITIE to find all the named entities in the file we just loaded.
        EntityMentionVector entities = ner.extractEntities(words);
        System.out.println("Number of entities found: " + entities.size());

        // Now print out all the named entities and their tags
        for (int i = 0; i < entities.size(); ++i) {
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
            printEntity(words, entity);
        }

    }

    public static void printEntity(
            StringVector words,
            EntityMention ent
    ) {
        // Print all the words in the range indicated by the entity ent.
        for (int i = ent.getStart(); i < ent.getEnd(); ++i) {
            System.out.print(words.get(i) + " ");
        }
        System.out.println("");
    }
}
