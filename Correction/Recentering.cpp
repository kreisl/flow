#include <memory>

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

/// \file QnCorrectionsQnVectorRecentering.cxx
/// \brief Implementation of procedures for Qn vector recentering.
#include "EventClassVariablesSet.h"
#include "CorrectionProfileComponents.h"
#include "CorrectionHistogramSparse.h"
#include "Recentering.h"
#include "SubEvent.h"
#include "ROOT/RMakeUnique.hxx"

/// \cond CLASSIMP
ClassImp(Qn::Recentering);
/// \endcond
namespace Qn {
const Int_t Recentering::fDefaultMinNoOfEntries = 2;
const char *Recentering::szCorrectionName = "Recentering and width equalization";
const char *Recentering::szSupportHistogramName = "Qn";
const char *Recentering::szCorrectedQnVectorName = "rec";
const char *Recentering::szQANotValidatedHistogramName = "Rec NvE";
const char *Recentering::szQAQnAverageHistogramName = "Rec Qn avg ";

/// Default constructor
/// Passes to the base class the identity data for the recentering and width equalization correction step
Recentering::Recentering() :
    CorrectionOnQvector(szCorrectionName, szPriority) {
  fApplyWidthEqualization = kFALSE;
  fMinNoOfEntriesToValidate = fDefaultMinNoOfEntries;
}

/// Asks for support data structures creation
///
/// Creates the recentered Qn vector
void Recentering::CreateSupportDataStructures() {
  fInputQnVector = fSubEvent->GetPreviousCorrectedQnVector(this);
  fCorrectedQnVector = std::make_unique<QVector>(*fInputQnVector);
  fCorrectedQnVector->SetCorrectionStep(QVector::RECENTERED);
}

/// Asks for support histograms creation
///
/// Allocates the histogram objects and creates the calibration histograms.
/// The histograms are constructed with standard deviation error calculation
/// for the proper behavior of optional gain equalization step.
///
/// Process concurrency requires Calibration Histograms creation for all c
/// concurrent processes but not for Input Histograms so, we delete previously
/// allocated ones.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
void Recentering::AttachSupportHistograms(TList *list) {
  auto hname = std::string(szSupportHistogramName) + "_" + fSubEvent->GetName();
  fInputHistograms = std::make_unique<CorrectionProfileComponents>(hname, fSubEvent->GetEventClassVariablesSet(),
                                                                   CorrectionHistogramBase::ErrorMode::SPREAD);
  fInputHistograms->SetNoOfEntriesThreshold(fMinNoOfEntriesToValidate);
  fCalibrationHistograms = std::make_unique<CorrectionProfileComponents>(hname, fSubEvent->GetEventClassVariablesSet(),
                                                                         CorrectionHistogramBase::ErrorMode::SPREAD);
  /* get information about the configured harmonics to pass it for histogram creation */
  Int_t nNoOfHarmonics = fSubEvent->GetNoOfHarmonics();
  auto harmonicsMap = new Int_t[nNoOfHarmonics];
  fSubEvent->GetHarmonicMap(harmonicsMap);
  fCalibrationHistograms->CreateComponentsProfileHistograms(list, nNoOfHarmonics, harmonicsMap);
  delete[] harmonicsMap;
}

/// Attaches the needed input information to the correction step
/// \param list list where the inputs should be found
/// \return kTRUE if everything went OK
void Recentering::AttachInput(TList *list) {
  if (fInputHistograms->AttachHistograms(list)) {
    fState = State::APPLYCOLLECT;
  }
}

/// Asks for QA histograms creation
///
/// Allocates the histogram objects and creates the QA histograms.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
void Recentering::AttachQAHistograms(TList *list) {
  auto hname = std::string(szQAQnAverageHistogramName) + "_" + fSubEvent->GetName();
  fQAQnAverageHistogram = std::make_unique<CorrectionProfileComponents>(hname, fSubEvent->GetEventClassVariablesSet());
  /* get information about the configured harmonics to pass it for histogram creation */
  auto nNoOfHarmonics = fSubEvent->GetNoOfHarmonics();
  auto harmonicsMap = new Int_t[nNoOfHarmonics];
  fSubEvent->GetHarmonicMap(harmonicsMap);
  fQAQnAverageHistogram->CreateComponentsProfileHistograms(list, nNoOfHarmonics, harmonicsMap);
  delete[] harmonicsMap;
}

/// Asks for non validated entries QA histograms creation
///
/// Allocates the histogram objects and creates the non validated entries QA histograms.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
void Recentering::AttachNveQAHistograms(TList *list) {
  auto hname = std::string(szQANotValidatedHistogramName) + "_" + fSubEvent->GetName();
  fQANotValidatedBin = std::make_unique<CorrectionHistogramSparse>(hname, fSubEvent->GetEventClassVariablesSet());
  fQANotValidatedBin->CreateHistogram(list);
}

/// Processes the correction step
///
/// Pure virtual function
/// \return kTRUE if the correction step was applied
bool Recentering::ProcessCorrections(const double *variableContainer) {
  int harmonic;
  switch (fState) {
    case State::CALIBRATION:
      /* collect the data needed to further produce correction parameters if the current Qn vector is good enough */
      /* we have not perform any correction yet */
      return false;
    case State::APPLYCOLLECT:
      /* collect the data needed to further produce correction parameters if the current Qn vector is good enough */
      /* and proceed to ... */
    case State::APPLY: /* apply the correction if the current Qn vector is good enough */
      if (fSubEvent->GetCurrentQnVector()->IsGoodQuality()) {
        /* we get the properties of the current Qn vector but its name */
        fCorrectedQnVector->SetCurrentEvent(*fSubEvent->GetCurrentQnVector());
        harmonic = fSubEvent->GetCurrentQnVector()->GetFirstHarmonic();
        /* let's check the correction histograms */
        Long64_t bin = fInputHistograms->GetBin(variableContainer);
        if (fInputHistograms->BinContentValidated(bin)) {
          /* correction information validated */
          while (harmonic!=-1) {
            Float_t widthX = 1.0;
            Float_t widthY = 1.0;
            if (fApplyWidthEqualization) {
              widthX = fInputHistograms->GetXBinError(harmonic, bin);
              widthY = fInputHistograms->GetYBinError(harmonic, bin);
            }
            fCorrectedQnVector->SetQx(harmonic, (fSubEvent->GetCurrentQnVector()->Qx(harmonic)
                - fInputHistograms->GetXBinContent(harmonic, bin))
                /widthX);
            fCorrectedQnVector->SetQy(harmonic, (fSubEvent->GetCurrentQnVector()->Qy(harmonic)
                - fInputHistograms->GetYBinContent(harmonic, bin))
                /widthY);
            harmonic = fSubEvent->GetCurrentQnVector()->GetNextHarmonic(harmonic);
          }
        } /* correction information not validated, we leave the Q vector untouched */
        else {
          if (fQANotValidatedBin) fQANotValidatedBin->Fill(variableContainer, 1.0);
        }
      } else {
        /* not done! input vector with bad quality */
        fCorrectedQnVector->SetGood(kFALSE);
      }
      /* and update the current Qn vector */
      fSubEvent->UpdateCurrentQnVector(*fCorrectedQnVector, QVector::CorrectionStep::RECENTERED);
      return true;
    case State::PASSIVE:
      /* we are in passive state waiting for proper conditions, no corrections applied */
      return false;
  }
}

/// Processes the correction step data collection
///
/// Pure virtual function
/// \return kTRUE if the correction step was applied
bool Recentering::ProcessDataCollection(const double *variableContainer) {
  int harmonic;
  switch (fState) {
    case State::CALIBRATION:
      /* collect the data needed to further produce correction parameters if the current Qn vector is good enough */
      if (fInputQnVector->IsGoodQuality()) {
        harmonic = fInputQnVector->GetFirstHarmonic();
        while (harmonic!=-1) {
          fCalibrationHistograms->FillX(harmonic, variableContainer, fInputQnVector->Qx(harmonic));
          fCalibrationHistograms->FillY(harmonic, variableContainer, fInputQnVector->Qy(harmonic));
          harmonic = fInputQnVector->GetNextHarmonic(harmonic);
        }
      }
      /* we have not perform any correction yet */
      return false;
    case State::APPLYCOLLECT:
      /* collect the data needed to further produce correction parameters if the current Qn vector is good enough */
      if (fInputQnVector->IsGoodQuality()) {
        harmonic = fInputQnVector->GetFirstHarmonic();
        while (harmonic!=-1) {
          fCalibrationHistograms->FillX(harmonic, variableContainer, fInputQnVector->Qx(harmonic));
          fCalibrationHistograms->FillY(harmonic, variableContainer, fInputQnVector->Qy(harmonic));
          harmonic = fInputQnVector->GetNextHarmonic(harmonic);
        }
      }
      /* and proceed to ... */
      /* FALLTHRU */
    case State::APPLY: /* apply the correction if the current Qn vector is good enough */
      /* provide QA info if required */
      if (fQAQnAverageHistogram) {
        harmonic = fCorrectedQnVector->GetFirstHarmonic();
        while (harmonic!=-1) {
          fQAQnAverageHistogram->FillX(harmonic, variableContainer, fCorrectedQnVector->Qx(harmonic));
          fQAQnAverageHistogram->FillY(harmonic, variableContainer, fCorrectedQnVector->Qy(harmonic));
          harmonic = fCorrectedQnVector->GetNextHarmonic(harmonic);
        }
      }
      return true;
    case State::PASSIVE:
      /* we are in passive state waiting for proper conditions, no corrections applied */
      return false;
  }
}

/// Clean the correction to accept a new event
void Recentering::ClearCorrectionStep() {
  fCorrectedQnVector->Reset();
}

/// Reports if the correction step is being applied
/// Returns TRUE if in the proper state for applying the correction step
/// \return TRUE if the correction step is being applied
bool Recentering::IsBeingApplied() const {
  switch (fState) {
    case State::CALIBRATION: return false;
    case State::APPLYCOLLECT:
      /* FALLTHRU */
    case State::APPLY: return true;
    case State::PASSIVE: return false;
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
Bool_t Recentering::ReportUsage(TList *calibrationList, TList *applyList) {
  switch (fState) {
    case State::CALIBRATION:calibrationList->Add(new TObjString(szCorrectionName));
      return false;
    case State::APPLYCOLLECT:calibrationList->Add(new TObjString(szCorrectionName));
      /* FALLTHRU */
    case State::APPLY:applyList->Add(new TObjString(szCorrectionName));
      return true;
    case State::PASSIVE: return false;
  }
}

}