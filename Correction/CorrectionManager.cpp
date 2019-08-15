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

#include "CorrectionManager.h"
#include "TList.h"

namespace Qn {

void CorrectionManager::InitializeCorrections() {
  // Connects the correction histogram list
  if (!correction_input_file_) {
    correction_input_file_ = std::make_unique<TFile>(correction_input_file_name_.data(), "READ");
  }
  if (correction_input_file_ && !correction_input_file_->IsZombie()) {
    auto input = dynamic_cast<TList *>(correction_input_file_->FindObjectAny(kCorrectionListName));
    correction_input_.reset(input);
    if (input) correction_input_->SetOwner(true);
  }
  // Prepares the correctionsteps
  detectors_.CreateSupportQVectors();
  correction_output = std::make_unique<TList>();
  correction_output->SetName(kCorrectionListName);
  correction_output->SetOwner(true);
}

void CorrectionManager::SetCurrentRunName(const std::string &name) {
  runs_.SetCurrentRun(name);
  if (!runs_.empty()) {
    auto current_run = new TList();
    current_run->SetName(runs_.GetCurrent().data());
    current_run->SetOwner(true);
    correction_output->Add(current_run);
    detectors_.CreateCorrectionHistograms(current_run);
  }
  if (correction_input_) {
    auto current_run = (TList *) correction_input_->FindObject(runs_.GetCurrent().data());
    if (current_run) {
      detectors_.AttachCorrectionInput(current_run);
    }
  }
  detectors_.IncludeQnVectors();
  if (fill_output_tree_ && out_tree_) {
    detectors_.SetOutputTree(out_tree_);
    variable_manager_.SetOutputTree(out_tree_);
  }
  detectors_.CreateReport();
}

void CorrectionManager::AttachQAHistograms() {
  correction_qa_histos_ = std::make_unique<TList>();
  correction_qa_histos_->SetName("QA_histograms");
  correction_qa_histos_->SetOwner(true);
  detectors_.AttachQAHistograms(correction_qa_histos_.get(),
                                fill_qa_histos_,
                                fill_validation_qa_histos_);
  auto event_qa_list = new TList();
  event_qa_list->SetOwner(true);
  event_qa_list->SetName("event_QA");
  event_cuts_.CreateCutReport("Cut_Report:");
  event_cuts_.AddToList(event_qa_list);
  event_histograms_.AddToList(event_qa_list);
  correction_qa_histos_->Add(event_qa_list);
}

void CorrectionManager::InitializeOnNode() {
  variable_manager_.Initialize();
  for (const auto &axis : correction_axes_callback_) {
    correction_axes_.Add(axis(&variable_manager_));
  }
  event_histograms_.Initialize(&variable_manager_);
  detectors_.Initialize(this);
  event_cuts_.Initialize(&variable_manager_);
  InitializeCorrections();
  AttachQAHistograms();
}

bool CorrectionManager::ProcessEvent() {
  event_passed_cuts_ = event_cuts_.CheckCuts(0);
  if (event_passed_cuts_) {
    event_cuts_.FillReport();
    variable_manager_.UpdateOutVariables();
    event_histograms_.Fill();
  }
  return event_passed_cuts_;
}

void CorrectionManager::ProcessCorrections() {
  if (event_passed_cuts_) {
    detectors_.ProcessCorrections();
    detectors_.FillReport();
    if (fill_output_tree_) out_tree_->Fill();
  }
}

void CorrectionManager::Reset() {
  event_passed_cuts_ = false;
  detectors_.ResetDetectors();
}

void CorrectionManager::Finalize() {
  auto calibration_list = (TList *) correction_output->FindObject(kCorrectionListName);
  if (calibration_list) {
    correction_output->Add(calibration_list->Clone("all"));
  }
}

}