#!/usr/bin/python
#
#    This example shows that a feature extractor can be shared between two
#    text categorizers
#
import sys
import os

# Make sure you put the mitielib folder into the python search path.  There are
# a lot of ways to do this, here we do it programmatically with the following
# two statements:
parent = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, parent + '/../../mitielib')

from mitie import *

def run_example():

    fe_filename= "../../MITIE-models/english/total_word_feature_extractor.dat"
    trainer1 = text_categorizer_trainer(fe_filename)

    # Don't forget to add the training data.  Here we have only two examples, but for real
    # uses you need to have thousands. You could also pass whole sentences in to the tokenize() function
    # to get the tokens.
    trainer1.add_labeled_text(["I","am","so","happy","and","exciting","to","make","this"],"positive")
    trainer1.add_labeled_text(["What","a","black","and","bad","day"],"negative")

    # The trainer can take advantage of a multi-core CPU.  So set the number of threads
    # equal to the number of processing cores for maximum training speed.
    trainer1.num_threads = 4

    # This function does the work of training.  Note that it can take a long time to run
    # when using larger training datasets.  So be patient.
    cat1 = trainer1.train()

    # Now that training is done we can save the categorizer object to disk like so.
    # In pure_model mode we do not include a copy of the feature extractor.
    cat1.save_to_disk("new_text_categorizer_1_pure_model.dat",pure_model=True)

    # Now train another categorizer and save it as pure model
    trainer2 = text_categorizer_trainer(fe_filename)
    trainer2.add_labeled_text(tokenize("Recharge my phone"),"positive")
    trainer2.add_labeled_text(tokenize("Recharge my number"),"positive")
    trainer2.add_labeled_text(tokenize("I want to recharge my phone"),"positive")
    trainer2.add_labeled_text(tokenize("Cancel"),"negative")
    trainer2.add_labeled_text(tokenize("Finish"),"negative")
    trainer2.add_labeled_text(tokenize("Close"),"negative")
    trainer2.num_threads = 4
    cat2 = trainer2.train()
    cat2.save_to_disk("new_text_categorizer_2_pure_model.dat",pure_model=True)

    # Load a feature extractor which will be shared between two categorizers
    text_feature_extractor = total_word_feature_extractor(fe_filename)

    # Load the two pure-model categorizers
    cat1_new = text_categorizer("new_text_categorizer_1_pure_model.dat")
    cat2_new = text_categorizer("new_text_categorizer_2_pure_model.dat")

    # Now use the feature extractor to categorize the text.
    # Observe that we are passing the same feature extractor to both the categorizers

    text1 = "I am so happy"
    pred, conf = cat1_new(tokenize(text1), text_feature_extractor)
    print ("predict sentiment of text '{0}' to be {1} with confidence {2}".format(text1, pred, conf))

    text2 = "Can you recharge my phone?"
    pred2, conf2 = cat2_new(tokenize(text2), text_feature_extractor)
    print ("predict sentiment of text '{0}' to be {1} with confidence {2}".format(text2, pred2, conf2))

    text3 = "Stop"
    pred3, conf3 = cat2_new(tokenize(text3), text_feature_extractor)
    print ("predict sentiment of text '{0}' to be {1} with confidence {2}".format(text3, pred3, conf3))


if __name__ == '__main__':
    run_example()

