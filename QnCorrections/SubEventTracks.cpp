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

/// \file QnCorrectionsDetectorConfigurationTracks.cxx
/// \brief Implementation of the track detector configuration class

#include "CorrectionProfileComponents.h"
#include "SubEventTracks.h"
#include "CorrectionLog.h"
#include "ROOT/RMakeUnique.hxx"

/// \cond CLASSIMP
ClassImp(Qn::SubEventTracks);
/// \endcond
namespace Qn {

/// Normal constructor
/// Allocates the data vector bank.
/// \param name the name of the detector configuration
/// \param eventClassesVariables the set of event classes variables
/// \param nNoOfHarmonics the number of harmonics that must be handled
/// \param harmonicMap an optional ordered array with the harmonic numbers
SubEventTracks::SubEventTracks(const char *name,
                               EventClassVariablesSet *eventClassesVariables,
                               Int_t nNoOfHarmonics,
                               Int_t *harmonicMap) :
    SubEvent(name, eventClassesVariables, nNoOfHarmonics, harmonicMap) {
}

/// Stores the framework manager pointer
/// Orders the base class to store the correction manager and informs
/// the Qn vector corrections they are now attached to the framework
/// \param manager the framework manager
void SubEventTracks::AttachCorrectionsManager(CorrectionCalculator *manager) {
  fCorrectionsManager = manager;

  if (manager!=nullptr) {
    for (auto &correction : fQnVectorCorrections) {
      correction->AttachedToFrameworkManager();
    }
  }
}

/// Asks for support data structures creation
///
/// The input data vector bank is allocated and the request is
/// transmitted to the Q vector corrections.
void SubEventTracks::CreateSupportDataStructures() {

  /* this is executed in the remote node so, allocate the data bank */
  fDataVectorBank.reserve(Qn::SubEvent::INITIALSIZE);
  for (auto &correction : fQnVectorCorrections) {
    correction->CreateSupportDataStructures();
  }
}

/// Asks for support histograms creation
///
/// The request is transmitted to the Q vector corrections.
///
/// A new histograms list is created for the detector configuration and incorporated
/// to the passed list. Then the new list is passed to the corrections.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
Bool_t SubEventTracks::CreateSupportHistograms(TList *list) {
  Bool_t retValue = kTRUE;
  auto detectorConfigurationList = new TList();
  detectorConfigurationList->SetName(this->GetName());
  detectorConfigurationList->SetOwner(kTRUE);
  for (auto &correction : fQnVectorCorrections) {
    retValue = retValue && correction->CreateSupportHistograms(detectorConfigurationList);
  }
  /* if list is empty delete it if not incorporate it */
  if (detectorConfigurationList->GetEntries()!=0) {
    list->Add(detectorConfigurationList);
  } else {
    delete detectorConfigurationList;
  }
  return retValue;
}

/// Asks for QA histograms creation
///
/// The request is transmitted to the Q vector corrections.
///
/// A new histograms list is created for the detector and incorporated
/// to the passed list. Then the new list is passed to the corrections.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
Bool_t SubEventTracks::CreateQAHistograms(TList *list) {
  Bool_t retValue = kTRUE;
  TList *detectorConfigurationList = new TList();
  detectorConfigurationList->SetName(this->GetName());
  detectorConfigurationList->SetOwner(kTRUE);

  /* the own QA average Qn vector components histogram */
  fQAQnAverageHistogram = std::make_unique<CorrectionProfileComponents>(
      Form("%s %s", szQAQnAverageHistogramName, this->GetName()),
      Form("%s %s", szQAQnAverageHistogramName, this->GetName()),
      this->GetEventClassVariablesSet());

  /* get information about the configured harmonics to pass it for histogram creation */
  Int_t nNoOfHarmonics = this->GetNoOfHarmonics();
  auto harmonicsMap = new Int_t[nNoOfHarmonics];
  this->GetHarmonicMap(harmonicsMap);
  fQAQnAverageHistogram->CreateComponentsProfileHistograms(detectorConfigurationList, nNoOfHarmonics, harmonicsMap);
  delete[] harmonicsMap;

  for (auto &correction : fQnVectorCorrections) {
    retValue = retValue && correction->CreateQAHistograms(detectorConfigurationList);
  }
  /* if list is empty delete it if not incorporate it */
  if (detectorConfigurationList->GetEntries()!=0) {
    list->Add(detectorConfigurationList);
  } else {
    delete detectorConfigurationList;
  }
  return retValue;
}

/// Asks for non validated entries QA histograms creation
///
/// The request is transmitted to the Q vector corrections.
///
/// A new histograms list is created for the detector and incorporated
/// to the passed list. Then the new list is passed to the corrections.
/// \param list list where the histograms should be incorporated for its persistence
/// \return kTRUE if everything went OK
Bool_t SubEventTracks::CreateNveQAHistograms(TList *list) {
  Bool_t retValue = kTRUE;
  auto detectorConfigurationList = new TList();
  detectorConfigurationList->SetName(this->GetName());
  detectorConfigurationList->SetOwner(kTRUE);
  for (auto &correction : fQnVectorCorrections) {
    retValue = retValue && correction->CreateNveQAHistograms(detectorConfigurationList);
  }
  /* if list is empty delete it if not incorporate it */
  if (detectorConfigurationList->GetEntries()!=0) {
    list->Add(detectorConfigurationList);
  } else {
    delete detectorConfigurationList;
  }
  return retValue;
}

/// Asks for attaching the needed input information to the correction steps
///
/// The detector list is extracted from the passed list and then
/// the request is transmitted to the Q vector corrections with the found list.
/// \param list list where the input information should be found
/// \return kTRUE if everything went OK
Bool_t SubEventTracks::AttachCorrectionInputs(TList *list) {
  auto detectorConfigurationList = (TList *) list->FindObject(this->GetName());
  if (detectorConfigurationList!=nullptr) {
    Bool_t retValue = kTRUE;
    for (auto &correction : fQnVectorCorrections) {
      retValue = retValue && correction->AttachInput(detectorConfigurationList);
    }
    return retValue;
  }
  return kFALSE;
}

/// Perform after calibration histograms attach actions
/// It is used to inform the different correction step that
/// all conditions for running the network are in place so
/// it is time to check if their requirements are satisfied
///
/// The request is transmitted to the Q vector corrections
void SubEventTracks::AfterInputsAttachActions() {
  /* now propagate it to Q vector corrections */
  for (auto &correction : fQnVectorCorrections) {
    correction->AfterInputsAttachActions();
  }
}

/// Fills the QA plain Qn vector average components histogram
/// \param variableContainer pointer to the variable content bank
void SubEventTracks::FillQAHistograms(const double *variableContainer) {
  if (fQAQnAverageHistogram!=nullptr) {
    Int_t harmonic = fPlainQnVector.GetFirstHarmonic();
    while (harmonic!=-1) {
      fQAQnAverageHistogram->FillX(harmonic, variableContainer, fPlainQnVector.Qx(harmonic));
      fQAQnAverageHistogram->FillY(harmonic, variableContainer, fPlainQnVector.Qy(harmonic));
      harmonic = fPlainQnVector.GetNextHarmonic(harmonic);
    }
  }
}

/// Include the the list of Qn vector associated to the detector configuration
/// into the passed list
///
/// A new list is created for the detector configuration and incorporated
/// to the passed list.
///
/// Always includes first the fully corrected Qn vector,
/// and then includes the plain Qn vector and asks to the different correction
/// steps to include their partially corrected Qn vectors.
/// The check if we are already there is because it could be late information
/// about the process name and then the correction histograms could still not
/// be attached and the constructed list does not contain the final Qn vectors.
/// \param list list where the corrected Qn vector should be added
void SubEventTracks::IncludeQnVectors(TList *list) {

  /* we check whether we are already there and if so we clean it and go again */
  Bool_t bAlreadyThere;
  TList *detectorConfigurationList;
  if (list->FindObject(this->GetName())) {
    detectorConfigurationList = (TList *) list->FindObject(this->GetName());
    detectorConfigurationList->Clear();
    bAlreadyThere = kTRUE;
  } else {
    detectorConfigurationList = new TList();
    detectorConfigurationList->SetName(this->GetName());
    detectorConfigurationList->SetOwner(kFALSE);
    bAlreadyThere = kFALSE;
  }

  detectorConfigurationList->Add(&fCorrectedQnVector);
  detectorConfigurationList->Add(&fPlainQnVector);
  for (auto &correction : fQnVectorCorrections) {
    correction->IncludeCorrectedQnVector(detectorConfigurationList);
  }
  if (!bAlreadyThere)
    list->Add(detectorConfigurationList);
}

/// Include only one instance of each input correction step
/// in execution order
///
/// There are not input correction so we do nothing
/// \param list list where the correction steps should be incorporated
void SubEventTracks::FillOverallInputCorrectionStepList(std::set<CorrectionStep *, CompareSteps> &set) const {
  (void) set;
}

/// Include only one instance of each Qn vector correction step
/// in execution order
///
/// The request is transmitted to the set of Qn vector corrections
/// \param list list where the correction steps should be incorporated
void SubEventTracks::FillOverallQnVectorCorrectionStepList(std::set<CorrectionStep *, CompareSteps> &set) const {
  fQnVectorCorrections.FillOverallCorrectionsList(set);
}

/// Provide information about assigned corrections
///
/// We create three list which items they own, incorporate info from the
/// correction steps and add them to the passed list
/// \param steps list for incorporating the list of assigned correction steps
/// \param calib list for incorporating the list of steps in calibrating status
/// \param apply list for incorporating the list of steps in applying status
void SubEventTracks::ReportOnCorrections(TList *steps, TList *calib, TList *apply) const {
  TList *mysteps = new TList();
  mysteps->SetOwner(kTRUE);
  mysteps->SetName(GetName());
  TList *mycalib = new TList();
  mycalib->SetOwner(kTRUE);
  mycalib->SetName(GetName());
  TList *myapply = new TList();
  myapply->SetOwner(kTRUE);
  myapply->SetName(GetName());

  /* incorporate Qn vector corrections */
  Bool_t keepIncorporating = kTRUE;
  for (auto &correction : fQnVectorCorrections) {
    mysteps->Add(new TObjString(correction->GetName()));
    /* incorporate additional info if the step will be reached */
    if (keepIncorporating) {
      Bool_t keep = correction->ReportUsage(mycalib, myapply);
      keepIncorporating = keepIncorporating && keep;
    }
  }
  steps->Add(mysteps);
  calib->Add(mycalib);
  apply->Add(myapply);
}

}