//
// Created by Lukas Kreis on 18.04.18.
//

#ifndef FLOW_SAMPLE_H
#define FLOW_SAMPLE_H

#include <utility>
#include "Profile.h"

namespace Qn {

struct StatisticMean {
  double mean = 0;
  double sum = 0;
  int n = 0;

  StatisticMean() = default;

  void Update(double value) {
    sum += value;
    n++;
    mean = sum/(float) n;
  }

  void operator+=(StatisticMean b) {
    sum += b.sum;
    n += b.n;
    mean = sum/(float) n;
  }

  void operator*=(StatisticMean b) {
    sum *= b.sum;
    n += b.n;
    mean *= b.mean;
  }

  void operator/=(StatisticMean b) {
    sum /= b.sum;
    n += b.n;
    mean /= b.mean;
  }

  void operator-=(StatisticMean b) {
    sum -= b.sum;
    n -= b.n;
    mean = sum/(float) n;
  }

  void operator*=(double b) {
    sum *= b;
    mean *= b;
  }
};

class Sample : public Profile {
 public:
  Sample() = default;

  virtual ~Sample() = default;

  explicit Sample(int n_samples) { samples_stat_.resize(n_samples); }

  Sample(Profile a, std::vector<StatisticMean> means) :
      Profile(a),
      samples_stat_(std::move(means)) {}

  void Fill(const double value, const std::vector<unsigned int> &samples) {
    Profile::Update(value);
    for (const auto sample : samples) {
      samples_stat_[sample].Update(value);
    }
  }

  void SetNumberOfSamples(int nsamples) { samples_stat_.resize(nsamples); }

  double SampleMean(int isample) const { return samples_stat_[isample].mean; }
//
  void CalculateCorrelatedError() {
    subsample_sum = 0;
    subsample_sum2_ = 0;
    subsample_entries_ = 0;
    for (const auto stat : samples_stat_) {
      subsample_sum += stat.mean;
      subsample_sum2_ += stat.mean*stat.mean;
      subsample_entries_++;
    }
    correlated_error_ = Statistics::Sigma(subsample_sum/subsample_entries_, subsample_sum2_, subsample_entries_);
  }

  inline double CorrelatedError() const { return correlated_error_; }
//
  friend Sample operator+(const Sample &a, const Sample &b);
  friend Sample operator-(const Sample &a, const Sample &b);
  friend Sample operator*(const Sample &a, const Sample &b);
  friend Sample operator/(const Sample &a, const Sample &b);
  friend Sample operator*(const Sample &a, double b);
  friend Sample Merge(const Sample &a, const Sample &b);
//
  inline Sample Sqrt() const {
    Sample a(*this);
    a.mean_ = std::sqrt(std::abs(mean_));
    a.sum_ = std::sqrt(std::abs(sum_));
    a.sum2_ = std::sqrt(std::abs(sum2_));
    a.binentries_ = std::sqrt(std::abs(binentries_));
//    a.error_ = 1./2.*a.error_/a.mean_;
    return a;
  }

 private:
  std::vector<StatisticMean> samples_stat_;
  int subsample_entries_ = 0;
  double subsample_sum = 0.;
  double subsample_sum2_ = 0.;
  double correlated_error_ = 0.;
  /// \cond CLASSIMP
 ClassDef(Sample, 1);
  /// \endcond
};

inline Sample operator+(const Sample &a, const Sample &b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  if (!b.samples_stat_.empty()) {
    for (auto &sum : sums) {
      sum += b.samples_stat_[i];
      ++i;
    }
  }
  Sample c(operator+((Profile) a, (Profile) b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample operator-(const Sample &a, const Sample &b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  if (!b.samples_stat_.empty()) {
    for (auto &sum : sums) {
      sum -= b.samples_stat_[i];
      ++i;
    }
  }
  Sample c(operator-((Profile) a, (Profile) b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample operator*(const Sample &a, const Sample &b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  if (!b.samples_stat_.empty()) {
    for (auto &sum : sums) {
      sum *= b.samples_stat_[i];
      ++i;
    }
  }
  Sample c(operator*((Profile) a, (Profile) b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample operator/(const Sample &a, const Sample &b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  if (!b.samples_stat_.empty()) {
    for (auto &sum : sums) {
      sum /= b.samples_stat_[i];
      ++i;
    }
  }
  Sample c(operator/((Profile) a, (Profile) b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample operator*(const Sample &a, double b) {
  std::vector<StatisticMean> sums(a.samples_stat_);
  int i = 0;
  for (auto &sum : sums) {
    sum *= b;
    ++i;
  }
  Sample c(operator*((Profile) a, b), sums);
  c.CalculateCorrelatedError();
  return c;
}

inline Sample Merge(const Sample &a, const Sample &b) {
  return a + b;
}

}

#endif //FLOW_SAMPLE_H
