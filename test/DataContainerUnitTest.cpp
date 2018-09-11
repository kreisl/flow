//
// Created by Lukas Kreis on 30.01.18.
//




#include <gtest/gtest.h>
#include "Base/DataContainer.h"
#include <TList.h>
#include <TFile.h>
#include <TRandom3.h>
#include <TProfile.h>

//TEST(DataContainerTest, Copy) {
//  Qn::DataContainer<Qn::QVector> container;
//  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  Qn::DataContainer<Qn::QVector> copy(container);
//  EXPECT_EQ(copy.size(), container.size());
//}
//
//TEST(DataContainerTest, AddAxes) {
//  Qn::DataContainer<Qn::QVector> container;
//  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  EXPECT_EQ(100, container.size());
//}
//
TEST(DataContainerTest, Projection) {
  Qn::DataContainerF container;
  container.AddAxes({{"a1", 30, 0, 10}, {"a2", 20, 0, 10}, {"a3", 10, 0, 10}});
  for (auto &bin : container) {
    bin = 1;
  }
//  auto integration = container.ProjectionInt([](float a, float b) { return a + b; });
//  for (const auto &bin : integration) {
//    EXPECT_EQ(10000, bin);
//  }
  auto projection = container.Projection({"a3"});
  for (const auto &bin : projection) {
    EXPECT_EQ(600, bin);
  }
}
//
//TEST(DataContainerTest, ProjectionProfile) {
//  Qn::DataContainerSample a;
//  a.AddAxes({{"a1",2,0,2},{"b1",2,0,2}});
//  TRandom3 rndm;
//  TProfile pa("pa","pa",1,0,1);
//  TProfile pb("pb","pb",1,0,1);
//  int j = 0;
//  std::vector<float> vec;
//  std::vector<float> vec2;
//  for (int i = 0; i < 100000; ++i) {
//    vec.push_back(rndm.Gaus());
//    vec2.push_back(rndm.Gaus());
//  }
//  for (auto &bin : a) {
//    for (int i = 0; i < 100000; ++i) {
//      if (j==0 || j==2) bin.Update(vec.at(i));
//      if (j==1 || j==3) bin.Update(vec2.at(i));
//      if (j==0) pa.Fill(0.5, vec.at(i));
//      if (j==1) pb.Fill(0.5, vec2.at(i));
//    }
//    j++;
//    std::cout <<"ibin" << j << ": "<< bin.Mean() << " " << bin.Error() << std::endl;
//  }
//  std::cout << "pa" << pa.GetBinContent(1) << " " << pa.GetBinError(1) << std::endl;
//  std::cout << "pb" << pb.GetBinContent(1) << " " << pb.GetBinError(1) << std::endl;
//  auto proj = a.Projection({"a1"},[](const Qn::Sample &a, const Qn::Sample &b){return a+b;});
//  for (auto &bin : proj) {
//    std::cout << bin.Mean() << " " << bin.Error() << std::endl;
//  }
//  pa.Add(&pb);
//  std::cout << pa.GetBinContent(1) << " " << pa.GetBinError(1) << std::endl;
//}
//
//TEST(DataContainerTest, Rebin) {
//  Qn::DataContainer<float> container;
//  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container) {
//    bin = 1;
//  }
//  auto rebin = container.Rebin({"a1", 5, 0, 10}, [](float a, float b) { return a + b; });
//  int numberofbins = 0;
//  for (const auto &bin : rebin) {
//    EXPECT_EQ(2, bin);
//    numberofbins++;
//  }
//  EXPECT_EQ(50, numberofbins);
//}
//
//TEST(DataContainerTest, Select) {
//  Qn::DataContainer<float> container;
//  container.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container) {
//    bin = 1;
//  }
//  auto rebin = container.Select({"a1", 5, 0, 5});
//  int numberofbins = 0;
//  for (const auto &bin : rebin) {
//    EXPECT_EQ(1, bin);
//    numberofbins++;
//  }
//  EXPECT_EQ(50, numberofbins);
//}
//
//TEST(DataContainerTest, Addition) {
//  Qn::DataContainer<float> container_a;
//  container_a.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container_a) {
//    bin = 1;
//  }
//  Qn::DataContainer<float> container_b;
//  container_b.AddAxes({{"a1", 10, 0, 10}, {"a2", 10, 0, 10}});
//  for (auto &bin : container_b) {
//    bin = 1;
//  }
//  auto container_sum = container_a + container_b;
//  int numberofbins = 0;
//  for (const auto &bin : container_sum) {
//    EXPECT_EQ(2, bin);
//    numberofbins++;
//  }
//  EXPECT_EQ(100, numberofbins);
//}
//
//TEST(DataContainerTest, Hadd) {
//  auto file_a = new TFile("testa.root", "RECREATE");
//  file_a->cd();
//  auto container_a = new Qn::DataContainerProfile();
//  container_a->AddAxes({{"a1", 2, 0, 10}});
//  for (auto &bin : *container_a) {
//    bin.Update(3.0);
//  }
//  file_a->WriteObject(container_a, "container");
//  file_a->Write();
//  auto file_b = new TFile("testb.root", "RECREATE");
//  file_b->cd();
//  auto container_b = new Qn::DataContainerProfile();
//  container_b->AddAxes({{"a1",2, 0, 10}});
//  for (auto &bin : *container_b) {
//    bin.Update(1.0);
//  }
//  file_b->WriteObject(container_b, "container");
//  file_b->Write();
//  system("rm test.root");
//  system("hadd test.root testa.root testb.root");
//  auto file_c = new TFile("test.root", "OPEN");
//  auto container_c = (Qn::DataContainerProfile *) file_c->Get("container");
//  for (auto &bin : *container_c) {
//    EXPECT_FLOAT_EQ(2.0, bin.Mean());
//  }
//}

