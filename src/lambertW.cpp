/* lambertW.cpp

 Copyright (C) 2015, Avraham Adler
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

References:

Corless, R. M.; Gonnet, G. H.; Hare, D. E.; Jeffrey, D. J. & Knuth, D. E. "On the Lambert W function", Advances in Computational Mathematics,
            Springer, 1996, 5, 329-359
*/

#include <Rcpp.h>
#include <math.h>

using namespace Rcpp;

const double EPS = 2.2204460492503131e-16;
const double M_1_E = 1.0 / M_E;

/* Halley Iteration
  Given x, we want to find W such that Wexp(W) = x, so Wexp(W) - x = 0.
  We can use Halley iteration to find this root; to do so it needs first and second derivative.
  f(W)    = W * exp(W) - x
  f'(W)   = W * exp(W) + exp(W)       = exp(W) * (W + 1)
  f''(W)  = exp(W) + (W + 1) * exp(W) = exp(W) * (W + 2)
  Halley Step:
   W_{n+1} = W_n - {2 * f(W_n) * f'(W_n)} / {2 * [f'(W_n)]^2 - f(W_n) * f''(W_n)}
*/

double HalleyIter(double x, double w_guess){
  double w = w_guess;
  int MaxEval = 16;
  bool CONVERGED = false;
  int i = 0;
  do {
    double ew = exp(w);
    double w1 = w + 1.0;
    double f0 = w * ew - x;
    f0 /= ((ew * w1) - (((w1 + 1.0) * f0) / (2 * w1))); /* Corliss et al. 5.9 */
    CONVERGED = fabs(f0) <= EPS;
    w -= f0;
     ++i;
  	} while (!CONVERGED && i < MaxEval);
  return(w);
}

// [[Rcpp::export]]
NumericVector lambertW0_C(NumericVector x) {
  int n = x.size();
  NumericVector result(n);
  double w;
  for (int i = 0; i < n; ++i) {
    if (x(i) == std::numeric_limits<double>::infinity()) {
      result(i) = std::numeric_limits<double>::infinity();
    } else if (x(i) < -M_1_E) {
      result(i) = std::numeric_limits<double>::quiet_NaN();
    } else if (fabs(x(i) + M_1_E) < 4 * EPS) {
      result(i) = -1.0;
    } else if (x(i) <= M_E - 0.5) {
/* Use expansion in Corliss 4.22 to create (3, 2) Pade approximant
   Numerator: -10189 / 303840 * p ^ 3 + 40529 / 303840 * p ^ 2 + 489 / 844 * p - 1
   Denominator: -14009 / 303840 * p^2 + 355 / 844 * p + 1
   Converted to digits to reduce needed operations
*/
      double p = sqrt(2 * (M_E * x(i) + 1));
      double Numer = ((-0.03353409689310163 * p + 0.1333892838335966) * p + 0.5793838862559242) * p - 1;
      double Denom = (-0.04610650342285413 * p + 0.4206161137440758) * p + 1;
      w = Numer / Denom;
      result(i) = HalleyIter(x(i), w);
      } else {
/* Use first five terms of Corliss et al. 4.19 */
      w = log(x(i));
      double L_2 = log(w);
      double L_3 = L_2 / w;
      double L_3_sq = L_3 * L_3;
      w += -L_2 + L_3 + 0.5 * L_3_sq - L_3 / w + L_3 / (w * w) - 1.5 * L_3_sq / w + L_3_sq * L_3 / 3;
      result(i) = HalleyIter(x(i), w);
    }
  }
  return(result);
}

// [[Rcpp::export]]
NumericVector lambertWm1_C(NumericVector x){
  int n = x.size();
  NumericVector result(n);
  double w;
  for (int i = 0; i < n; ++i) {
    if (x(i) == 0) {
      result(i) = -std::numeric_limits<double>::infinity();
    } else if (x(i) < -M_1_E || x(i) > 0.0) {
      result(i) = std::numeric_limits<double>::quiet_NaN();
    } else if (fabs(x(i) + M_1_E) < 4 * EPS) {
      result(i) = -1.0;
    } else {
/* Use first five terms of Corliss et al. 4.19 */
      w = log(-x(i));
      double L_2 = log(-w);
      double L_3 = L_2 / w;
      double L_3_sq = L_3 * L_3;
      w += -L_2 + L_3 + 0.5 * L_3_sq - L_3 / w + L_3 / (w * w) - 1.5 * L_3_sq / w + L_3_sq * L_3 / 3;
      result(i) = HalleyIter(x(i), w);
    }
  }
  return(result);
}
