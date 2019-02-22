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

#ifndef FLOW_CORRELATOR_H
#define FLOW_CORRELATOR_H

#include "QVector.h"
#include "Correlation.h"
#include "Sampler.h"

namespace Qn {
enum Weight {
  OBSERVABLE = false,
  REFERENCE = true,
};
class Correlator {
 public:
  using size_type = std::size_t;
  using corr_func = std::function<double(std::vector<Qn::QVector> &)>;
  Correlator(std::vector<std::string> input_names, corr_func lambda)
      : lambda_correlation_(std::move(lambda)), input_names_(std::move(input_names)) {
    is_reference_.resize(input_names.size());
    for (auto q : is_reference_) {
      q = false;
    }
    is_reference_[0] = true;
  }

  Correlator(std::vector<std::string> input_names, corr_func lambda, const TH1F &base)
      : binned_result_(new Qn::DataContainer<TH1F>()),
        lambda_correlation_(std::move(lambda)),
        input_names_(std::move(input_names)) {
    binned_result_->InitializeEntries(base);
    is_reference_.resize(input_names.size());
    for (auto q : is_reference_) {
      q = false;
    }
    is_reference_[0] = true;
  }

  void ConfigureSampler(Sampler::Method method, size_type nsamples) { sampler_.Configure(method, nsamples); }

  void BuildSamples(std::size_t nevents);

  const std::vector<std::string> &GetInputNames() const { return input_names_; }

  void FillCorrelation(const std::vector<DataContainerQVector> &inputs,
                       const std::vector<unsigned long> &eventindex,
                       std::size_t event_id);

  void FindAutoCorrelations();

  void RemoveAutoCorrelation();

  void ConfigureCorrelation(const std::vector<DataContainerQVector> &input, std::vector<Qn::Axis> event);

  Qn::Correlation GetCorrelation() const { return correlation_; }

  DataContainerStats GetResult() const { return result_; }

  std::shared_ptr<DataContainer<TH1F>> GetBinnedResult() const { return binned_result_; }

  void SetReferenceQVectors(std::vector<Weight> weights) {
    int i = 0;
    for (auto b : is_reference_) {
      b = weights[i];
      ++i;
    }
  }

 private:
  Qn::Sampler sampler_;
  Qn::Correlation correlation_;
  Qn::DataContainerStats result_;
  std::shared_ptr<Qn::DataContainer<TH1F>> binned_result_;
  std::function<double(std::vector<Qn::QVector> &)> lambda_correlation_;
  std::vector<std::string> input_names_;
  std::vector<std::vector<size_type>> autocorrelated_bins_;
  std::vector<bool> is_reference_;
};
}

#endif //FLOW_CORRELATOR_H
