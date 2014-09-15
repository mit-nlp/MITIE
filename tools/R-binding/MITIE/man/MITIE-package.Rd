\name{MITIE-package}
\alias{MITIE-package}
\alias{MITIE}
\docType{package}
\title{
MIT Information Extraction (MITIE) Interface
}
\description{
The MITIE project provides free (even for commercial use) state-of-the-art information extraction tools. 
  The current release includes tools for performing named entity extraction and binary relation detection,
  as well as tools for training custom extractors and relation detectors. 
}
\details{
\tabular{ll}{
Package: \tab MITIE\cr
Type: \tab Package\cr
Version: \tab 0.2\cr
Date: \tab 2014-07-10\cr
License: \tab Boost Software License - Version 1.0 - August 17th, 2003\cr
URL: \tab https://github.com/mit-nlp/MITIE\cr
Imports: \tab Rcpp (>= 0.11.0)\cr
LinkingTo: \tab Rcpp\cr
}

\code{\link{mitie_tokenize}} tokenizes text prior to extraction.

\code{\link{mitie_load_entire_file}} loads text from file into a string.

\code{\link{mitie_range_overlaps}} detects whether two entity ranges overlap.

\code{\linkS4class{NamedEntityExtractor}} is a reference class for extracting named entities such as people, locations, and organizations from text.

\code{\linkS4class{BinaryRelationDetector}} is a reference class for detecting relationships between entities such as person / place_of_birth.

\code{\linkS4class{NERTrainer}} is a reference class for training a new \code{\linkS4class{NamedEntityExtractor}} given examples.

\code{\linkS4class{NERTrainingInstance}} is a reference class for creating examples for \code{\linkS4class{NERTrainer}}.

\code{\linkS4class{BinaryRelationDetectorTrainer}} is a reference class for training a new \code{\linkS4class{BinaryRelationDetector}} given positive and negative examples.

}
\author{
Davis King, Michael Yee, Wade Shen

Maintainer: Michael Yee <myee@ll.mit.edu>
}
\keyword{ package }
\examples{
\dontrun{
# Get package overview

help(MITIE)

# Get help for functions

help(mitie_load_entire_file)
help(mitie_tokenize)

# Get help for reference classes, e.g., NamedEntityExtractor

help(NamedEntityExtractor)
NamedEntityExtractor$methods()
NamedEntityExtractor$help("save_to_disk")
}
}
