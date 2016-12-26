#!/usr/bin/python
#
#    This example shows how to use the MITIE Python API to train a
#    text_categorizer that produces smaller model files.  However, these
#    smaller files are dependent on some feature extractor file which must be
#    provided when the model is loaded, as you will see in this example.
#
#    For a comparison of what a non-pure version of the API looks like you can
#    read the categorize_text.py and train_text_categorizer.py examples. 
#
#
import sys, os

# Make sure you put the mitielib folder into the python search path.  There are
# a lot of ways to do this, here we do it programmatically with the following
# two statements:
parent = os.path.dirname(os.path.realpath(__file__))
sys.path.append(parent + '/../../mitielib')

from mitie import *

fe_filename= "../../MITIE-models/english/total_word_feature_extractor.dat"
trainer = text_categorizer_trainer(fe_filename)

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

# Now that training is done we can save the categorizer object to disk like so. 
# In pure_model mode we do not include a copy of the feature extractor.
cat.save_to_disk("new_text_categorizer_pure_model.dat",pure_model=True)


# Now to load load the 'pure model' from disk we also pass the feature extractor filename
cat2 = text_categorizer("new_text_categorizer_pure_model.dat",fe_filename)

text1 = "I am so happy"
pred, conf = cat2(tokenize(text1))
print ("predict sentiment of text '{0}' to be {1} with confidence {2}".format(text1,pred,conf))
