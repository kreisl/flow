/**************************************************************************************************
 *                                                                                                *
 * Package:       FlowVectorCorrections                                                           *
 * Authors:       Jaap Onderwaater, GSI, jacobus.onderwaater@cern.ch                              *
 *                Ilya Selyuzhenkov, GSI, ilya.selyuzhenkov@gmail.com                             *
 *                Víctor González, UCM, victor.gonzalez@cern.ch                                   *
 *                Contributors are mentioned in the code where appropriate.                       *
 * Development:   2012-2016                                                                       *
 *                                                                                                *
 * This file is part of FlowVectorCorrections, a software package that corrects Q-vector          *
 * measurements for effects of nonuniform detector acceptance. The corrections in this package    *
 * are based on publication:                                                                      *
 *                                                                                                *
 *  [1] "Effects of non-uniform acceptance in anisotropic flow measurements"                      *
 *  Ilya Selyuzhenkov and Sergei Voloshin                                                         *
 *  Phys. Rev. C 77, 034904 (2008)                                                                *
 *                                                                                                *
 * The procedure proposed in [1] is extended with the following steps:                            *
 * (*) alignment correction between subevents                                                     *
 * (*) possibility to extract the twist and rescaling corrections                                 *
 *      for the case of three detector subevents                                                  *
 *      (currently limited to the case of two “hit-only” and one “tracking” detectors)            *
 * (*) (optional) channel equalization                                                            *
 * (*) flow vector width equalization                                                             *
 *                                                                                                *
 * FlowVectorCorrections is distributed under the terms of the GNU General Public License (GPL)   *
 * (https://en.wikipedia.org/wiki/GNU_General_Public_License)                                     *
 * either version 3 of the License, or (at your option) any later version.                        *
 *                                                                                                *
 **************************************************************************************************/

