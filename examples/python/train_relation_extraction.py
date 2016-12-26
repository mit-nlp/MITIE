#!/usr/bin/python
#
#   This example shows how to use the MITIE Python API to train a binary_relation_detector.
#
import sys, os
# Make sure you put the mitielib folder into the python search path.  There are
# a lot of ways to do this, here we do it programmatically with the following
# two statements:
parent = os.path.dirname(os.path.realpath(__file__))
sys.path.append(parent + '/../../mitielib')

from mitie import *


# The training process for a binary relation detector requires a MITIE NER object as
# input.  So we load the saved NER model first.
ner = named_entity_extractor("../../MITIE-models/english/ner_model.dat")

# This object is responsible for doing the training work.  The first argument to the
# constructor is a string that is used to identify the relation detector.  So you
# should put some informative string here.  In this case, we use the name of one of
# the freebase relations.  That is, the "person born-in location" relation.
trainer = binary_relation_detector_trainer("people.person.place_of_birth", ner)

# When you train this kind of algorithm, you need to create a set of training
# examples.  This dataset should include examples of the binary relations you would
# like to detect as well as examples of things that are not what you want to detect.
# To keep this little tutorial simple, we will use just the sentence "Ben Franklin was born in Boston" 
# as training data, but note that for real applications you will likely require
# many thousands of examples to create a high quality relation detector.
#
# So here we create a tokenized version of that sentence.  
sentence = ["Ben", "Franklin", "was", "born", "in", "Boston"]
# Tell the trainer that "Ben Franklin" was born in the location "Boston".  The
# first xrange argument indicates where the person's name is and the second
# xrange indicates the location they were born in.
trainer.add_positive_binary_relation(sentence, xrange(0,2), xrange(5,6))  

# You should also give some negative examples.  Here we give a single negative where
# we keep the same sentence but flip the named entity arguments.  So this is telling
# the trainer that it is not true that Boston was born in Ben Franklin.
trainer.add_negative_binary_relation(sentence, xrange(5,6), xrange(0,2))

# Again, note that you need much more training data than this to make high quality
# relation detectors.  We use just this small amount here to keep the example program
# simple.

# This call runs the actual trainer based on all the training data.  It might take a
# while to run so be patient.
rel_detector = trainer.train()

# Once finished, we can save the relation detector to disk like so.  This will allow 
# you to use a statement like:
#   rel_detector = binary_relation_detector("rel_classifier.svm") 
# to read the detector later on.
rel_detector.save_to_disk("rel_classifier.svm")



# Now let's test it out a little bit.  

# Was Ben Franklin born in Boson?  If the score is > 0 then the
# binary_relation_detector is predicting that he was.  In this case, the number is
# positive so the detector made the right decision.
print ("detection score:", rel_detector(ner.extract_binary_relation(sentence, xrange(0,2), xrange(5,6))))

# Now let's try a different sentence
sentence = ["Jimmy", "Smith", ",", "a", "guy", "raised", "in", "France"]
# Was Jimmy Smith born in France?  Again, the detector correctly gives a number > 0.
print ("detection score:", rel_detector(ner.extract_binary_relation(sentence, xrange(0,2), xrange(7,8))))
# Now let's ask if France was born in Jimmy Smith.  This should be false and happily
# the detector also correctly predicts a number < 0.
print ("detection score:", rel_detector(ner.extract_binary_relation(sentence, xrange(7,8), xrange(0,2))))

