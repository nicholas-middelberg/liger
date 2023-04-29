#define ARMA_64BIT_WORD 1
#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::plugins(cpp11)]]

using namespace Rcpp;
using namespace arma;

// [[Rcpp::export]]
arma::sp_mat scaleNotCenterFast(arma::sp_mat x) {
  int nrow = x.n_rows, ncol = x.n_cols;

  NumericVector sum_of_squares(ncol);
  for(arma::sp_mat::const_iterator i = x.begin(); i != x.end(); ++i)
  {
    sum_of_squares(i.col()) += (*i)*(*i);
  }
  for (int i = 0; i < ncol; ++i)
  {
    sum_of_squares(i) = sqrt(sum_of_squares(i)/(nrow-1));
  }
  for(arma::sp_mat::iterator i = x.begin(); i != x.end(); ++i)
  {
    *i /= sum_of_squares(i.col());
  }
  return x;
}

// [[Rcpp::export]]
NumericVector rowMeansFast(arma::sp_mat x) {
  int nrow = x.n_rows, ncol = x.n_cols;

  NumericVector means(nrow);
  for (int i = 0; i < nrow; ++i)
  {
    means(i) = 0;
  }
  for(arma::sp_mat::const_iterator i = x.begin(); i != x.end(); ++i)
  {
    means(i.row()) += *i;
  }
  for (int i = 0; i < nrow; ++i)
  {
    means(i) /= ncol;
  }
  return means;
}

// [[Rcpp::export]]
NumericVector rowVarsFast(arma::sp_mat x, NumericVector means) {
  int nrow = x.n_rows, ncol = x.n_cols;

  NumericVector vars(nrow);
  NumericVector nonzero_vals(nrow);

  for(arma::sp_mat::const_iterator i = x.begin(); i != x.end(); ++i)
  {
    vars(i.row()) += (*i-means(i.row()))*(*i-means(i.row()));
    nonzero_vals(i.row()) += 1;
  }
  // Add back square mean error for zero elements
  // const_iterator only loops over nonzero elements
  for (int i = 0; i < nrow; ++i)
  {
    vars(i) += (ncol - nonzero_vals(i))*(means(i)*means(i));
    vars(i) /= (ncol-1);
  }
  return vars;
}

// [[Rcpp::export]]
NumericVector sumSquaredDeviations(arma::sp_mat x, NumericVector means) {
  int nrow = x.n_rows, ncol = x.n_cols;

  NumericVector ssd(nrow);
  NumericVector nonzero_vals(nrow);

  for(arma::sp_mat::const_iterator i = x.begin(); i != x.end(); ++i)
  {
    ssd(i.row()) += (*i-means(i.row()))*(*i-means(i.row()));
    nonzero_vals(i.row()) += 1;
  }
  // Add back square mean error for zero elements
  // const_iterator only loops over nonzero elements
  for (int i = 0; i < nrow; ++i)
  {
    ssd(i) += (ncol - nonzero_vals(i))*(means(i)*means(i));
  }
  return ssd;
}

//  [[Rcpp::export]]
NumericMatrix denseZScore(NumericMatrix & x, NumericVector m){
  int nrow = x.nrow(), ncol = x.ncol();
  NumericVector v(nrow);
  NumericVector nz(nrow);
  NumericMatrix Z = clone(x);
  NumericVector r(ncol);
  for(int i = 0; i < nrow; i++){
    r = Z(i, _);
    for(NumericVector::iterator j = r.begin(); j != r.end(); ++j){
      v(i) += (*j - m(i)) * (*j - m(i));
    }
    v(i) /= ncol - 1;
    v(i) = sqrt(v(i));
    for (NumericVector::iterator j = r.begin(); j != r.end(); ++j) {
      *j -= m(i);
      *j /= v(i);
    }
  }
  return Z;
}

//  [[Rcpp::export]]
NumericVector rowVarsDense(arma::mat x, arma::vec m) {
  int nrow = x.n_rows;
  int ncol = x.n_cols;
  NumericVector out(nrow);
  for (int i = 0; i < nrow; ++i) {
    arma::rowvec row_i = x.row(i);
    arma::vec diff = row_i.t() - m[i];
    out(i) = arma::accu(arma::square(diff)) / (ncol - 1);
  }
  return out;
}

/* standardize matrix rows using given mean and standard deviation,
 clip values larger than vmax to vmax,
 then return variance for each row */
// [[Rcpp::export(rng = false)]]
NumericVector SparseRowVarStd(arma::sp_mat x,
                              NumericVector mu,
                              NumericVector sd,
                              double vmax){
  x = x.t();
  NumericVector allVars(x.n_cols);
  NumericVector colSum(x.n_cols);
  NumericVector nNonZero(x.n_cols);
  for (arma::sp_mat::const_iterator i = x.begin(); i != x.end(); ++i) {
    int k = i.col();
    if (sd[k] == 0) continue;
    nNonZero[k] += 1;
    colSum[k] += pow(std::min(vmax, (*i - mu[k]) / sd[k]), 2);
  }
  for (int k=0; k<x.n_cols; ++k) {
    allVars[k] = (colSum[k] + pow((0 - mu[k]) / sd[k], 2) * (x.n_rows - nNonZero[k])) / (x.n_rows - 1);
  }
  return(allVars);
}
