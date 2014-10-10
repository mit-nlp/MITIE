import edu.mit.ll.mitie.*;

public class ner 
{
    public static void print_entity (
        StringVector words,
        EntityMention ent
    )
    {
        for (int i = ent.getStart(); i < ent.getEnd(); ++i)
        {
            System.out.print(words.get(i) + " ");
        }
        System.out.println("");
    }

    public static void main(String args[])
    {
        System.loadLibrary("javamitie");
        

        StringVector words = mitie.tokenize(mitie.loadEntireFile(args[0]));

        NamedEntityExtractor ner = new NamedEntityExtractor(args[1]);

        EntityMentionVector ents = ner.extractEntities(words);
        System.out.println("number of entities " + ents.size());

        for (int i = 0; i < ents.size(); ++i)
        {
            System.out.println("entity " + ner.getPossibleNerTags().get(ents.get(i).getTag()));
            print_entity(words, ents.get(i));
        }


        BinaryRelationDetector rel_detector = new BinaryRelationDetector(args[2]);
        System.out.println("relation detector type " + rel_detector.getNameString());

        for (int i = 1; i < ents.size(); ++i)
        {
            if (rel_detector.classify(ner.extractBinaryRelation(words, ents.get(i), ents.get(i-1))) > 0)
            {
                print_entity(words, ents.get(i));
                print_entity(words, ents.get(i-1));
                System.out.println("");
            }
            if (rel_detector.classify(ner.extractBinaryRelation(words, ents.get(i-1), ents.get(i))) > 0)
            {
                print_entity(words, ents.get(i-1));
                print_entity(words, ents.get(i));
                System.out.println("");
            }
        }

    }
}

