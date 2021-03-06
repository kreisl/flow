

#include <random>
#include "gtest/gtest.h"
#include "CorrectionManager.h"

TEST(CorrectionUnitTest, Correction) {
  int kNEvents = 1000;
  int kNTracks = 1000;
  Qn::CorrectionManager manager;
  auto file = new TFile("testoutput.root", "RECREATE");
  file->cd();
  auto tree = new TTree();
  tree->SetName("QVectors");
  manager.SetFillCalibrationQA(true);
  manager.SetFillValidationQA(true);
  manager.SetFillOutputTree(true);
  enum variables {
    kPhi,
    kAxis1,
    kCentrality
  };
  manager.AddVariable("phi",kPhi,1);
  manager.AddVariable("centrality",kCentrality,1);
  manager.AddVariable("axis1",kAxis1,1);

  manager.AddCorrectionAxis({"centrality",100,0.,100.});
  manager.AddEventVariable("centrality");

  Qn::AxisD axis("axis1",10,0,100);
  manager.AddDetector("TEST",Qn::DetectorType::TRACK,"phi","Ones",{axis},
      {1,2},Qn::QVector::Normalization::M);
  manager.AddHisto1D("TEST",{"phi",100,0,2*TMath::Pi()});

//  auto test_configuration = [](Qn::SubEvent *config){
//    auto rec = new Qn::Recentering();
//    rec->SetApplyWidthEqualization(true);
//    config->AddCorrectionOnQnVector(rec);
//  };
  Qn::Recentering rec;
  rec.SetApplyWidthEqualization(true);

//  manager.SetCorrectionSteps("TEST",test_configuration);
  manager.AddCorrectionOnQnVector("TEST",rec);
  manager.SetOutputQVectors("TEST",{Qn::QVector::CorrectionStep::PLAIN, Qn::QVector::CorrectionStep::RECENTERED});

  manager.SetCalibrationInputFileName("correctionfile.root");
  manager.ConnectOutputTree(tree);
  manager.InitializeOnNode();
  manager.SetCurrentRunName("run1");
  auto var = manager.GetVariableContainer();
  std::mt19937 gen(0);
  std::uniform_real_distribution<double> centrality(0.,100.);
  std::uniform_real_distribution<double> axis1(0.,100.);
  std::uniform_real_distribution<double> phi(0.,2*TMath::Pi());
  for (int i = 0; i < kNEvents; ++i) {
    manager.Reset();
    auto c = centrality(gen);;
    var[kCentrality] = c;
    if(!manager.ProcessEvent()) continue;
    for (int j = 0; j < kNTracks; ++j) {
      var[kAxis1] = axis1(gen);
      var[kPhi] = phi(gen);
      manager.FillTrackingDetectors();
    }
    manager.ProcessCorrections();
  }
  manager.Finalize();

  file->cd();
  manager.GetCorrectionList()->Write("CorrectionHistograms",TObject::kSingleKey);
  manager.GetCorrectionQAList()->Write("QA",TObject::kSingleKey);
  tree->Write();
  file->Write();
  file->Close();
}