/// \file QnCorrectionsInputGainEqualization.cxx
/// \brief Implementation of procedures for gain equalization on input data.
#include "EventClassVariablesSet.h"
#include "CorrectionProfileChannelizedIngress.h"
#include "CorrectionProfileChannelized.h"
#include "CorrectionHistogramChannelizedSparse.h"
#include "SubEventChannels.h"
#include "GainEqualization.h"
#include "ROOT/RMakeUnique.hxx"
/// \cond CLASSIMP
ClassImp(Qn::GainEqualization);
/// \endcond
namespace Qn {

/// Default constructor
/// Passes to the base class the identity data for the Gain equalization correction step
GainEqualization::GainEqualization() : CorrectionOnInputData(szCorrectionName, szPriority) {}

/// Attaches the needed input information to the correction step
///
/// If the attachment succeeded asks for hard coded group weights to
/// the detector configuration
/// \param list list where the inputs should be found
/// \return kTRUE if everything went OK
void GainEqualization::AttachInput(TList *list) {
  auto ownerConfiguration = dynamic_cast<SubEventChannels *>(fSubEvent);
  if (fInputHistograms->AttachHistograms(list,
                                         ownerConfiguration->GetUsedChannelsMask(),
                                         ownerConfiguration->GetChannelsGroups())) {
    fState = State::APPLYCOLLECT;
    fHardCodedWeights = ownerConfiguration->GetHardCodedGroupWeights();
  }
}

/// Asks for support data structures creation
///
/// Does nothing for the time being
void GainEqualization::CreateSupportDataStructures() {

}

/// Asks for support histograms creation
///
/// Allocates the histogram objects and creates the calibration histograms.
/// The histograms are constructed with standard deviation error calculation
/// for the proper behavior of the gain equalization.
///
/// Process concurrency requires Calibration Histograms creation for all c
/// concurrent processes but not for Input Histograms so, we delete previously
/// allocated ones.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
void GainEqualization::AttachSupportHistograms(TList *list) {
  std::string name = std::string(szSupportHistogramName) + "_" + fSubEvent->GetName();
  auto *sub_event = dynamic_cast<SubEventChannels *>(fSubEvent);
  fInputHistograms = std::make_unique<CorrectionProfileChannelizedIngress>(name,
                                                                           sub_event->GetEventClassVariablesSet(),
                                                                           sub_event->GetNoOfChannels(),
                                                                           CorrectionHistogramBase::ErrorMode::SPREAD);
  fInputHistograms->SetNoOfEntriesThreshold(fMinNoOfEntriesToValidate);
  fCalibrationHistograms = std::make_unique<CorrectionProfileChannelized>(name,
                                                                          sub_event->GetEventClassVariablesSet(),
                                                                          sub_event->GetNoOfChannels(),
                                                                          CorrectionHistogramBase::ErrorMode::SPREAD);
  fCalibrationHistograms->CreateProfileHistograms(list,
                                                  sub_event->GetUsedChannelsMask(),
                                                  sub_event->GetChannelsGroups());
}

/// Asks for QA histograms creation
///
/// Allocates the histogram objects and creates the QA histograms.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
void GainEqualization::AttachQAHistograms(TList *list) {
  std::string name = std::string(szSupportHistogramName) + "_" + fSubEvent->GetName();
  std::string before_name = name + "Before";
  std::string before_title = name + " before gain equalization";
  std::string after_name = name + "After";
  std::string after_title = name + " after gain equalization";
  auto sub_event = dynamic_cast<SubEventChannels *>(fSubEvent);
  fQAMultiplicityBefore = std::make_unique<CorrectionProfileChannelized>(before_name, before_title,
                                                                         sub_event->GetEventClassVariablesSet(),
                                                                         sub_event->GetNoOfChannels());
  fQAMultiplicityBefore->CreateProfileHistograms(list, sub_event->GetUsedChannelsMask(),
                                                 sub_event->GetChannelsGroups());
  fQAMultiplicityAfter = std::make_unique<CorrectionProfileChannelized>(after_name, after_title,
                                                                        sub_event->GetEventClassVariablesSet(),
                                                                        sub_event->GetNoOfChannels());
  fQAMultiplicityAfter->CreateProfileHistograms(list, sub_event->GetUsedChannelsMask(),
                                                sub_event->GetChannelsGroups());
}

/// Asks for non validated entries QA histograms creation
///
/// Allocates the histogram objects and creates the non validated entries QA histograms.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
void GainEqualization::AttachNveQAHistograms(TList *list) {
  auto sub_event = dynamic_cast<SubEventChannels *>(fSubEvent);
  std::string name = std::string(szQANotValidatedHistogramName) + "_" + fSubEvent->GetName();
  fQANotValidatedBin = std::make_unique<CorrectionHistogramChannelizedSparse>(name,
                                                                              sub_event->GetEventClassVariablesSet(),
                                                                              sub_event->GetNoOfChannels());
  fQANotValidatedBin->CreateChannelizedHistogram(list, sub_event->GetUsedChannelsMask());
}

/// Processes the correction step
///
/// Data are always taken from the data bank from the equalized weights
/// allowing chaining of input data corrections so, caution must be taken to be
/// sure that, on initialising, weight and equalized weight match
/// Due to this structure as today it is not possible to split data collection
/// from correction processing. If so is required probably multiple equalization
/// structures should be included.
/// \return kTRUE if the correction step was applied
bool GainEqualization::ProcessCorrections(const double *variableContainer) {
  switch (fState) {
    case State::CALIBRATION:
      /* collect the data needed to further produce equalization parameters */
      for (const auto &dataVector : fSubEvent->GetInputDataBank()) {
        fCalibrationHistograms->Fill(variableContainer, dataVector.GetId(), dataVector.EqualizedWeight());
      }
      return false;
    case State::APPLYCOLLECT:
      /* collect the data needed to further produce equalization parameters */
      for (const auto &dataVector : fSubEvent->GetInputDataBank()) {
        fCalibrationHistograms->Fill(variableContainer, dataVector.GetId(), dataVector.EqualizedWeight());
      }
      /* and proceed to ... */
      /* FALLTHRU */
    case State::APPLY: /* apply the equalization */
      /* collect QA data if asked */
      if (fQAMultiplicityBefore) {
        for (const auto &dataVector : fSubEvent->GetInputDataBank()) {
          fQAMultiplicityBefore->Fill(variableContainer, dataVector.GetId(), dataVector.EqualizedWeight());
        }
      }
      /* store the equalized weights in the data vector bank according to equalization method */
      switch (fEqualizationMethod) {
        case Method::NONE:
          for (auto &dataVector : fSubEvent->GetInputDataBank()) {
            dataVector.SetEqualizedWeight(dataVector.EqualizedWeight());
          }
          break;
        case Method::AVERAGE:
          for (auto &dataVector : fSubEvent->GetInputDataBank()) {
            Long64_t bin = fInputHistograms->GetBin(variableContainer, dataVector.GetId());
            if (fInputHistograms->BinContentValidated(bin)) {
              Float_t average = fInputHistograms->GetBinContent(bin);
              /* let's handle the potential group weights usage */
              Float_t groupweight = 1.0;
              if (fUseChannelGroupsWeights) {
                groupweight = fInputHistograms->GetGrpBinContent(fInputHistograms->GetGrpBin(variableContainer,
                                                                                             dataVector.GetId()));
              } else {
                if (fHardCodedWeights) {
                  groupweight = fHardCodedWeights[dataVector.GetId()];
                }
              }
              if (fMinimumSignificantValue < average)
                dataVector.SetEqualizedWeight((dataVector.EqualizedWeight()/average)*groupweight);
              else
                dataVector.SetEqualizedWeight(0.0);
            } else {
              if (fQANotValidatedBin) fQANotValidatedBin->Fill(variableContainer, dataVector.GetId(), 1.0);
            }
          }
          break;
        case Method::WIDTH:
          for (auto &dataVector : fSubEvent->GetInputDataBank()) {
            Long64_t bin = fInputHistograms->GetBin(variableContainer, dataVector.GetId());
            if (fInputHistograms->BinContentValidated(bin)) {
              Float_t average =
                  fInputHistograms->GetBinContent(fInputHistograms->GetBin(variableContainer, dataVector.GetId()));
              Float_t width =
                  fInputHistograms->GetBinError(fInputHistograms->GetBin(variableContainer, dataVector.GetId()));
              /* let's handle the potential group weights usage */
              Float_t groupweight = 1.0;
              if (fUseChannelGroupsWeights) {
                groupweight = fInputHistograms->GetGrpBinContent(fInputHistograms->GetGrpBin(variableContainer,
                                                                                             dataVector.GetId()));
              } else {
                if (fHardCodedWeights) {
                  groupweight = fHardCodedWeights[dataVector.GetId()];
                }
              }
              if (fMinimumSignificantValue < average)
                dataVector.SetEqualizedWeight(
                    (fShift + fScale*(dataVector.EqualizedWeight() - average)/width)*groupweight);
              else
                dataVector.SetEqualizedWeight(0.0);
            } else {
              if (fQANotValidatedBin) fQANotValidatedBin->Fill(variableContainer, dataVector.GetId(), 1.0);
            }
          }
          break;
      }
      /* collect QA data if asked */
      if (fQAMultiplicityAfter) {
        for (const auto &dataVector : fSubEvent->GetInputDataBank()) {
          fQAMultiplicityAfter->Fill(variableContainer, dataVector.GetId(), dataVector.EqualizedWeight());
        }
      }
      return true;;
    case State::PASSIVE:
      /* we are in passive state waiting for proper conditions, no corrections applied */
      return false;
  }
}

/// Processes the correction data collection step
///
/// Data are always taken from the data bank from the equalized weights
/// allowing chaining of input data corrections so, caution must be taken to be
/// sure that, on initialising, weight and equalized weight match
/// Due to this structure as today it is not possible to split data collection
/// from correction processing. If so is required probably multiple equalization
/// structures should be included.
/// So this function only retures the proper value according to the status.
/// \return kTRUE if the correction step was applied
Bool_t GainEqualization::ProcessDataCollection(const double *variableContainer) {
  (void) variableContainer;
  switch (fState) {
    case State::CALIBRATION:return false;
    case State::APPLYCOLLECT:
      /* FALLTHRU */
    case State::APPLY:return true;
    case State::PASSIVE:return false;
  }

}

/// Report on correction usage
/// Correction step should incorporate its name in calibration
/// list if it is producing information calibration in the ongoing
/// step and in the apply list if it is applying correction in
/// the ongoing step.
/// \param calibrationList list containing the correction steps producing calibration information
/// \param applyList list containing the correction steps applying corrections
/// \return kTRUE if the correction step is being applied
Bool_t GainEqualization::ReportUsage(TList *calibrationList, TList *applyList) {
  switch (fState) {
    case State::CALIBRATION:
      /* we are collecting */
      calibrationList->Add(new TObjString(szCorrectionName));
      /* but not applying */
      return kFALSE;
      break;
    case State::APPLYCOLLECT:
      /* we are collecting */
      calibrationList->Add(new TObjString(szCorrectionName));
      /* FALLTHRU */
    case State::APPLY:
      /* and applying */
      applyList->Add(new TObjString(szCorrectionName));
      break;
    default:return kFALSE;
  }
  return kTRUE;
}
}
