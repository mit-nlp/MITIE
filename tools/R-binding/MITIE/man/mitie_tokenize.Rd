\name{mitie_tokenize}
\alias{mitie_tokenize}
\title{Tokenize text according to CoNLL 2003 conventions.}
\description{\code{mitie_tokenize} tokenizes text in the same way the CoNLL 2003 dataset was tokenized.}
\usage{
mitie_tokenize(text)
}
\arguments{
  \item{text}{string to tokenize.}
}
\value{vector of strings, one element per non-whitespace token.}
\examples{
\dontrun{

text <- mitie_load_entire_file("article.txt")
tokens <- mitie_tokenize(text)
}
}
