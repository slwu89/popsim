#' Bellman-Harris Branching Process (G~Gamma, H~Negative Binomial)
#'
#' Designed to simulate the early phases of an epidemic, before density-dependence kicks in.
#' It tries to be fast.
#'
#' @examples
#' set.seed(42)
#' R0 <- 2.6
#' k <- 0.16
#' mu <- 8.4
#' stdev <- 3.8
#' shape <- (mu/stdev)^2
#' scale <- (stdev^2)/mu
#' index_cases <- 40
#' max_cases <- 5e4
#' max_time <- 90
#' times <- bhbp(R0,k,shape,scale,index_cases,max_cases,max_time)
#' @useDynLib popsim bhbp_C
#' @export
bhbp <- function(R0,k,shape,scale,index_cases,max_cases,max_time){
  .Call(bhbp_C,as.numeric(R0),as.numeric(k),as.numeric(shape),as.numeric(scale),as.integer(index_cases),as.integer(max_cases),as.numeric(max_time))
}
