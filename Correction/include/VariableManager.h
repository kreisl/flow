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

#ifndef FLOW_VARIABLEMANAGER_H
#define FLOW_VARIABLEMANAGER_H

#include <string>
#include <map>
#include <utility>
#include <cmath>
#include "TTree.h"

#include "InputVariable.h"

/**
 * @brief Attaches a variable to a chosen tree.
 * Type is converted to the chosen type.
 * @tparam T type of variable saved in the tree.
 */
template<typename T>
class OutValue {
 public:
  explicit OutValue(Qn::InputVariableD var) : var_(std::move(var)) {}
  /**
   * @brief updates the value. To be called every event.
   */
  void UpdateValue() { value_ = static_cast<T>(*var_.begin()); }
  /**
   * @brief Creates a new branch in the tree.
   * @param tree output tree
   */
  void SetToTree(TTree *tree) { tree->Branch(var_.Name().data(), &value_); }
 private:
  T value_; /// value which is written
  Qn::InputVariableD var_; /// Variable to be written to the tree
};

namespace Qn {
/**
 * @brief Manages the input variables for the correction step.
 * A variable consist of a name and a unsigned integer position in the value array and an unsigned integer length.
 * Different variables all access the same variable container.
 */
class VariableManager {
 public:

  VariableManager() {
    for (int i = 0; i < kMaxSize; ++i) {
      variable_values_float_[i] = NAN;
    }
  }

  void InitializeVariableContainers() {
    variable_values_float_ = new double[kMaxSize]; /// non-owning pointer to variables
    variable_values_ones_ = new double[kMaxSize]; /// values container of ones.
    for (auto &var : name_var_map_) {
      var.second.var_container = variable_values_float_;
    }
    name_var_map_["Ones"].var_container = variable_values_ones_;
  }

  /**
   * @brief Creates a new variable in the variable manager.
   * @param name Name of the new variable.
   * @param id position of the variable in the values container.
   * @param length length of the variable in the values container.
   */
  void CreateVariable(std::string name, const int id, const int length) {
    InputVariableD var(id, length, name);
    name_var_map_.emplace(name, var);
  }
  /**
   * @brief Initializes the variable container for ones.
   */
  void CreateVariableOnes() {
    InputVariableD var(0, kMaxSize, "Ones");
    for (unsigned int i = 0; i < kMaxSize; ++i) { variable_values_ones_[i] = 1.0; }
    name_var_map_.emplace("Ones", var);
  }
  /**
   * @brief Creates a channel variable (variables which counts from 0 to the size-1).
   * @param name name of the variable
   * @param size number of channels
   */
  void CreateChannelVariable(std::string name, const int size) {
    InputVariableD var(0, size, name);
    auto *arr = new double[size];
    for (int i = 0; i < size; ++i) { arr[i] = i; }
    var.var_container = arr;
    name_var_map_.emplace(name, var);
  }
  /**
   * @brief Finds the variable in the variable manager.
   * @param name Name of the variable.
   * @return Variable of the given name.
   */
  InputVariableD FindVariable(const std::string &name) const { return name_var_map_.at(name); }
  /**
   * @brief Find the position in the values container of a variable with a given name.
   * @param name Name of the variable.
   * @return position in the values container.
   */
  int FindNum(const std::string &name) const { return name_var_map_.at(name).id_; }
  /**
   * @brief Get the values container.
   * @return a pointer to the values container.
   */
  double *GetVariableContainer() { return variable_values_float_; }
  /**
   * @brief Sets the given pointer to the pointer to the values container.
   * It is used to forward the data to the correction step
   * @param var values container pointer pointer.
   */
  void FillToQnCorrections(double **var) {*var = variable_values_float_;}

  /**
   * @brief Register the variable to be saved in the output tree as float.
   * Beware of conversion from double.
   * @param name Name of the variable.
   */
  void RegisterOutputF(const std::string &name) {
    OutValue<Float_t> val(FindVariable(name));
    output_vars_f_.push_back(val);
  }

  /**
   * @brief Register the variable to be saved in the output tree as long.
   * Beware of conversion from double.
   * @param name Name of the variable.
   */
  void RegisterOutputL(const std::string &name) {
    OutValue<Long64_t> val(FindVariable(name));
    output_vars_l_.push_back(val);
  }

  /**
   * @brief Creates Branches in tree for saving the event information.
   * @param tree output tree to contain the event information
   */
  void SetOutputToTree(TTree *tree) {
    for (auto &element : output_vars_f_) { element.SetToTree(tree); }
    for (auto &element : output_vars_l_) { element.SetToTree(tree); }
  }

  /**
   * @brief Updates the output variables.
   */
  void UpdateOutVariables() {
    for (auto &element : output_vars_f_) { element.UpdateValue(); }
    for (auto &element : output_vars_l_) { element.UpdateValue(); }
  }

 private:
  static constexpr int kMaxSize = 11000; /// Maximum number of variables.
  double *variable_values_float_ = nullptr; /// non-owning pointer to variables
  double *variable_values_ones_ = nullptr; /// values container of ones.
  std::map<std::string, InputVariableD> name_var_map_; /// name to variable map
  std::vector<OutValue<float>> output_vars_f_; /// variables registered for output as float
  std::vector<OutValue<Long64_t>> output_vars_l_; /// variables registered for output as long
};
}

#endif //FLOW_VARIABLEMANAGER_H
