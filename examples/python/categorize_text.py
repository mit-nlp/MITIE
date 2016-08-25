#!/usr/bin/python
#
#    This example shows how to use the MITIE Python API to use a text_categorizer.
#
#
import sys, os


# Make sure you put the mitielib folder into the python search path.  There are
# a lot of ways to do this, here we do it programmatically with the following
# two statements:
parent = os.path.dirname(os.path.realpath(__file__))
sys.path.append(parent + '/../../mitielib')

from mitie import *

test_tokens = ["What","a","black","and","bad","day"]
test_tokens_2 = ["I","am","so","happy"]

# load a pre-trained text categorizer
cat = text_categorizer("new_text_categorizer.dat")

# call the categorizer with a list of tokens, the response is a label (a string)
# and a score (a number) indicating the confidence of the categorizer
label, score = cat(test_tokens)
print(label,score)


label, score = cat(test_tokens_2)
print(label,score)
