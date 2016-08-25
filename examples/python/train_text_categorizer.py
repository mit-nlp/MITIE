#!/usr/bin/python
#
#    This example shows how to use the MITIE Python API to train a text_categorizer.
#
#
import sys, os

# Make sure you put the mitielib folder into the python search path.  There are
# a lot of ways to do this, here we do it programmatically with the following
# two statements:
parent = os.path.dirname(os.path.realpath(__file__))
sys.path.append(parent + '/../../mitielib')

from mitie import *


trainer = text_categorizer_trainer("../../MITIE-models/english/total_word_feature_extractor.dat")

# Don't forget to add the training data.  Here we have only two examples, but for real
# uses you need to have thousands. You could also pass whole sentences in to the tokenize() function
# to get the tokens.
trainer.add_labeled_text(["I","am","so","happy","and","exciting","to","make","this"],"positive")
trainer.add_labeled_text(["What","a","black","and","bad","day"],"negative")

# The trainer can take advantage of a multi-core CPU.  So set the number of threads
# equal to the number of processing cores for maximum training speed.
trainer.num_threads = 4


# This function does the work of training.  Note that it can take a long time to run
# when using larger training datasets.  So be patient.
cat = trainer.train()

# Now that training is done we can save the categorizer object to disk like so.  This will
# allow you to load the model back in using a statement like:
#   cat = text_categorizer("new_text_categorizer.dat").
cat.save_to_disk("new_text_categorizer.dat")

