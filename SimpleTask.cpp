//
// Created by Lukas Kreis on 13.11.17.
//

#include <TCanvas.h>
#include <TFile.h>
#include <ReducedEvent/AliReducedVarManager.h>
#include <TLegend.h>
#include "SimpleTask.h"
#include "DataContainerHelper.h"
SimpleTask::SimpleTask(std::string filelist, std::string treename) :
    in_tree_(this->MakeChain(filelist, treename)),
    reader_(in_tree_.get()) {}

void SimpleTask::Run() {
  AddEventVariable("CentralityVZERO");
  AddDataContainer("TPC_reference");
  AddDataContainer("TPC");
  AddDataContainer("VZEROA_reference");
  AddDataContainer("VZEROC_reference");
  AddDataContainer("FMDA_reference");
  AddDataContainer("FMDC_reference");
  AddDataContainer("ZDCA_reference");
  AddDataContainer("ZDCC_reference");
  AddDataContainer("ZDC");
  int events = 1;
  reader_.SetEntry(0);
  Initialize();
  Process();
  while (reader_.Next()) {
    events++;
    Process();
  }
  Finalize();
  std::cout << "number of events: " << events << std::endl;
}

void SimpleTask::Initialize() {
  std::vector<Qn::Axis> noaxes;
  eventaxes_.emplace_back("CentralityVZERO", std::vector<float>{0., 5., 10., 20., 30., 40., 50., 60., 70., 80.}, 1);

  auto tpc = *values_.at("TPC_reference");
  auto tpcetapt = *values_.at("TPC");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  auto fc = *values_.at("FMDC_reference");
  auto fa = *values_.at("FMDA_reference");
  auto zc = *values_.at("ZDCC_reference");
  auto za = *values_.at("ZDCA_reference");
  auto z = *values_.at("ZDC");

  auto scalar = [](std::vector<Qn::QVector> &a) -> double {
    return a[0].x(2)*a[1].x(2) + a[0].y(2)*a[1].y(2);
  };
  auto XY = [](std::vector<Qn::QVector> &a) {
    return a[0].x(1)*a[1].y(1);
  };
  auto YX = [](std::vector<Qn::QVector> &a) {
    return a[0].y(1)*a[1].x(1);
  };
  auto XX = [](std::vector<Qn::QVector> &a) {
    return a[0].x(1)*a[1].x(1);
  };
  auto YY = [](std::vector<Qn::QVector> &a) {
    return a[0].y(1)*a[1].y(1);
  };
  auto tpcpt =
      tpcetapt.Projection({tpcetapt.GetAxis("Pt")},
                          [](Qn::QVector &a, Qn::QVector &b) {
                            return (a + b).Normal(Qn::QVector::Normalization::QOVERM);
                          });
  AddCorrelation("tpcptvc", {tpcpt, vc}, eventaxes_, scalar);
  AddCorrelation("tpcptva", {tpcpt, vc}, eventaxes_, scalar);
  AddCorrelation("tpcvc", {tpc, vc}, eventaxes_, scalar);
  AddCorrelation("tpcva", {tpc, va}, eventaxes_, scalar);
  AddCorrelation("vavc", {va, vc}, eventaxes_, scalar);
  AddCorrelation("zazcxy", {za, zc}, eventaxes_, XY);
  AddCorrelation("zazcxx", {za, zc}, eventaxes_, XX);
  AddCorrelation("zazcyy", {za, zc}, eventaxes_, YY);
  AddCorrelation("zazcyx", {za, zc}, eventaxes_, YX);
  AddCorrelation("tpczaxx", {tpc, za}, eventaxes_, XX);
  AddCorrelation("tpczayy", {tpc, za}, eventaxes_, YY);
  AddCorrelation("tpczcxx", {tpc, zc}, eventaxes_, XX);
  AddCorrelation("tpczcyy", {tpc, zc}, eventaxes_, YY);
}

