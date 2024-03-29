% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/dtree.R
\name{update}
\alias{update}
\alias{update.dirichlet_tree}
\title{Update a \code{dirichlet_tree} model by observing some ranked ballots.}
\usage{
\method{update}{dirichlet_tree}(object, ballots, ...)
}
\arguments{
\item{object}{A \code{dirichlet_tree} object.}

\item{ballots}{A set of ballots - must be of type \code{prefio::preferences}.}

\item{\\dots}{Unused.}
}
\value{
The \code{dirichlet_tree} object.
}
\description{
\code{update} updates a Dirichlet-tree model with observations to obtain
a posterior distribution on the ranked ballots. See
\insertCite{dtree_evoteid;textual}{elections.dtree} for implementation
details.
}
\references{
\insertRef{dtree_eis}{elections.dtree}.

\insertRef{dtree_evoteid}{elections.dtree}.
}
