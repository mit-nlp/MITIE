\name{BinaryRelationDetectorTrainer-class}
\Rdversion{1.1}
\docType{class}
\alias{BinaryRelationDetectorTrainer-class}
\alias{BinaryRelationDetectorTrainer}

\title{Class \code{"BinaryRelationDetectorTrainer"}}
\description{
Tool for training a new \code{\linkS4class{BinaryRelationDetector}} given positive and negative examples.
}
\section{Extends}{

All reference classes extend and inherit methods from \code{"\linkS4class{envRefClass}"}.

}

\examples{
\dontrun{

# Get detailed help for reference class methods

BinaryRelationDetectorTrainer$methods()
BinaryRelationDetectorTrainer$help(add_negative_binary_relation)
BinaryRelationDetectorTrainer$help(add_positive_binary_relation)
BinaryRelationDetectorTrainer$help(get_beta)
BinaryRelationDetectorTrainer$help(get_num_negative_examples)
BinaryRelationDetectorTrainer$help(get_num_positive_examples)
BinaryRelationDetectorTrainer$help(get_num_threads)
BinaryRelationDetectorTrainer$help(get_relation_name)
BinaryRelationDetectorTrainer$help(set_beta)
BinaryRelationDetectorTrainer$help(set_num_threads)
BinaryRelationDetectorTrainer$help(train)

# The training process for a binary relation detector requires a MITIE NER object as
# input.  So we load the saved NER model first.
# Note: models can be downloaded from http://sourceforge.net/projects/mitie/files/

ner_model_path <- "/path/MITIE-models/english/ner_model.dat"
ner <- NamedEntityExtractor$new(ner_model_path)

# This object is responsible for doing the training work.  The first argument to the
# constructor is a string that is used to identify the relation detector.  So you
# should put some informative string here.  In this case, we use the name of one of
# the freebase relations.  That is, the "person born-in location" relation.

trainer <- BinaryRelationDetectorTrainer$new("people.person.place_of_birth", ner)

# When you train this kind of algorithm, you need to create a set of training
# examples.  This dataset should include examples of the binary relations you would
# like to detect as well as examples of things that are not what you want to detect.
# To keep this little tutorial simple, we will use just the sentence "Ben Franklin was born in Boston" 
# as training data, but note that for real applications you will likely require
# many thousands of examples to create a high quality relation detector.
#
# So here we create a tokenized version of that sentence.  

sentence <- mitie_tokenize("Ben Franklin was born in Boston")

# Tell the trainer that "Ben Franklin" was born in the location "Boston".  The
# first argument indicates where the person's name is and the second
# indicates the location they were born in.

arg1 <- list(start = 1, end = 2)
arg2 <- list(start = 6, end = 6)
trainer$add_positive_binary_relation(sentence, arg1, arg2)

# You should also give some negative examples.  Here we give a single negative where
# we keep the same sentence but flip the named entity arguments.  So this is telling
# the trainer that it is not true that Boston was born in Ben Franklin.

trainer$add_negative_binary_relation(sentence, arg2, arg1)

# Again, note that you need much more training data than this to make high quality
# relation detectors.  We use just this small amount here to keep the example program
# simple.

# This call runs the actual trainer based on all the training data.  It might take a
# while to run so be patient.  This creates an object of type BinaryRelationDetector.

rel_detector <- trainer$train()

# Once finished, we can save the relation detector to disk like so.  This will allow 
# you to use a statement like:
#   rel_detector <- BinaryRelationDetector$new("rel_classifier.svm") 
# to read the detector later on.

rel_detector$save_to_disk("rel_classifier.svm")

# Now let's test it out a little bit.  

# Was Ben Franklin born in Boston?  If the score is > 0 then the
# bBinaryRelationDetector is predicting that he was.  In this case, the number is
# positive so the detector made the right decision.

rel_detector$score(ner$create_binary_relation(sentence, arg1, arg2))

# Now let's try a different sentence

sentence <- mitie_tokenize("Jimmy Smith, a guy raised in France")

# Was Jimmy Smith born in France?  Again, the detector correctly gives a number > 0.

arg1 <- list(start = 1, end = 2)
arg2 <- list(start = 8, end = 8)
rel_detector$score(ner$create_binary_relation(sentence, arg1, arg2))

# Now let's ask if France was born in Jimmy Smith.  This should be false and happily
# the detector also correctly predicts a number < 0.

rel_detector$score(ner$create_binary_relation(sentence, arg2, arg1))
}
}
\keyword{classes}
\section{Fields}{
  \describe{
    \item{\code{.trainer}:}{Object of class \code{externalptr} pointer to binary relation detector trainer C++ object. }
  }
}
\section{Methods}{
  \describe{
    \item{\code{add_negative_binary_relation(tokens, arg1, arg2)}:}{ Adds a negative training instance into the trainer. }
    \item{\code{add_positive_binary_relation(tokens, arg1, arg2)}:}{ Adds a positive training instance into the trainer. }
    \item{\code{get_beta()}:}{ Returns value of beta parameter. }
    \item{\code{get_num_negative_examples()}:}{ Returns number of negative examples added with add_negative_binary_relation(). }
    \item{\code{get_num_positive_examples()}:}{ Returns number of positive examples added with add_positive_binary_relation(). }
    \item{\code{get_num_threads()}:}{ Returns number of threads that will be used to perform training. }
    \item{\code{get_relation_name()}:}{ Returns type of relation that this object will classify. }
    \item{\code{set_beta(beta)}:}{ Sets value of parameter that controls trade-off between trying to avoid false alarms but also detecting everything. }
    \item{\code{set_num_threads(num_threads)}:}{ Sets number of threads that will be used for training. }
    \item{\code{train()}:}{ Trains a binary relation detector based on the postive and negative training instances. }
%%    \item{\code{initialize(relation_name, ner, ...)}:}{ ~~ }
  }
}
