#!/usr/bin/python
import sys, os

# Make sure you put the mitielib folder into the python search path.  There are a lot of
# ways to do this, here we do it programmatically with the following two statements:
parent = os.path.dirname(os.path.realpath(__file__))
sys.path.append(parent + '/../../mitielib')

from mitie import *

print "loading NER model..."
ner = named_entity_extractor('../../MITIE-models/ner_model.dat')
print "\nTags output by this NER model:", ner.get_possible_ner_tags()

# Load a text file and convert it into a list of words.  
tokens = tokenize(load_entire_file('../../sample_text.txt'))
print "Tokenized input:", tokens


entities = ner.extract_entities(tokens)
print "\nEntities found:", entities
print "\nNumber of entities detected:", len(entities)

# entities is a list of tuples, each containing the entity tag and an xrange that indicates
# which tokens are part of the entity.  The entities are also listed in the order they
# appear in the input text file.  Here we just print the text and tag for each entity to
# the screen.
for e in entities:
    range = e[0]
    tag = e[1]
    sys.stdout.write("   " + tag + ": ")
    for i in range:
        sys.stdout.write(tokens[i] + " ")
    print ""