//TEST(DataContainerTest, ProjectionExclude) {
//  Qn::DataContainerF container_a;
//  container_a.AddAxes({{"a1", 2, 0, 10}, {"a2", 2, 0, 10}});
//  for (auto &bin : container_a) {
//    bin = 1.0;
//  }
//  auto container_b = container_a.ProjectionEX({"a1"}, [](float a, float b) { return a + b; }, {0});
//  for (auto &bin : container_b) {
//    std::cout << bin << std::endl;
//  }
//}
//
//TEST(DataContainerTest, Diagonal) {
//  Qn::DataContainerF data;
//  data.AddAxes({{"a2", 2, 0, 2}, {"a3", 2, 0, 2}});
//  for (unsigned long i = 0; i < data.GetAxis("a2").size(); i++) {
//    for (unsigned long j = 0; j < data.GetAxis("a3").size(); j++) {
//      data.At({i, i, j}) = 1;
//    }
//  }
//  std::cout << data.At(0) << std::endl;
//}
//
//TEST(DataContainerTest, DiagonalTest) {
//  Qn::DataContainerF data;
//  data.AddAxes({{"a1", 20, 0, 2}, {"a2", 20, 0, 2}, {"a3", 20, 0, 2}});
//  for (auto &bin : data) {
//    bin = 1;
//  }
//  auto test = data.GetDiagonal({{"a1", 20, 0, 2}, {"a2", 20, 0, 2}});
//}
//
//TEST(DataContainerTest, Apply) {
//  Qn::DataContainerF data1({{"a1", 5, 0, 2}});
//  Qn::DataContainerF data2({{"a1", 5, 0, 2}, {"a2", 5, 0, 2}});
//  int ibin = 0;
//  for (auto &bin : data1) {
//    bin = ++ibin;
//  }
//  for (auto &bin : data2) {
//    bin = 1;
//  }
//  auto result = data1.Apply(data2, [](float a, float b) { return a + b; });
//  ibin = 0;
//  for (const auto &bin : result) {
//    EXPECT_FLOAT_EQ(bin, 2 + (ibin++ / 5));
//  }
//}