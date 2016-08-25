#!/usr/bin/python
#
#    This example shows how to use the MITIE Python API to train a named_entity_extractor.
#
#
import sys, os
import ipdb
# Make sure you put the mitielib folder into the python search path.  There are
# a lot of ways to do this, here we do it programmatically with the following
# two statements:
#parent = os.path.dirname(os.path.realpath(__file__))
#sys.path.append(parent + '/../../mitielib')

from mitie import *

test_tokens = ["What","a","black","and","bad","day"]
test_tokens_2 = ["I","am","so","happy"]

# This function does the work of training.  Note that it can take a long time to run
# when using larger training datasets.  So be patient.
cat = text_categorizer("new_text_categorizer.dat")

label, score = cat(test_tokens)
print(label,score)


label, score = cat(test_tokens_2)
print(label,score)
