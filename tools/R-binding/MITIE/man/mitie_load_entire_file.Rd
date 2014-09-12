\name{mitie_load_entire_file}
\alias{mitie_load_entire_file}
\title{Loads text from file into a string.}
\description{\code{mitie_load_entire_file} reads a file from disk and returns the contents as a string.}
\usage{
mitie_load_entire_file(filename)
}
\arguments{
  \item{filename}{name of file to load from disk.}
}
\value{string containing contents of file.}
\examples{
\dontrun{

text <- mitie_load_entire_file("article.txt")
}
}
