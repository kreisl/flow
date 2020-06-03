// Flow Vector Correction Framework
//
// Copyright (C) 2019  Lukas Kreis Ilya Selyuzhenkov
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
#ifndef FLOW_DATAFRAMECORRELATION_H
#define FLOW_DATAFRAMECORRELATION_H

#include <functional>
#include <cstring>
#include <type_traits>

#include "ROOT/RIntegerSequence.hxx"
#include "ROOT/RDataFrame.hxx"

#include "Stats.h"
#include "TTreeReader.h"
#include "DataContainer.h"
#include "TemplateHelpers.h"

namespace Qn {
namespace Correlation {

template<typename Function, typename Qvectors, typename InputDataContainers>
class Correlation;

template<typename Function, typename... Qvectors, typename... InputDataContainers>
class Correlation<Function, std::tuple<Qvectors...>, std::tuple<InputDataContainers...>> {
 public:
  constexpr static std::size_t NInputs = sizeof...(Qvectors);
  using DataContainerTypeTuple = typename std::tuple<InputDataContainers...>;
  using FunctionType = Function;
  using CollelationHolder = std::vector<CorrelationResult>;

  explicit Correlation(Function function) : function_(function) {}

  void Initialize(TTreeReader &reader) {
    std::vector<TTreeReaderValue<DataContainerQVector>> input_data;
    for (auto name : input_names_) {
      input_data.emplace_back(reader, name.data());
    }
    reader.SetLocalEntry(1);
    for (std::size_t i = 0; i < input_data.size(); ++i) {
      auto &i_data = input_data[i];
      if (i_data.GetSetupStatus() < 0) {
        auto message = std::string("The Q-Vector entry") +
            i_data.GetBranchName() + "in the tree is not valid. Cannot setup the correlation";
        throw std::runtime_error(message);
      }
      if (!i_data->IsIntegrated()) {
        AddAxes(input_data, i);
      }
    }
    correlation_result_.resize(data_container_correlation_.size());
    reader.Restart();
  }

  template<typename ...Names>
  void SetInputNames(Names ...names) {
    static_assert(sizeof...(names)==NInputs,
                  "The input names must match the number of variables in the correlation function");
    input_names_ = {names...};
  }

  template<typename ...Weights>
  void SetWeights(Weights ...weights) {
    static_assert(sizeof...(weights)==NInputs,
                  "The input weights must match the number of variables in the correlation function");
    std::array<Qn::Stats::Weights, NInputs> weights_array{weights...};
    for (std::size_t i = 0; i < NInputs; ++i) {
      use_weights_[i] = weights_array[i]==Qn::Stats::Weights::OBSERVABLE;
    }
  }

  bool IsObservable() const {
    return std::any_of(std::begin(use_weights_), std::end(use_weights_),[](bool a){return a;});
  }

  const CollelationHolder &Correlate(InputDataContainers... input) {
    for (auto &bin : correlation_result_) { bin.validity = false; }
    std::size_t output_bin = 0;
    std::array<const Qn::QVector *, NInputs> q_vectors;
    std::array<DataContainerQVector, NInputs> input_array = {input...};
    IterateOverBins(output_bin, q_vectors, input_array, 0);
    return correlation_result_;
  }

  std::vector<Qn::AxisD> GetCorrelationAxes() const {
    return !data_container_correlation_.IsIntegrated() ? data_container_correlation_.GetAxes()
                                                       : std::vector<Qn::AxisD>{};
  }

  std::vector<std::string> GetInputNames() const {
    return std::vector<std::string>{std::begin(input_names_), std::end(input_names_)};
  }

 private:

  double CalculateWeights(const std::array<const Qn::QVector *, NInputs> &q_array) const {
    int i = 0;
    double weight = 1.0;
    for (const auto &q : q_array) {
      if (use_weights_[i]) weight *= q->sumweights();
      ++i;
    }
    return weight;
  }

  void IterateOverBins(std::size_t &output_bin,
                       std::array<const Qn::QVector *, NInputs> &q_array,
                       const std::array<DataContainerQVector, NInputs> &input_array,
                       std::size_t iteration) {
    // ends recursive iteration over the data inputs
    if (iteration + 1==NInputs) {
      // iterates over all bins of the input data
      for (const auto &bin : input_array[iteration]) {
        // skips empty bins
        if (bin.n() < 1) {
          ++output_bin;
          continue;
        }
        // save pointer to Q vector in an array
        q_array[iteration] = &bin;
        // calculate the output weight
        auto weight = CalculateWeights(q_array);
        // Apply the correlation function on the inputs saved in the array.
        // Save together with the weight and the validity in the output container in the  output bin.
        correlation_result_[output_bin] = {TemplateHelpers::Call(function_, q_array), true, weight};
        // increment output bin.
        ++output_bin;
      }
      // ends the recursion
      return;
    }
    // starts the recursion over the input data.
    // iterates over all bins of the input data.
    for (const auto &bin : input_array[iteration]) {
      // skips empty bins
      if (bin.n() < 1) {
        ++output_bin;
        continue;
      }
      // save pointer to Q vector in an array
      q_array[iteration] = &bin;
      // next step of recursion
      IterateOverBins(output_bin, q_array, input_array, iteration + 1);
    }
  }

  void AddAxes(std::vector<TTreeReaderValue<DataContainerQVector>> &data_containers, std::size_t i) {
    for (auto axis :data_containers[i]->GetAxes()) {
      // default name of the axis.
      std::string name = axis.Name();
      // check all other inputs for an axis with the same name, or for another input with the same name.
      for (std::size_t j = 0; j < data_containers.size(); ++j) {
        if (i!=j) {
          auto &other = data_containers[j];
          // only check for non-integrated containers. Integrated containers do not add their axis to the correlation.
          if (!other->IsIntegrated()) {
            for (const auto &other_axis :other->GetAxes()) {
              if (axis==other_axis) {
                auto input_name = data_containers[i].GetBranchName();
                if (std::strcmp(input_name, other.GetBranchName())==0) {
                  // Prepends the input position and name to the axis name if another identical input is present.
                  name = std::to_string(i) + "_" + input_name + "_" + axis.Name();
                } else {
                  // Prepends the input name to the axis name if another input with an identical axis name is present.
                  name = std::string(input_name) + "_" + axis.Name();
                }
              }
            }
          }
        }
      }
      // Renames the axis and adds it to the correlation container.
      axis.SetName(name);
      data_container_correlation_.AddAxis(axis);
    }
  }

  Qn::DataContainerCorrelation data_container_correlation_;
  std::vector<CorrelationResult> correlation_result_;
  Function function_;
  std::array<std::string, NInputs> input_names_;
  std::array<bool, NInputs> use_weights_;
};

}
}
#endif //FLOW_DATAFRAMECORRELATION_H
