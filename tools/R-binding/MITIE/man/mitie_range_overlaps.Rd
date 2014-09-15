\name{mitie_range_overlaps}
\alias{mitie_range_overlaps}
\title{Detect whether two entity ranges overlap.}
\description{\code{mitie_range_overlaps} detects whether two entity ranges overlap.}
\usage{
mitie_range_overlaps(arg1, arg2)
}
\arguments{
  \item{arg1}{named list containing members ``start'' (index of argument 1's first token) and ``end'' (index of argument 1's last token)}
  \item{arg2}{named list containing members ``start'' (index of argument 2's first token) and ``end'' (index of argument 2's last token)}
}
\value{TRUE if arg1 and arg2 overlap (where start and end are inclusive indices); FALSE otherwise}
\examples{
\dontrun{

arg1 <- list(start = 2, end = 4)
arg2 <- list(start = 5, end = 7)
mitie_range_overlaps(arg1, arg2)
# FALSE

arg1 <- list(start = 2, end = 3)
arg2 <- list(start = 3, end = 5)
mitie_range_overlaps(arg1, arg2)
# TRUE
}
}
