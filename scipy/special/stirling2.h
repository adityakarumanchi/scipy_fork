#ifndef STIRLING_H
#define STIRLING_H

#include <cmath>
#include <complex>
#include <limits>
#include <memory>

#include <xsf/binom.h>
#include <xsf/lambertw.h>
#include "sf_error.h"


/*     Stirling numbers of the second kind
 *
 * SYNOPSIS: Stirling numbers of the second kind count the
 *  number of ways to make a partition of n distinct elements
 *  into k non-empty subsets.
 *
 * DESCRIPTION: n is the number of distinct elements of the set
 *  to be partitioned and k is the number of non-empty subsets.
 *  The values for n < 0 or k < 0 are interpreted as 0. If you
 * ACCURACY: The method returns a double type.
 *
 * NOTE: this file was added by Lucas Roberts
 */

// Dynamic programming

double _stirling2_dp(double n, double k) {
    if ((n == 0 && k == 0) || (n==1 && k==1)) {
        return 1.;
    }
    if (k <= 0 || k > n || n < 0) {
        return 0.;
    }
    int arraySize = k <= n - k + 1 ? k : n - k + 1;
    auto curr = std::unique_ptr<double[]>{new (std::nothrow) double[arraySize]};
    if (curr == nullptr) {
        sf_error("stirling2", SF_ERROR_MEMORY, NULL);
        return std::numeric_limits<double>::quiet_NaN();
    }
    for (int i = 0; i < arraySize; i++) {
        curr[i] = 1.;
    }
    if (k <= n - k + 1) {
        for (int i = 1; i < n - k + 1; i++) {
            for (int j = 1; j < k; j++) {
                curr[j] = (j + 1) * curr[j] + curr[j - 1];
                if (std::isinf(curr[j])) {
                    sf_error("stirling2", SF_ERROR_OVERFLOW, NULL);
                    return INFINITY; // numeric overflow
                }
            }
        }
    } else {
        for (int i = 1; i < k; i++) {
            for (int j = 1; j < n - k + 1; j++) {
                curr[j] = (i + 1) * curr[j - 1] + curr[j];
                if (std::isinf(curr[j])) {
                    sf_error("stirling2", SF_ERROR_OVERFLOW, NULL);
                    return INFINITY; // numeric overflow
                }
            }
        }
    }
    return curr[arraySize - 1];
}


// second order Temme approximation

double _stirling2_temme(double n, double k) {
  if ((n == k && n >= 0) || (n > 0 && k==1)) {
      return 1.;
  }
  if (k <= 0 || k > n || n < 0) {
      return 0.;
  }
  double mu = static_cast<double>(k) / n;
  double d = std::exp(-1/mu) / mu;
  std::complex<double> delta = std::complex<double>(-d, 0);
  // note: lambert returns complex value, we only want the real part
  // matching k=0, tolerance=1e-8 from _lambertw.py
  std::complex<double> lwv = xsf::lambertw(delta, 0, 1e-8);
  double x0 = lwv.real() + 1/mu;
  double t0 = (1/mu) - 1;
  double F = std::sqrt(t0/((1 + t0)*(x0 - t0)));
  double A = -n * std::log(x0) + k * std::log(exp(x0) - 1)
             - k * t0 + (n - k) * std::log(t0);
  // write F1 as numerator and denominator and apply Horner rule to num
  double xt = x0*t0;
  double t0power3 = t0*t0*t0;
  // first all x only terms
  double num = -2*x0*x0*x0;
  // then all t only terms
  num += ((t0 + 2)*t0 + 2)*(2*t0power3);
  // finally mixed x^a * t^b terms
  num += (-6*t0power3 + (8*t0 - 6*x0 - 5)*xt + ((2.*x0+1.)*x0+3.)*x0)*xt;
  double denom = (24*F*(1 + t0) * (1 + t0)*(x0 - t0)*(x0 - t0)*(x0 - t0)*(x0-t0));
  double F1 = num / denom;
  return std::exp(A) * std::pow(k, n - k) * xsf::binom(n, k) * (F - F1/k);
}


/*
 *  This is the main entrypoint from stirling2 which handles dispatch to each
 *  of the (private) functions here in the file that implement specific
 *  ways of approximating the stirling numbers of the second kind
 *
 */

double _stirling2_inexact(double n, double k) {
  if (n <= 50) {
    return _stirling2_dp(n, k);
  } else {
    return _stirling2_temme(n, k);
  }
}

#endif
