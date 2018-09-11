//
// Created by Lukas Kreis on 20.11.17.
//

#ifndef FLOW_DATACONTAINERHELPER_H
#define FLOW_DATACONTAINERHELPER_H

#include <TGraph.h>
#include <TGraphErrors.h>
#include <iostream>
#include <TMultiGraph.h>
//#include "DataContainer.h"
#include "Profile.h"

namespace Qn {

///**
// * Create a profile graph from a one dimensional DataContainerVF, which containing a distribution of floating point numbers in each bin.
// * @param data Datacontainer with one Axis to plot as a TGraphErrors.
// * @return A graph with errors corresponding to the standard error of the mean.
// */
//inline TGraphErrors *DataToProfileGraph(const Qn::DataContainerSample &data) {
//  if (data.GetAxes().size() > 1) {
//    std::cout << "Data container has more than one dimension. " << std::endl;
//    std::cout << "Cannot draw as Graph. Use Projection() to make it one dimensional." << std::endl;
//  }
//  auto graph = new TGraphErrors((int) data.size());
//  unsigned int ibin = 0;
//  for (auto &bin : data) {
//    float xhi = data.GetAxes().front().GetUpperBinEdge(ibin);
//    float xlo = data.GetAxes().front().GetLowerBinEdge(ibin);
//    float x = xlo + ((xhi - xlo) / 2);
//    graph->SetPoint(ibin, x, bin.Mean());
//    graph->SetPointError(ibin, 0, bin.Error());
//    ibin++;
//  }
//  return graph;
//};
//
//inline TGraphErrors *DataToProfileGraph(const Qn::DataContainerProfile &data) {
//  if (data.GetAxes().size() > 1) {
//    std::cout << "Data container has more than one dimension. " << std::endl;
//    std::cout << "Cannot draw as Graph. Use Projection() to make it one dimensional." << std::endl;
//  }
//  auto graph = new TGraphErrors((int) data.size());
//  unsigned int ibin = 0;
//  for (auto &bin : data) {
//    float xhi = data.GetAxes().front().GetUpperBinEdge(ibin);
//    float xlo = data.GetAxes().front().GetLowerBinEdge(ibin);
//    float x = xlo + ((xhi - xlo) / 2);
//    graph->SetPoint(ibin, x, bin.Mean());
//    graph->SetPointError(ibin, 0, bin.Error());
//    ibin++;
//  }
//  return graph;
//};

//inline TMultiGraph *DataToMultiGraph(const Qn::DataContainerSample &data, const std::string &axisname) {
//  auto multigraph = new TMultiGraph();
//  if (data.GetAxes().size() != 2) {
//    std::cout << "Data Container dimension has wrong dimension " << data.GetAxes().size() << "\n";
//    return multigraph;
//  }
//  Qn::Axis axis;
//  try { axis = data.GetAxis(axisname);}
//  catch(std::exception&) {std::cout << "axis not found" << "\n"; return multigraph;}
//  for (unsigned int ibin = 0; ibin < axis.size(); ++ibin) {
//    auto subdata = data.Select({axisname,{axis.GetLowerBinEdge(ibin),axis.GetUpperBinEdge(ibin)}});
//    auto subgraph = Qn::DataToProfileGraph(subdata);
//    subgraph->SetTitle(Form("%.2f - %.2f",axis.GetLowerBinEdge(ibin),axis.GetUpperBinEdge(ibin)));
//    subgraph->SetMarkerStyle(kFullCircle);
//    multigraph->Add(subgraph);
//  }
//  return multigraph;
//}
//
//inline TMultiGraph *DataToMultiGraph(const Qn::DataContainerProfile &data, const std::string &axisname) {
//  auto multigraph = new TMultiGraph();
//  if (data.GetAxes().size() != 2) {
//    std::cout << "Data Container dimension has wrong dimension " << data.GetAxes().size() << "\n";
//    return multigraph;
//  }
//  Qn::Axis axis;
//  try { axis = data.GetAxis(axisname);}
//  catch(std::exception&) {std::cout << "axis not found" << "\n"; return multigraph;}
//  for (unsigned int ibin = 0; ibin < axis.size(); ++ibin) {
//    auto subdata = data.Select({axisname,{axis.GetLowerBinEdge(ibin),axis.GetUpperBinEdge(ibin)}});
//    auto subgraph = Qn::DataToProfileGraph(subdata);
//    subgraph->SetTitle(Form("%.2f - %.2f",axis.GetLowerBinEdge(ibin),axis.GetUpperBinEdge(ibin)));
//    subgraph->SetMarkerStyle(kFullCircle);
//    multigraph->Add(subgraph);
//  }
//  return multigraph;
//}

}
#endif //FLOW_DATACONTAINERHELPER_H
