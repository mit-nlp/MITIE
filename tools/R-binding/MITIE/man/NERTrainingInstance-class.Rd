\name{NERTrainingInstance-class}
\Rdversion{1.1}
\docType{class}
\alias{NERTrainingInstance-class}
\alias{NERTrainingInstance}

\title{Class \code{"NERTrainingInstance"}}
\description{
Reference class for creating training examples for \code{\linkS4class{NERTrainer}}.
}
\section{Extends}{

All reference classes extend and inherit methods from \code{"\linkS4class{envRefClass}"}.

}

\examples{
\dontrun{

# The NERTrainer requires a dataset of sentences (or sentence or paragraph length 
# chunks of text) where each sentence is annotated with the entities you want to find.  
# Note that the training API takes tokenized sentences.  It is up to you how you 
# tokenize them, you can use the default tokenizer that comes with MITIE or any 
# other method you like.  

tokens <- mitie_tokenize("My name is Davis King and I work for MIT.")
#  [1] "My"    "name"  "is"    "Davis" "King"  "and"   "I"     "work"  "for"  
# [10] "MIT"   "."    

sample <- NERTrainingInstance$new(tokens = tokens)

# Now that we have the tokens stored, we add the entity annotations.  The first
# annotation indicates that the token starting at index 4 and ending at 5 is a
# person.  I.e. "Davis King" is a person name.

sample$add_entity(4, 5, "person")
sample$add_entity(10, 10, "org")

# See NERTrainer documentation for examples of how to train NamedEntityExtractor
# objects using annotated examples like the one created above.
}
}
\keyword{classes}
\section{Fields}{
  \describe{
    \item{\code{tokens}:}{Object of class \code{character} character vector containing words from a piece of text (typically created with \code{\link{mitie_tokenize}}). }
    \item{\code{.entities}:}{Object of class \code{list} list of entities that have been added to this training instance. }
  }
}
\section{Methods}{
  \describe{
    \item{\code{add_entity(start, end, tag)}:}{ Adds the given entity to this training instance. }
%%  \item{\code{initialize(...)}:}{ ~~ }
  }
}