void SimpleTask::Process() {
  auto tpc = *values_.at("TPC_reference");
  auto tpcetapt = *values_.at("TPC");
  auto vc = *values_.at("VZEROC_reference");
  auto va = *values_.at("VZEROA_reference");
  auto fc = *values_.at("FMDC_reference");
  auto fa = *values_.at("FMDA_reference");
  auto zc = *values_.at("ZDCC_reference");
  auto za = *values_.at("ZDCA_reference");
  auto z = *values_.at("ZDC");

  std::vector<float> eventparameters;
  eventparameters.push_back(*eventvalues_.at("CentralityVZERO"));
  auto eventbin = Qn::CalculateEventBin(eventaxes_, eventparameters);
  for (auto bin : eventbin) {
    if (bin==-1) return;
  }

  auto tpcpt =
      tpcetapt.Projection({tpcetapt.GetAxis("Pt")},
                          [](Qn::QVector &a, Qn::QVector &b) {
                            return (a + b).Normal(Qn::QVector::Normalization::QOVERM);
                          });

  correlations_.at("tpcptva").Fill({tpcpt, va}, eventbin);
  correlations_.at("tpcptvc").Fill({tpcpt, vc}, eventbin);
  correlations_.at("tpcva").Fill({tpc, va}, eventbin);
  correlations_.at("tpcvc").Fill({tpc, vc}, eventbin);
  correlations_.at("vavc").Fill({va, vc}, eventbin);
  correlations_.at("tpczaxx").Fill({tpc, za}, eventbin);
  correlations_.at("tpczayy").Fill({tpc, za}, eventbin);
  correlations_.at("tpczcxx").Fill({tpc, zc}, eventbin);
  correlations_.at("tpczcyy").Fill({tpc, zc}, eventbin);
  correlations_.at("zazcxx").Fill({za, zc}, eventbin);
  correlations_.at("zazcyy").Fill({za, zc}, eventbin);
  correlations_.at("zazcyx").Fill({za, zc}, eventbin);
  correlations_.at("zazcxy").Fill({za, zc}, eventbin);

}

