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
        nerTrainingInstance.addEntity(3,2, "person");
        nerTrainingInstance.addEntity(9,1, "org");

        NerTrainer nerTrainer = new NerTrainer("total_word_feature_extractor.dat");
        nerTrainer.add(nerTrainingInstance.getImpl());
        //nerTrainer.setThreadNum(4);

        //nerTrainer.train("new_ner_model.dat");

        System.out.println(nerTrainer.getSize());

    }
}


