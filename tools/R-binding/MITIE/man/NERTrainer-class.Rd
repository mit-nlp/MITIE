\name{NERTrainer-class}
\Rdversion{1.1}
\docType{class}
\alias{NERTrainer-class}
\alias{NERTrainer}

\title{Class \code{"NERTrainer"}}
\description{
Tool for training a new \code{\linkS4class{NamedEntityExtractor}} given examples from different entity types.
}
\section{Extends}{

All reference classes extend and inherit methods from \code{"\linkS4class{envRefClass}"}.

}

\examples{
\dontrun{

# Get detailed help for reference class methods

NERTrainer$methods()
NERTrainer$help(add_training_instance)
NERTrainer$help(get_beta)
NERTrainer$help(get_num_threads)
NERTrainer$help(get_size)
NERTrainer$help(set_beta)
NERTrainer$help(set_num_threads)
NERTrainer$help(train)

# When you train a NamedEntityExtractor you need to get a dataset of sentences (or
# sentence or paragraph length chunks of text) where each sentence is annotated with the
# entities you want to find.  For example, if we wanted to find all the names of people and
# organizations then we would need to get a bunch of sentences with examples of person
# names and organizations in them.  Here is an example:
#     My name is Davis King and I work for MIT.
# "Davis King" is a person name and "MIT" is an organization.  
#
# You then give MITIE these example sentences with their entity annotations and it will
# learn to detect them.  That is what we do below.  

# So let's make the first training example.  We use the sentence above.  Note that the
# training API takes tokenized sentences.  It is up to you how you tokenize them, you
# can use the default tokenizer that comes with MITIE or any other method you like.  

tokens <- mitie_tokenize("My name is Davis King and I work for MIT.")
#  [1] "My"    "name"  "is"    "Davis" "King"  "and"   "I"     "work"  "for"  
# [10] "MIT"   "."    

sample <- NERTrainingInstance$new(tokens = tokens)

# Now that we have the tokens stored, we add the entity annotations.  The first
# annotation indicates that the token starting at index 4 and ending at 5 is a
# person.  I.e. "Davis King" is a person name.

sample$add_entity(4, 5, "person")
sample$add_entity(10, 10, "org")

# And we add another training example

tokens <- mitie_tokenize("The other day at work I saw Brian Smith from CMU.")
sample2 <- NERTrainingInstance$new(tokens = tokens)
sample2$add_entity(8, 9, "person")
sample2$add_entity(11, 11, "org")

# Now that we have some annotated example sentences we can create the object that does
# the actual training, the NERtrainer.  The constructor for this object takes a string 
# that should contain the file name for a saved mitie::total_word_feature_extractor.
# The total_word_feature_extractor is MITIE's primary method for analyzing words and
# is created by the tool in the MITIE/tools/wordrep folder.  The wordrep tool analyzes
# a large document corpus, learns important word statistics, and then outputs a
# total_word_feature_extractor that is knowledgeable about a particular language (e.g.
# English).  MITIE comes with a total_word_feature_extractor for English so that is
# what we use here.  But if you need to make your own you do so using a command line 
# statement like:
#    wordrep -e a_folder_containing_only_text_files
# and wordrep will create a total_word_feature_extractor.dat based on the supplied
# text files.  Note that wordrep can take a long time to run or require a lot of RAM
# if a large text dataset is given.  So use a powerful machine and be patient.
# Note: models can be downloaded from http://sourceforge.net/projects/mitie/files/

wordrep_path <- "/path/MITIE-models/english/total_word_feature_extractor.dat"
trainer <- NERTrainer$new(wordrep_path)

# Don't forget to add the training data.  Here we have only two examples, but for real
# uses you need to have thousands.  

trainer$add_training_instance(sample)
trainer$add_training_instance(sample2)

# The trainer can take advantage of a multi-core CPU.  So set the number of threads
# equal to the number of processing cores for maximum training speed.

trainer$set_num_threads(2)

# This function does the work of training.  Note that it can take a long time to run
# when using larger training datasets.  So be patient.
# The train() method returns a object of the reference class NamedEntityExtractor.

ner <- trainer$train()

# Now that training is done we can save the ner object to disk like so.  This will
# allow you to load the model back in using a statement like:
#   ner <- NamedEntityExtractor$("new_ner_model.dat").

ner$save_to_disk("new_ner_model.dat")

# But now let's try out the ner object.  It was only trained on a small dataset but it
# has still learned a little.  So let's give it a whirl.  But first, print a list of
# possible tags.  In this case, it is just "person" and "org".

tag_names <- ner$get_possible_ner_tags()
tag_names
# [1] "person" "org"  

# Now let's make up a test sentence and ask the ner object to find the entities.

tokens <- mitie_tokenize("I met with John Becker at HBU.")
entities <- ner$extract_entities(tokens)

# Happily, it found the correct answers, "John Becker" and "HBU" in this case which we
# print out below.

for (i in 1:length(entities)) {
    entity = entities[[i]]
    position = paste("(", entity$start, ",", entity$end, ")", sep="")
    text = paste(tokens[entity$start:entity$end], collapse=" ")
    print(paste(text, "/", tag_names[entity$tag], "@", position))
}
}
}
\keyword{classes}
\section{Fields}{
  \describe{
    \item{\code{.trainer}:}{Object of class \code{externalptr} pointer to ner trainer C++ object. }
  }
}
\section{Methods}{
  \describe{
    \item{\code{add_training_instance(instance)}:}{ Adds the given \code{\linkS4class{NERTrainingInstance}} object into this object. }
    \item{\code{get_beta()}:}{ Returns value of beta parameter. }
    \item{\code{get_num_threads()}:}{ Returns number of threads that will be used to perform training. }
    \item{\code{get_size()}:}{ Returns number of training instances that have been added to this object. }
    \item{\code{set_beta(beta)}:}{ Sets value of parameter that controls trade-off between trying to avoid false alarms but also detecting everything. }
    \item{\code{set_num_threads(num_threads)}:}{ Sets number of threads that will be used for training. }
    \item{\code{train()}:}{ Trains a named entity extractor based on the training instances added with \code{add_training_instance()}. }
%%  \item{\code{initialize(wordrep_filename, ...)}:}{ Construct new \code{NERTrainer} object that will use specified word represention model file. }
  }
}
