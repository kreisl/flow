// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FLOW_PROFILE_H
#define FLOW_PROFILE_H

#include <iterator>
#include <algorithm>
#include <utility>
#include <array>

#include "TMathBase.h"
#include "TMath.h"

namespace Qn {

namespace Statistics {

inline double Sigma(const double mean, const double sum2, const int n) {
  double variance = TMath::Abs(sum2/(double) n - mean*mean);
  return sqrt(variance)/sqrt((double) n);
}

inline double Sigma(const double mean, const double sum2, const double n) {
  double variance = TMath::Abs(sum2/n - mean*mean);
  return sqrt(variance)/sqrt(n);
}
}

class Profile {
 public:
  Profile() = default;
  Profile(double mean, double sum, double sum2, double error, int entries) :
      mean_(mean),
      sum_(sum),
      sum2_(sum2),
      entries_(entries),
      binentries_((double) entries),
      error_(error),
      mult_weight_(entries) {}
  Profile(double mean, double sum, double sum2, double error, int entries, double binentries) :
      mean_(mean),
      sum_(sum),
      sum2_(sum2),
      entries_(entries),
      binentries_(binentries),
      error_(error),
      mult_weight_(entries_) {}
  Profile(double mean, double sum, double sum2, double error, int entries, double binentries, double mult_weight) :
      mean_(mean),
      sum_(sum),
      sum2_(sum2),
      entries_(entries),
      binentries_(binentries),
      error_(error),
      mult_weight_(mult_weight) {}
  virtual ~Profile() = default;

  inline void Update(double value) {
    sum_ = sum_ + value;
    ++entries_;
    binentries_ += 1.;
    mean_ = sum_/(entries_);
    sum2_ = sum2_ + value*value;
    error_ = Qn::Statistics::Sigma(mean_, sum2_, entries_);
    mult_weight_ = 1.;
  };

  inline void Update(double value, double mult_weight) {
    sum_ = sum_ + value;
    double multsum = mult_weight_*entries_ + mult_weight;
    ++entries_;
    binentries_ += 1.;
    mean_ = sum_/(entries_);
    sum2_ = sum2_ + value*value;
    error_ = Qn::Statistics::Sigma(mean_, sum2_, entries_);
    mult_weight_ = multsum/entries_;
  };

  double Mean() const { return mean_; }
  double Sum() const { return sum_; }
  double Sum2() const { return sum2_; }
  virtual double Error() const { return error_; }
  int Entries() const { return entries_; }
  double BinEntries() const { return binentries_; }
  double MultWeight() const { return mult_weight_; }

  inline Profile Sqrt() const {
    Profile a(*this);
    a.mean_ = std::sqrt(std::abs(mean_));
//    a.sum_ = std::abs(sum_);
//    a.sum2_ = std::abs(sum2_);
//    a.error_ = 1./2.*a.error_/a.mean_;
    return a;
  }

  friend Qn::Profile operator+(Qn::Profile a, Qn::Profile b);
  friend Qn::Profile operator-(Qn::Profile a, Qn::Profile b);
  friend Qn::Profile operator*(Qn::Profile a, Qn::Profile b);
  friend Qn::Profile operator/(Qn::Profile a, Qn::Profile b);
  friend Qn::Profile operator*(Qn::Profile a, double b);
  friend Qn::Profile Merge(Qn::Profile a, Qn::Profile b);
  friend Qn::Profile AddBins(Qn::Profile a, Qn::Profile b);
  friend void SetToZero(Qn::Profile &a);
 protected:
  double mean_ = 0;
  double sum_ = 0;
  double sum2_ = 0;
  int entries_ = 0;
  double binentries_ = 0;
  double error_ = 0;
  double mult_weight_ = 1.;

  /// \cond CLASSIMP
 ClassDef(Profile, 3);
/// \endcond
};

inline void SetToZero(Qn::Profile &a) {
  a.mean_ = 0;
  a.sum_ = 0;
  a.sum2_ = 0;
  a.entries_ = 0;
  a.binentries_ = 0;
  a.error_ = 0;
  a.mult_weight_ = 1.;
}

inline Qn::Profile operator*(Qn::Profile a, double b) {
  double nsum = a.sum_*b;
  double nsum2 = a.sum2_*b;
  int nentries = a.entries_;
  double nmean = a.mean_*b;
  double nerror = a.error_*fabs(b);
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries);
  return c;
}

inline Qn::Profile AddBins(Qn::Profile a, Qn::Profile b) {
  int nentries = a.entries_ + b.entries_;
  double binentries = a.binentries_ + b.binentries_;
  double nsum = 2*(a.mult_weight_*a.sum_ + b.mult_weight_*b.sum_)/(a.mult_weight_ + b.mult_weight_);
  double nsum2 = 2*(a.mult_weight_*a.sum2_ + b.mult_weight_*b.sum2_)/(a.mult_weight_ + b.mult_weight_);
  double nmean = 0;
  double mult_weight = 0;
  if (nentries > 0) {
    mult_weight = (a.mult_weight_*a.entries_ + b.mult_weight_*b.entries_)/binentries;
  }
  if (binentries > 0) nmean = nsum/binentries;
  double nerror = Qn::Statistics::Sigma(nmean, nsum2, nentries);
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries, binentries, mult_weight);
  return c;
}

