\name{NamedEntityExtractor-class}
\Rdversion{1.1}
\docType{class}
\alias{NamedEntityExtractor-class}
\alias{NamedEntityExtractor}

\title{Class \code{"NamedEntityExtractor"}}
\description{
Tool for extracting named entities such as people, locations, and organizations from text.
}
\section{Extends}{

All reference classes extend and inherit methods from \code{"\linkS4class{envRefClass}"}.

}

\examples{
\dontrun{

# Get detailed help for reference class methods

NamedEntityExtractor$methods()
NamedEntityExtractor$help(create_binary_relation)
NamedEntityExtractor$help(extract_entities)
NamedEntityExtractor$help(get_possible_ner_tags)
NamedEntityExtractor$help(save_to_disk)

# Load named entity extractor from disk
# Note: models can be downloaded from http://sourceforge.net/projects/mitie/files/binaries
# - e.g., http://sourceforge.net/projects/mitie/files/binaries/MITIE-models-v0.2.tar.bz2

ner_model_path <- "/path/MITIE-models/english/ner_model.dat"
ner <- NamedEntityExtractor$new(ner_model_path)

# Print out what kind of tags this tagger can predict

tag_names <- ner$get_possible_ner_tags()
tag_names
#  [1] "PERSON"       "LOCATION"     "ORGANIZATION" "MISC"

# Prepare some data

tokens <- mitie_tokenize("Bill Gates was born in Seattle, Washington. Bill used to be the CEO of Microsoft.")
tokens
#  [1] "Bill"       "Gates"      "was"        "born"       "in"        
#  [6] "Seattle"    ","          "Washington" "."          "Bill"      
# [11] "used"       "to"         "be"         "the"        "CEO"       
# [16] "of"         "Microsoft"  "."  

# Extract entities

entities <- ner$extract_entities(tokens)
for (i in 1:length(entities)) {
    entity = entities[[i]]
    position = paste("(", entity$start, ",", entity$end, ")", sep="")
    text = paste(tokens[entity$start:entity$end], collapse=" ")
    print(paste(text, "/", tag_names[entity$tag], "@", position))
}
# [1] "Bill Gates / PERSON @ (1,2)"
# [1] "Seattle / LOCATION @ (6,6)"
# [1] "Washington / LOCATION @ (8,8)"
# [1] "Bill / PERSON @ (10,10)"
# [1] "Microsoft / ORGANIZATION @ (17,17)"
}
}
\keyword{classes}
\section{Fields}{
  \describe{
    \item{\code{.ner}:}{Object of class \code{externalptr} pointer to named entity extractor C++ object. }
  }
}
\section{Methods}{
  \describe{
    \item{\code{create_binary_relation(tokens, arg1, arg2)}:}{ Converts a raw relation mention pair into an object that can be scored by a \code{\linkS4class{BinaryRelationDetectorTrainer}}. }
    \item{\code{extract_entities(tokens)}:}{ Extract named entities from text. }
    \item{\code{get_possible_ner_tags()}:}{ Returns character vector of tag labels that this extractor can recognize. }
    \item{\code{save_to_disk(filename)}:}{ Saves entity extractor object to disk. }
%%    \item{\code{initialize(filename, ...)}:}{ Construct new \code{NamedEntityExtractor} object from saved model (using \code{NamedEntityExtractor$new(filename)}). }
  }
}
