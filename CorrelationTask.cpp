//
// Created by Lukas Kreis on 13.11.17.
//
#include <utility>
#include <TCanvas.h>
#include <TFile.h>
#include <TLegend.h>
#include <ReducedEvent/AliReducedVarManager.h>
#include <TTreeReaderValue.h>

#include "CorrelationTask.h"
#include "Base/DataContainerHelper.h"

CorrelationTask::CorrelationTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(std::move(filelist), treename)),
    reader_(new TTreeReader(in_tree_.get())) {}

void CorrelationTask::Configure(Qn::CorrelationManager &a) {
  // Correlation functions
  auto scalar = [](const std::vector<Qn::QVector> &a) {
    return a[0].x(2)*a[1].x(2) + a[0].y(2)*a[1].y(2);
  };
  auto scalarsign = [](const std::vector<Qn::QVector> &a) {
    return a[0].x(2)*a[1].x(2) - a[0].y(2)*a[1].y(2);
  };

  // Set file names
  a.SetESECalibrationFile("ese.root");
  a.SetOutputFile("corr.root");

  // ESE Configuration
  a.AddESE("ZDCA", 1, 800);

  // Add Detectors for Correlation
  a.AddQVectors("TPC, TPC_R, V0A, V0C, T0A, T0C, ZDCA, ZDCC");
//  a.AddQVectors("TPC_R, V0A");

  // Add Eventvariables for correlation
  // Use Qn::Axis
  a.AddEventVariable({"CentralityV0M", {0., 5., 10., 20., 30., 40., 50., 60., 70.}});
//  a.AddEventVariable({"VTXZ", 10,-10,10});

  a.AddProjection("TPC","TPC_RR","");
  // Add Correlation
//  a.AddCorrelation("TPC_RR_TPC_RR","TPC_RR,TPC_RR",scalar,0,Qn::Sampler::Method::NONE);
//  a.AddCorrelation("TPCPT_V0A", "TPC, V0A", scalarsign, 10, Qn::Sampler::Method::BOOTSTRAP);
  a.AddCorrelation("TPCPT_V0C", "TPC, V0C", scalarsign, 10, Qn::Sampler::Method::BOOTSTRAP);
  a.AddCorrelation("TPC_V0A", "TPC_R, V0A", scalarsign, 10, Qn::Sampler::Method::BOOTSTRAP);
//  a.AddCorrelation("TPC_V0C", "TPC_R, V0C", scalarsign, 10, Qn::Sampler::Method::BOOTSTRAP);
//  a.AddCorrelation("V0A_V0C", "V0A, V0C", scalar, 10, Qn::Sampler::Method::BOOTSTRAP);
//  a.AddCorrelation("ZDCAC_XX","ZDCA, ZDCC",[](const std::vector<Qn::QVector> &a){return -a[0].x(1)*a[1].x(1);});
//  a.AddCorrelation("ZDCAC_YY","ZDCA, ZDCC",[](const std::vector<Qn::QVector> &a){return a[0].y(1)*a[1].y(1);});
//  a.AddCorrelation("ZDCAC_YX","ZDCA, ZDCC",[](const std::vector<Qn::QVector> &a){return a[0].y(1)*a[1].x(1);});
//  a.AddCorrelation("ZDCAC_XY","ZDCA, ZDCC",[](const std::vector<Qn::QVector> &a){return a[0].x(1)*a[1].y(1);});
}

void CorrelationTask::Run() {
  Qn::CorrelationManager a(reader_, in_tree_->GetEntries());
  in_tree_->LoadTree(0); // prevents weird TTree errors
  Configure(a);
  int events = 1;
  reader_->SetEntry(0); //  first entry needed to access configuration of DataContainers in the input file
  a.Initialize();
  in_tree_->LoadTree(0); // prevents weird TTree errors
  while (reader_->Next()) {
    events++;
    a.Process();
  }
  a.Finalize();
  std::cout << "number of analyzed events: " << events << std::endl;
}

std::unique_ptr<TChain> CorrelationTask::MakeChain(std::string filename, std::string treename) {
  std::unique_ptr<TChain> chain(new TChain(treename.data()));
  std::ifstream in;
  in.open(filename);
  std::string line;
  std::cout << "files in TChain:" << "\n";
  while ((in >> line).good()) {
    if (!line.empty()) {
      chain->AddFile(line.data());
      std::cout << line << std::endl;
    }
  }
  return chain;
}