void SimpleTask::Finalize() {
//
  TList list;
  auto outputfile = TFile::Open("correlations.root", "RECREATE");

  for (const auto &correlation : correlations_) {
    correlation.second.GetCorrelation().Write(correlation.first.data());
  }
  outputfile->Close();


  auto tpcvc = correlations_.at("tpcvc").GetCorrelation();
  auto tpcva = correlations_.at("tpcva").GetCorrelation();
//  auto tpcptvc = correlations_.at("tpcptvc").GetCorrelation();
//  auto tpcptva = correlations_.at("tpcptva").GetCorrelation();
  auto vavc = correlations_.at("vavc").GetCorrelation();
//  auto zazcxx = correlations_.at("zazcxx").GetCorrelation();
//  auto zazcyy = correlations_.at("zazcyy").GetCorrelation();
//  auto zazcxy = correlations_.at("zazcxy").GetCorrelation();
//  auto zazcyx = correlations_.at("zazcyx").GetCorrelation();
//  auto tpcz = correlations_.at("tpcz").GetCorrelation();
//  auto tpczz = correlations_.at("tpczz").GetCorrelation();
//  auto tpczaxx = correlations_.at("tpczaxx").GetCorrelation();
//  auto tpczayy = correlations_.at("tpczayy").GetCorrelation();
//  auto tpczcxx = correlations_.at("tpczcxx").GetCorrelation();
//  auto tpczcyy = correlations_.at("tpczcyy").GetCorrelation();

  auto multiply = [](Qn::Statistics a, Qn::Statistics b) {
    return a*b;
  };

  auto divide = [](Qn::Statistics a, Qn::Statistics b) {
    return a/b;
  };

  auto sqrt = [](Qn::Statistics a) {
    return a.Sqrt();
  };

  auto add = [](Qn::Statistics a, Qn::Statistics b) {
    return a + b;
  };

  auto rvatpcvc = tpcva.Apply(vavc, multiply).Apply(tpcvc, divide).Map(sqrt);
  auto v2tpcva = tpcva.Apply(rvatpcvc, divide);
//
//  auto v1tpcza = tpczaxx.Apply(zazcxx.Map(sqrt), divide).Apply(tpczayy.Apply(zazcyy.Map(sqrt), divide),
//                                                               add).Map([](Qn::Statistics a) {
//    return a*(1/TMath::Sqrt(2));
//  });
//  auto v1tpczc = tpczcxx.Apply(zazcxx.Map(sqrt), divide).Apply(tpczcyy.Apply(zazcyy.Map(sqrt), divide),
//                                                               add).Map([](Qn::Statistics a) {
//    return a*(-1/TMath::Sqrt(2));
//  });
//  auto v1tpcz = v1tpcza.Apply(v1tpczc, add).Map([](Qn::Statistics a) { return a*(1.0/2.0); });
//
//  auto v2tpcptva = tpcptva.Apply(rvatpcvc, divide);
//  auto v2tpcptvacent10 = v2tpcptva.Select({"CentralityVZERO", {10, 20}, 23});
//  v2tpcptvacent10 = v2tpcptvacent10.Projection({v2tpcptva.GetAxes().at(1)}, add);
//  auto v2tpcptvacent20 =
//      v2tpcptva.Select({"CentralityVZERO", {20, 30}, 23}).Projection({v2tpcptva.GetAxes().at(1)}, add);
//  auto v2tpcptvacent30 =
//      v2tpcptva.Select({"CentralityVZERO", {30, 40}, 23}).Projection({v2tpcptva.GetAxes().at(1)}, add);
//
//  auto *c3 = new TCanvas("c3", "c3", 800, 600);
//  auto t = Qn::DataToProfileGraph(v1tpcza);
//  t->Draw();
//  t->SetMinimum(-0.0005);
//  t->SetMaximum(0.0005);
//  Qn::DataToProfileGraph(v1tpczc)->Draw("LP");
//  auto c = Qn::DataToProfileGraph(v1tpcz);
//  c->SetLineColor(kRed);
//  c->Draw("LP");
//  c3->SaveAs("v1.pdf");
//
//  auto *c1 = new TCanvas("c1", "c1", 1800, 600);
//  auto v21 = Qn::DataToProfileGraph(v2tpcptvacent10);
//  auto v22 = Qn::DataToProfileGraph(v2tpcptvacent20);
//  auto v23 = Qn::DataToProfileGraph(v2tpcptvacent30);
//  v21->SetLineColor(kBlue);
//  v22->SetLineColor(kRed);
//  v23->Draw("APL");
//  v22->Draw("PL");
//  v21->Draw("PL");
//  c1->SaveAs("test.root");
//  c1->SaveAs("test.pdf");
//
  auto *c4 = new TCanvas("c4", "c4", 800, 600);
  Qn::DataToProfileGraph(v2tpcva)->Draw();
  c4->SaveAs("v2tpcva.pdf");
//
//  auto *c2 = new TCanvas("c2", "c2", 800, 600);
//  c2->cd();
//  auto gzazcxx = Qn::DataToProfileGraph(zazcxx);
//  gzazcxx->SetLineColor(kBlue);
//  gzazcxx->Draw("ALP");
//  gzazcxx->SetMaximum(0.001);
//  gzazcxx->SetMinimum(-0.0014);
//  auto gzazcyy = Qn::DataToProfileGraph(zazcyy);
//  gzazcyy->SetLineColor(kRed);
//  gzazcyy->Draw("LP");
//  auto gzazcxy = Qn::DataToProfileGraph(zazcxy);
//  gzazcxy->SetLineColor(kGreen + 1);
//  gzazcxy->SetLineWidth(2);
//  gzazcxy->Draw("LP");
//  auto gzazcyx = Qn::DataToProfileGraph(zazcyx);
//  gzazcyx->Draw("LP");
//  auto *legend = new TLegend(0.70, 0.7, 0.85, 0.85);
//  legend->AddEntry(gzazcxx, "axcx", "lp");
//  legend->AddEntry(gzazcyy, "aycy", "lp");
//  legend->AddEntry(gzazcyx, "aycx", "lp");
//  legend->AddEntry(gzazcxy, "axcy", "lp");
//  legend->Draw();
//  c2->SaveAs("xy.pdf");
}

std::unique_ptr<TChain> SimpleTask::MakeChain(std::string filename, std::string treename) {
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
void SimpleTask::AddDataContainer(std::string name) {
  TTreeReaderValue<Qn::DataContainerQVector>
      value(reader_, name.data());
  values_.insert(std::make_pair(name, value));
}
void SimpleTask::AddEventVariable(std::string name) {
  TTreeReaderValue<float> value(reader_, name.data());
  eventvalues_.insert(std::make_pair(name, value));
}

void SimpleTask::AddCorrelation(std::string name,
                                std::vector<Qn::DataContainerQVector> containers,
                                std::vector<Qn::Axis> axes,
                                std::function<double(std::vector<Qn::QVector> &)> lambda) {
  Qn::Correlation correlation(std::move(containers), std::move(axes), std::move(lambda));
  correlations_.emplace(name, correlation);
}