#!/usr/bin/python
#
#    This example shows how to use the MITIE Python API to get word features from total_word_feature_extractor
#
#
import sys, os

# Make sure you put the mitielib folder into the python search path.  There are
# a lot of ways to do this, here we do it programmatically with the following
# two statements:
parent = os.path.dirname(os.path.realpath(__file__))
sys.path.append(parent + '/../../mitielib')

from mitie import *

print ("loading Total Word Feature Extractor...")

twfe = total_word_feature_extractor('../../MITIE-models/english/total_word_feature_extractor.dat')

# Get fingerprint of feature dictionary
print ("Fingerprint of feature dictionary", twfe.fingerprint)
print ()
# Get number of dimensions of feature vectors
print ("Number of dimensions of feature vectors", twfe.num_dimensions)
print ()
# Get number of words in the dictionary
print ("Number of words in the dictionary", twfe.num_words_in_dictionary)
print ()
# Get list of words in the dictionary
words=twfe.get_words_in_dictionary()
print ("First 10 words in dictionary", words[0:10])
print ()
# Get features for one word
feats = twfe.get_feature_vector("home")
print ("First 5 features of word 'home'", feats[0:5])
# The total word feature extractor will generate feature vectors for words not
# in its dictionary as well.  It does this by looking at word morphology.  
feats = twfe.get_feature_vector("_word_not_in_dictionary_")
print ("First 5 features of word '_word_not_in_dictionary_'", feats[0:5])