//inline Qn::Profile operator+(Qn::Profile a, Qn::Profile b) {
//  int nentries = a.entries_ + b.entries_;
//  double nsum = (a.sum_*a.entries_ + b.sum_*b.entries_)/nentries;
//  double nsum2 = (a.sum2_*a.entries_ + b.sum2_*b.entries_)/nentries;
//  double nmean = (a.entries_*a.mean_ + b.entries_*b.mean_)/nentries;
//  double nerror =
//  std::sqrt(std::abs((a.error_*a.error_*a.entries_ + b.error_*b.error_*b.entries_ - 2*a.mean_*b.mean_))/nentries);
//  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries);
//  return c;
//}

inline Qn::Profile operator+(Qn::Profile a, Qn::Profile b) {
  int nentries = a.entries_ + b.entries_;
  double binentries = a.binentries_ + b.binentries_;
  double nsum = 2*(a.mult_weight_*a.sum_ + b.mult_weight_*b.sum_)/(a.mult_weight_ + b.mult_weight_);
  double nsum2 = 2*(a.mult_weight_*a.sum2_ + b.mult_weight_*b.sum2_)/(a.mult_weight_ + b.mult_weight_);
  double nmean = 0;
  double mult_weight = 0;
  if (nentries > 0) {
    mult_weight = (a.mult_weight_*a.entries_ + b.mult_weight_*b.entries_)/binentries;
  }
  if (binentries > 0) nmean = nsum/binentries;
  double nerror = Qn::Statistics::Sigma(nmean, nsum2, nentries);
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries, binentries, mult_weight);
  return c;
}

inline Qn::Profile Merge(Qn::Profile a, Qn::Profile b) {
  int nentries = a.entries_ + b.entries_;
  double nsum = a.sum_ + b.sum_;
  double nmean = nsum/nentries;
  double nsum2 = a.sum2_ + b.sum2_;
  double mult_weight = (a.mult_weight_*a.entries_ + b.mult_weight_*b.entries_)/(a.entries_ + b.entries_);
  double nerror = Qn::Statistics::Sigma(nmean, nsum2, nentries);
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries, mult_weight);
  return c;
}

inline Qn::Profile operator-(Qn::Profile a, Qn::Profile b) {
  int nentries = a.entries_ + b.entries_;
  double mult_weight = (a.mult_weight_*a.entries_ + b.mult_weight_*b.entries_)/(a.entries_ + b.entries_);
  double nsum = a.sum_ - b.sum_;
  double nsum2 = a.sum2_ - b.sum2_;
  double binentries = a.binentries_ - b.binentries_;
  double nmean = nsum/nentries;
  double nerror = Qn::Statistics::Sigma(nmean, nsum2, binentries);
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries, binentries, mult_weight);
  return c;
}

inline Qn::Profile operator*(Qn::Profile a, Qn::Profile b) {
  double nmean = a.mean_*b.mean_;
  int nentries = a.entries_;
  double mult_weight = a.mult_weight_;
  double binentries = a.binentries_*b.binentries_;
  double nsum2 = a.mean_*a.mean_*b.error_*b.error_ + b.mean_*b.mean_*a.error_*a.error_;
  double nerror = std::sqrt(a.mean_*a.mean_*b.error_*b.error_ + b.mean_*b.mean_*a.error_*a.error_);
  double nsum = a.sum_*b.sum_;
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries, binentries, mult_weight);
  return c;
}

inline Qn::Profile operator/(Qn::Profile a, Qn::Profile b) {
  double nmean = 0;
  double nsum2 = 0;
  if (std::abs(b.Mean() - 0) > 10e-8) {
    nmean = a.mean_/b.mean_;
    auto asq = a.sum_*a.sum_;
    auto bsq = b.sum_*b.sum_;
    nsum2 = (a.sum2_*bsq + b.sum2_*asq)/(bsq*bsq);
  } else {
    nsum2 = 0;
  }
  int nentries = a.Entries();
  double multweight = a.mult_weight_;
  double binentries = a.binentries_/b.binentries_;
  double nsum = a.sum_/b.sum_;
  double nerror =
      sqrt((a.error_*a.error_*b.mean_*b.mean_ + b.error_*b.error_*a.mean_*a.mean_)/(b.mean_*b.mean_*b.mean_*b.mean_));
  Qn::Profile c(nmean, nsum, nsum2, nerror, nentries, binentries, multweight);
  return c;
}

}

#endif //FLOW_STATISTICS_H
