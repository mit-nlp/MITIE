import edu.mit.ll.mitie.*;

public class TrainSeparateDocCategorizerExample {

    public static void main(String[] args) {
        // train models using the separation API
        StringVector sentence = new StringVector();
        sentence.add("I");
        sentence.add("am");
        sentence.add("so");
        sentence.add("happy");
        sentence.add("and");
        sentence.add("exciting");
        sentence.add("to");
        sentence.add("make");
        sentence.add("this");

        StringVector sentence2 = new StringVector();
        sentence2.add("What");
        sentence2.add("a");
        sentence2.add("black");
        sentence2.add("and");
        sentence2.add("bad");
        sentence2.add("day");

        // Now that we have some annotated example sentences we can create the object that does
        // the actual training, the rainer.  The constructor for this object takes a string
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
        TextCategorizerTrainer trainer = new TextCategorizerTrainer(
                "../../MITIE-models/english/total_word_feature_extractor.dat");

        // Don't forget to add the training data.  Here we have only two examples, but for real
        // uses you need to have thousands.
        trainer.add(sentence, "positive");
        trainer.add(sentence2, "negative");

        // The trainer can take advantage of a multi-core CPU.  So set the number of threads
        // equal to the number of processing cores for maximum training speed.
        trainer.setThreadNum(4);
        // This function does the work of training.  Note that it can take a long time to run
        // when using larger training datasets.  So be patient.
        trainer.trainSeparateModels("pure_text_categorizer_model.dat");

        // restore the model using the pure model and extractor
        TextCategorizer categorizer = new TextCategorizer(
                "pure_text_categorizer_model.dat",
                "../../MITIE-models/english/total_word_feature_extractor.dat"
        );

        // Finally, lets test out our new model on an example sentence
        StringVector testSentence = new StringVector();
        testSentence.add("It");
        testSentence.add("is");
        testSentence.add("really");
        testSentence.add("exciting");

        System.out.println("Tags output by this text categorizer model are: ");
        StringVector possibleTags = categorizer.getPossibleNerTags();
        for (int i = 0; i < possibleTags.size(); ++i)
            System.out.println(possibleTags.get(i));

        // Now ask MITIE to detect the type of the text we just loaded.
        SDPair result = categorizer.categorizeDoc(testSentence);
        System.out.println("The type of this text is: " + result.getFirst() + ", with confidence score as " + result.getSecond());
    }
}
