/*
    This example shows how to use the MITIE Java API to perform named entity recognition
    and also how to run a binary relation detector on top of the named entity recognition
    outputs.
*/
import edu.mit.ll.mitie.*;

public class nerExample 
{
    public static void main(String args[])
    {
        // To use MITIE, the first thing you need to do is load the compiled C++ library
        // part of MITIE.  For this to work you must have already compiled the MITIE Java
        // API located in mitielib/java.  Moreover, you must also make sure that the
        // javamitie shared library or dll file is in your system PATH if on Windows or the
        // LD_LIBRARY_PATH environment variable if on a UNIX system.  Note that this
        // example program comes with both .bat and .sh scripts that show how to do this on
        // any system.
        System.loadLibrary("javamitie");
        


        System.out.println("loading NER model...");
        NamedEntityExtractor ner = new NamedEntityExtractor("../../MITIE-models/english/ner_model.dat");

        System.out.println("Tags output by this NER model are: ");
        StringVector possibleTags = ner.getPossibleNerTags();
        for (int i = 0; i < possibleTags.size(); ++i)
            System.out.println(possibleTags.get(i));


        // Load a text file and convert it into a list of words.
        StringVector words = mitie.tokenize(mitie.loadEntireFile("../../sample_text.txt"));


        // Now ask MITIE to find all the named entities in the file we just loaded.
        EntityMentionVector entities = ner.extractEntities(words);
        System.out.println("Number of entities found: " + entities.size());

        // Now print out all the named entities and their tags
        for (int i = 0; i < entities.size(); ++i)
        {
            // Each EntityMention contains three integers.  The start and end define the
            // range of tokens in the words vector that are part of the entity.  There is
            // also a tag which indicates which element of possibleTags is associated with
            // the entity.  So we can print out all the tagged entities as follows:
            EntityMention entity = entities.get(i);
            String tag = possibleTags.get(entity.getTag());
            System.out.print("Entity tag: " + tag + "\t Entity text: ");
            printEntity(words, entity);
        }







        // Now let's run one of MITIE's binary relation detectors.  MITIE comes with a
        // bunch of different types of relation detector and includes tools allowing you
        // to train new detectors.  However, here we simply use one, the "person born in
        // place" relation detector.
        BinaryRelationDetector relDetector = new BinaryRelationDetector("../../MITIE-models/english/binary_relations/rel_classifier_people.person.place_of_birth.svm");
        System.out.println("Relation detector type: " + relDetector.getNameString());

        // Let's ask MITIE if any of the neighboring pairs of entities are examples of the
        // "person born in place" relation.
        for (int i = 1; i < entities.size(); ++i)
        {
            // Was entities.get(i) born in entities.get(i-1)?  Note that the detection has
            // two steps in MITIE. First, you convert a pair of entities into a special
            // representation.  Then you ask the detector to classify that pair of
            // entities.  If the score value is > 0 then it is saying that it has found a
            // relation.  The larger the score the more confident it is.  Finally, the
            // reason we do detection in two parts is so you can reuse the intermediate
            // object in many calls to different relation detectors without needing to redo
            // the processing done in extractBinaryRelation().  However, in this example
            // such complications are not necessary.
            if (relDetector.classify(ner.extractBinaryRelation(words, entities.get(i), entities.get(i-1))) > 0)
            {
                printEntity(words, entities.get(i));
                System.out.println("BORN IN");
                printEntity(words, entities.get(i-1));
                System.out.println("");
            }
            // Was entities.get(i-1) born in entities.get(i)?
            if (relDetector.classify(ner.extractBinaryRelation(words, entities.get(i-1), entities.get(i))) > 0)
            {
                printEntity(words, entities.get(i-1));
                System.out.println("BORN IN");
                printEntity(words, entities.get(i));
                System.out.println("");
            }
        }
    }

    public static void printEntity (
        StringVector words,
        EntityMention ent
    )
    {
        // Print all the words in the range indicated by the entity ent.
        for (int i = ent.getStart(); i < ent.getEnd(); ++i)
        {
            System.out.print(words.get(i) + " ");
        }
        System.out.println("");
    }

}

