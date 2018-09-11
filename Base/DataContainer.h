/**
 * @file QnDataContainer.h
 * @author Lukas Kreis
 */
#ifndef QNDATACONTAINER_H
#define QNDATACONTAINER_H

#include "Axis.h"
#include "Rtypes.h"
#include "TMath.h"
#include "TObject.h"
#include "TH1F.h"
#include "TGraphErrors.h"

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <TCollection.h>
#include <iostream>
#include <TMultiGraph.h>
#include <TBrowser.h>

#include "DataVector.h"
#include "QVector.h"
#include "Sample.h"
#include "EventShape.h"

/**
 * QnCorrectionsframework
 */
namespace Qn {

inline void SetToZero(float &a) {
  a = 0;
}

/**
 * @brief      Template container class for Q-vectors and correlations
 * @param T    Type of object inside of container
 */
template<typename T>
class DataContainer : public TObject {
 public:
/**
 * Constructor
 * @param name Name of data container.
 */
  DataContainer() : integrated_(true) {
    axes_.push_back({"integrated", 1, 0, 1});
    dimension_ = 1;
    data_.resize(1);
    stride_.resize(1);
    CalculateStride();
  };

  explicit DataContainer(std::vector<Qn::Axis> axes) {
    AddAxes(axes);
  }

  virtual ~DataContainer() = default;

  using SIZETYPE = typename std::vector<T>::size_type;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  const_iterator begin() const { return data_.cbegin(); } ///< iterator for external use
  const_iterator end() const { return data_.cend(); } ///< iterator for external use
  iterator begin() { return data_.begin(); } ///< iterator for external use
  iterator end() { return data_.end(); } ///< iterator for external use

  /**
 * Size of data container
 * @return number of entries in the container
 */
  SIZETYPE size() const { return data_.size(); }

/**
 * Adds axes for storing data
 * @param axes vector of axes
 */
  void AddAxes(const std::vector<Axis> &axes) {
    if (axes.size()!=0) {
      if (integrated_) this->Reset();
      for (const auto &axis : axes) {
        AddAxis(axis);
      }
    }
  }
/**
 * Adds existing axis for storing the data with variable binning
 * @param axis Axis to be added.
 */
  void AddAxis(const Axis &axis) {
    if (integrated_) this->Reset();
    if (std::find_if(axes_.begin(), axes_.end(), [axis](const Axis &axisc) { return axisc.Name()==axis.Name(); })
        !=axes_.end())
      throw std::runtime_error("Axis already defined in vector.");
    axes_.push_back(axis);
    dimension_++;
    SIZETYPE totalbins = 1;
    for (const auto &iaxis : axes_) {
      totalbins *= iaxis.size();
    }
    data_.resize(totalbins);
    stride_.resize((SIZETYPE) dimension_ + 1);
    CalculateStride();
  }

  void InitializeEntries(const T obj) {
    for (auto &bin : data_) {
      bin = obj;
    }
  }

/**
 * Get element in the specified bin
 * @param bins Vector of bin indices of the desired element
 * @return     Element
 */
  inline T const &At(const typename std::vector<SIZETYPE> &bins) const { return data_.at(GetLinearIndex(bins)); }

/**
 * Get element in the specified bin
 * @param bins Vector of bin indices of the desired element
 * @return     Element
 */
  inline T &At(const typename std::vector<SIZETYPE> &bins) { return data_.at(GetLinearIndex(bins)); }

/**
 * Get element in the specified bin
 * @param index index of element
 * @return      Element
 */
  inline T &At(const long index) { return data_.at(index); }
/**
 * Calls function on element specified by indices.
 * @tparam Function type of function to be called on the object
 * @param indices indicies multidimensional indices of the element.
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 */
  template<typename Function>
  void CallOnElement(const typename std::vector<SIZETYPE> &indices, Function &&lambda) {
    lambda(data_[GetLinearIndex(indices)]);
  }

/**
 * Calls function on element for integrated datacontainer.
 * @tparam Function type of function to be called on the object
 * @param coordinates coordinates of element to be modified
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 */
  template<typename Function>
  void CallOnElement(Function &&lambda) {
    lambda(data_[0]);
  }

/**
 * Calls function on element specified by indices.
 * @tparam Function type of function to be called on the object
 * @param coordinates coordinates of element to be modified
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 */
  template<typename Function>
  void CallOnElement(const std::vector<float> &coordinates, Function &&lambda) {
    lambda(data_[GetLinearIndex(GetIndex(coordinates))]);
  }

/**
 * Calls function on element specified by indices.
 * @tparam Function type of function to be called on the object
 * @param index linear index of element to be modified
 * @param lambda function to be called on the element. Takes element of type T as an argument.
 */
  template<typename Function>
  inline void CallOnElement(const long index, Function &&lambda) {
    lambda(data_[index]);
  }
/**
 * Get vector of axes
 * @return Vector of axes
 */
  inline const std::vector<Axis> &GetAxes() const { return axes_; }

  /**
 * Get vector of axes
 * @return Vector of axes
 */
  inline std::vector<Axis> &GetAxes() { return axes_; }
/**
 * Get Axis with the given name.
 * Throws exception when not found.
 * @param name  Name of the desired axis
 * @return      Axis
 */
  Axis GetAxis(const std::string name) const {
    for (auto axis: axes_) {
      if (name==axis.Name()) return axis;
    }
    throw std::out_of_range("axis not found aborting");
  }

/**
 * Calculates indices in multiple dimensions from linearized index
 * @param indices Outparameter for the indices
 * @param offset Index of linearized vector
 */
  void GetIndex(typename std::vector<SIZETYPE> &indices, const unsigned long offset) const {
    unsigned long temp = offset;
    if (offset < data_.size()) {
      indices.resize(dimension_);
      for (unsigned int i = 0; i < dimension_ - 1; ++i) {
        indices[dimension_ - i - 1] = temp%axes_[dimension_ - i - 1].size();
        temp = temp/axes_[dimension_ - i - 1].size();
      }
      indices[0] = temp;
    }
  }

/**
 * Gives description by concenating axis names with coordinates
 * @param offset linear index
 * @return string with bin description
 */
  std::string GetBinDescription(const unsigned long offset) const {
    std::vector<unsigned long> indices;
    GetIndex(indices, offset);
    if (indices.empty()) return "invalid offset";
    std::string outstring;
    int i = 0;
    for (auto it = axes_.cbegin(); it!=axes_.cend(); ++it) {
      const auto &axis = *it;
      outstring += axis.Name();
      outstring += "(" + std::to_string(axis.GetLowerBinEdge(indices[i])) + ", "
          + std::to_string(axis.GetUpperBinEdge(indices[i])) + ")";
      if (it + 1!=axes_.cend()) outstring += "; ";
      ++i;
    }
    return outstring;
  }
/**
 * Projects datacontainer on a subset of axes
 * @tparam Function typename of function.
 * @param axes subset of axes used for the projection.
 * @param lambda Function used to add two entries.
 * @return projected datacontainer.
 */
  template<typename Function>
  DataContainer<T> Projection(const std::vector<std::string> names,
                              Function &&lambda) const {
    DataContainer<T> projection;
    unsigned long linearindex = 0;
    std::vector<bool> isprojected;
    auto originalaxes = this->GetAxes();
    for (const auto &originalaxis : originalaxes) {
      for (const auto &name : names) {
        isprojected.push_back((originalaxis.Name()==name)==0);
        if (originalaxis.Name()==name) projection.AddAxis(originalaxis);
      }
    }
    if (names.empty()) {
      for (const auto &bin : data_) {
        projection.At(0) = lambda(projection.At(0), bin);

      }
    } else {
      std::vector<unsigned long> indices;
      indices.reserve(dimension_);
      for (const auto &bin : data_) {
        this->GetIndex(indices, linearindex);
        for (auto index = indices.begin(); index < indices.end(); ++index) {
          if (isprojected[std::distance(indices.begin(), index)]) indices.erase(index);
        }
        projection.At(indices) = lambda(projection.At(indices), bin);
        ++linearindex;
      }
    }
    return projection;
  }

  /**
 * Projects datacontainer on a subset of axes
 * @tparam Function typename of function.
 * @param axes subset of axes used for the projection.
 * @param lambda Function used to add two entries.
 * @return projected datacontainer.
 */
  DataContainer<T> Projection(const std::vector<std::string> names) const {
    DataContainer<T> projection;
    unsigned long linearindex = 0;
    std::vector<bool> isprojected;
    auto originalaxes = this->GetAxes();
    auto lambda = [](const T &a, const T &b) { return a + b; };
    for (const auto &originalaxis : originalaxes) {
      for (const auto &name : names) {
        isprojected.push_back((originalaxis.Name()==name)==0);
        if (originalaxis.Name()==name) projection.AddAxis(originalaxis);
      }
    }
    if (names.empty()) {
      for (const auto &bin : data_) {
        projection.At(0) = lambda(projection.At(0), bin);

      }
    } else if(!names.empty() && isprojected.empty()) {
      std::string errormsg = "Axes not found cannot project.";
      throw std::logic_error(errormsg);
    } else {
      std::vector<unsigned long> indices;
      indices.reserve(dimension_);
      for (const auto &bin : data_) {
        this->GetIndex(indices, linearindex);
        for (int i = indices.size() - 1; i >= 0; --i) {
          if (isprojected[i]) {
            indices.erase(indices.begin() + i);
          }
        }
        projection.At(indices) = lambda(projection.At(indices), bin);
        ++linearindex;
      }
    }
    return projection;
  }

  /**
 * Projects datacontainer on a subset of axes
 * @tparam Function typename of function.
 * @param axes subset of axes used for the projection.
 * @param lambda Function used to add two entries.
 * @return projected datacontainer.
 */
  template<typename Function>
  DataContainer<T> ProjectionEX(const std::vector<std::string> names,
                                Function &&lambda,
                                std::vector<int> exindices) const {
    DataContainer<T> projection;
    unsigned long linearindex = 0;
    std::vector<bool> isprojected;
    auto originalaxes = this->GetAxes();
    for (const auto &originalaxis : originalaxes) {
      for (const auto &name : names) {
        isprojected.push_back((originalaxis.Name()==name)==0);
        if (originalaxis.Name()==name) projection.AddAxis(originalaxis);
      }
    }
    if (names.empty()) {
      for (const auto &bin : data_) {
        projection.At(0) = lambda(projection.At(0), bin);
      }
    } else {
      std::vector<unsigned long> indices;
      indices.reserve(dimension_);
      for (const auto &bin : data_) {
        if (std::find(exindices.begin(), exindices.end(), linearindex)!=exindices.end()) {
          ++linearindex;
          continue;
        }
        this->GetIndex(indices, linearindex);
        for (auto index = indices.begin(); index < indices.end(); ++index) {
          if (isprojected[std::distance(indices.begin(), index)]) indices.erase(index);
        }
        projection.At(indices) = lambda(projection.At(indices), bin);
        ++linearindex;
      }
    }
    return projection;
  }

/**
 * "Integrates" container
 * @tparam Function typename of function.
 * @param axes subset of axes used for the projection.
 * @param lambda Function used to add two entries.
 * @return integrated container
 */
  template<typename Function>
  DataContainer<T> ProjectionInt(Function &&lambda) const {
    DataContainer<T> projection;
    for (const auto &bin : data_) {
      projection.data_[0] = lambda(projection.data_[0], bin);
    }
    return projection;
  }
/**
 * Map function to datacontainer. Does not modify the original container.
 * @tparam Function function
 * @param lambda unary function to be applied to each element.
 * @return datacontainer after applying function.
 */
  template<typename Function>
  DataContainer<T> Map(Function &&lambda) {
    DataContainer<T> result(*this);
    std::transform(data_.begin(), data_.end(), result.begin(), [&lambda](T &element) { return lambda(element); });
    return result;
  }

/**
 * Selects subrange of axis of datacontainer. If resulting axis is one dimensional it is deleted and dimension of
 * the resulting container is reduced by one.
 * @tparam Function type of function
 * @param data input data container
 * @param axis subrange of axis to perform selection
 * @return
 */
  DataContainer<T> Select(const Qn::Axis &axis) const {
    DataContainer<T> selected;
    long axisposition = 0;
    long tmpaxisposition = 0;
    for (const auto &a : axes_) {
      if (a.Name()==axis.Name()) {
        selected.AddAxis(axis);
        axisposition = tmpaxisposition;
      } else {
        selected.AddAxis(a);
      }
      tmpaxisposition++;
    }
    unsigned long index = 0;
    std::vector<unsigned long> indices;
    indices.reserve(dimension_);
    for (const auto &bin : data_) {
      GetIndex(indices, index);
      auto binlow = axes_[axisposition].GetLowerBinEdge(indices[axisposition]);
      auto binhigh = axes_[axisposition].GetUpperBinEdge(indices[axisposition]);
      auto binmid = binlow + (binhigh - binlow)/2;
      auto rebinnedindex = axis.FindBin(binmid);
      if (rebinnedindex!=-1) {
        indices[axisposition] = static_cast<unsigned long>(rebinnedindex);
        selected.CallOnElement(indices, [&bin](T &element) { element = bin; });
      }
      ++index;
    }
    if (axis.size()==1) {
      selected.axes_.erase(selected.axes_.begin() + axisposition);
      selected.stride_.resize(selected.axes_.size() + 1);
      selected.dimension_ = selected.axes_.size();
      selected.CalculateStride();
    }
    return selected;
  }

/**
 * Apply function to two datacontainers.
 * The axes need to have the same order.
 * Elements in datacontainer with smaller dimensions are used as "integrated bins".
 * @tparam Function type of function
 * @param data Datacontainer
 * @param lambda function to be applied on both elements
 * @return resulting datacontainer.
 */
  template<typename Function>
  DataContainer<T> Apply(const DataContainer<T> &data, Function &&lambda) const {
    DataContainer<T> result;
    std::vector<SIZETYPE> indices;
    unsigned long index = 0;
    if (axes_.size() > data.axes_.size()) {
      for (unsigned long iaxis = 0; iaxis < data.axes_.size() - 1; ++iaxis) {
        if (axes_[iaxis].Name()!=data.axes_[iaxis].Name()) {
          std::string errormsg = "Axes do not match.";
          throw std::logic_error(errormsg);
        }
      }
      result.AddAxes(axes_);
      indices.reserve(dimension_);
      for (const auto &bin_a : data_) {
        GetIndex(indices, index);
        result.data_[index] = lambda(bin_a, data.At(indices));
        ++index;
      }
    } else {
      for (unsigned long iaxis = axes_.size() - 1; iaxis > 0; --iaxis) {
        if (axes_[iaxis].Name()!=data.axes_[iaxis].Name()) {
          std::string errormsg = "Axes do not match.";
          throw std::logic_error(errormsg);
        }
      }
      result.AddAxes(data.axes_);
      indices.reserve(data.dimension_);
      for (const auto &bin_b : data.data_) {
        data.GetIndex(indices, index);
        result.data_[index] = lambda(At(indices), bin_b);
        ++index;
      }
    }
    return result;
  }

/**
 * Rebins the Datacontainer using the supplied function to calculate the new bin entries of the specified axis.
 * @tparam Function
 * @param rebinaxis axis to be rebinned.
 * @param lambda function used to calculate new bin entries.
 * @return rebinned datacontainer.
 */
  template<typename Function>
  DataContainer<T> Rebin(const Axis &rebinaxis, Function &&lambda) const {
    DataContainer<T> rebinned;
    unsigned long axisposition = 0;
    bool axisfound = false;
    /*
     * Check if axis to be rebinned is found in the datacontainer.
     */
    for (auto axis = std::begin(axes_); axis < std::end(axes_); ++axis) {
      if (axis->Name()==rebinaxis.Name()) {
        rebinned.AddAxis(rebinaxis);
        axisposition = static_cast<unsigned long>(std::distance(axes_.begin(), axis));
        axisfound = true;
      } else {
        rebinned.AddAxis(*axis);
      }
    }
    if (!axisfound) {
      std::string errormsg = "Datacontainer does not have axis of name " + rebinaxis.Name();
      throw std::logic_error(errormsg);
    }
    /*
     * Check if there is no overlap in the bin edges.
     */
    bool rebin_ok = true;
    for (const auto &rebinedge : rebinaxis) {
      bool found = false;
      for (const auto &binedge : (Axis) axes_.at(axisposition)) {
        float test = TMath::Abs(rebinedge - binedge);
        if (test < 10e-4) {
          found = rebin_ok;
          break;
        }
      }
      rebin_ok = rebin_ok && found;
    }
    if (!rebin_ok) {
      std::string errormsg = "Rebinned axis has overlapping bins." + rebinaxis.Name();
      throw std::logic_error(errormsg);
    }
    unsigned long ibin = 0;
    std::vector<SIZETYPE> indices;
    indices.reserve(dimension_);
    for (const auto &bin : data_) {
      GetIndex(indices, ibin);
      auto binlow = axes_[axisposition].GetLowerBinEdge(indices[axisposition]);
      auto binhigh = axes_[axisposition].GetUpperBinEdge(indices[axisposition]);
      auto binmid = binlow + (binhigh - binlow)/2;
      auto rebinnedindex = rebinaxis.FindBin(binmid);
      indices[axisposition] = static_cast<unsigned long>(rebinnedindex);
      if (rebinnedindex!=-1) rebinned.At(indices) = lambda(rebinned.At(indices), bin);
      ++ibin;
    }
    return rebinned;
  }

  /*
   * Clears data to be filled. To be called after one event.
   */
  void ClearData() {
    auto size = data_.size();
//    data_.clear();
//    data_.resize(size);
    data_.assign(size, T());
  }
/**
 * Calculates one dimensional index from a vector of indices.
 * @param index vector of indices in multiple dimensions
 * @return      index in one dimension
 */
  SIZETYPE GetLinearIndex(const std::vector<SIZETYPE> &index) const {
    SIZETYPE offset = (index[dimension_ - 1]);
    for (unsigned int i = 0; i < dimension_ - 1; ++i) {
      offset += stride_[i + 1]*(index[i]);
    }
    return offset;
  }
/**
 * Checks if datacontainer is integrated.
 * It is integrated if first axis is the integrated axis with Id == -1;
 * @return true if integrated, else false.
 */
  inline bool IsIntegrated() const { return integrated_; }
/**
 * Merges DataContainer with DataContainers in TCollection.
 * Function used in "hadd"
 * A function with signature T Merge( T, T) needs to be implemented for merging to work.
 * @param inputlist List of datacontainers
 * @return size of datacontainer. dummyvalue
 */
  Long64_t Merge(TCollection *inputlist) {
    TIter next(inputlist);
    while (auto data = (DataContainer<T> *) next()) {
      auto lambda = [](const T &a, const T &b) { return Qn::Merge(a, b); };
      *this = this->Apply(*data, lambda);
    }
    return this->size();
  }

 private:
  bool integrated_ = true;
  unsigned long dimension_ = 0; ///< dimensionality of data
  std::vector<T> data_; ///< linearized vector of data
  std::vector<Axis> axes_; ///< Vector of axes
  std::vector<long> stride_; ///< Offset for conversion into one dimensional vector.

  void Reset() {
    integrated_ = false;
    dimension_ = 0;
    data_.clear();
    axes_.clear();
    stride_.clear();
  }

/**
 * Calculates multidimensional index from coordinates
 * @param coordinates floating point coordinates
 * @return index belonging to coordinates
 */
  typename std::vector<SIZETYPE> GetIndex(const std::vector<float> &coordinates) const {
    typename std::vector<SIZETYPE> indices;
    unsigned long axisindex = 0;
    for (const auto &axis : axes_) {
      auto bin = axis.FindBin(coordinates[axisindex]);
      if (bin < 0 || bin > axis.size())
        throw std::out_of_range("bin out of specified range");
      indices.push_back(bin);
      axisindex++;
    }
    return indices;
  }

/**
 * Calculates offset for transformation into one dimensional vector.
 */
  void CalculateStride() {
    stride_[dimension_] = 1;
    for (unsigned int i = 0; i < dimension_; ++i) {
      stride_[dimension_ - i - 1] = stride_[dimension_ - i]*axes_[dimension_ - i - 1].size();
    }
  }

 public:

  void NDraw(Option_t *option, std::string axis_name ="") {}
  void Browse(TBrowser *b) {}

/**
 * Calculates linear index from coordinates
 * returns -1 if outside of range.
 * @param coordinates floating point coordinates
 * @return linear index
 */
  long GetLinearIndex(const std::vector<float> &coordinates) const {
    typename std::vector<SIZETYPE> indices;
    unsigned long axisindex = 0;
    for (const auto &axis : axes_) {
      auto bin = axis.FindBin(coordinates[axisindex]);
      if (bin >= (unsigned int) axis.size() || bin < 0) {
        return -1;
      } else {
        indices.push_back(bin);
        axisindex++;
      }
    }
    return GetLinearIndex(indices);
  }

  std::vector<int> GetDiagonal(const std::vector<Axis> &axes) {
    std::vector<int> diagonal;
    int stride_sum = 0;
    int nbins_diagonal = 1;
    int iaxis = 0;
    for (const auto &ref_axis : axes_) {
      bool found = false;
      for (const auto &axis : axes) {
        if (ref_axis==axis) {
          stride_sum += stride_[iaxis];
          found = true;
        }
      }
      if (!found) nbins_diagonal *= ref_axis.size();
      ++iaxis;
    }
    for (unsigned int ibin = 0; ibin < axes[0].size(); ++ibin) {
      for (int idiag = 0; idiag < nbins_diagonal; ++idiag) {
        diagonal.push_back(ibin*stride_sum + idiag);
      }
    }
    return diagonal;
  }

/// \cond CLASSIMP
 ClassDef(DataContainer, 11);
/// \endcond
};

template<>
inline void DataContainer<Qn::Sample>::Browse(TBrowser *b) {
  for (auto &axis : axes_) {
    auto proj = this->Projection({axis.Name()});
    auto graph = new TGraphErrors((int) proj.size());
    unsigned int ibin = 0;
    for (auto &bin : proj) {
      float xhi = proj.axes_.front().GetUpperBinEdge(ibin);
      float xlo = proj.axes_.front().GetLowerBinEdge(ibin);
      float x = xlo + ((xhi - xlo)/2);
      graph->SetPoint(ibin, x, bin.Mean());
      graph->SetPointError(ibin, 0, bin.Error());
      ibin++;
    }
    graph->SetName(axis.Name().data());
    graph->SetTitle(axis.Name().data());
    b->Add(graph);
//    graph->SetTitle(this->GetName());
//    browsables.AddAtAndExpand(graph,i);
//    browsables->SetOwner(true);
  }
}

template<>
inline void DataContainer<Qn::Sample>::NDraw(Option_t *option, std::string axis_name) {
  if (axes_.size()==1) {
    auto graph = new TGraphErrors((int) data_.size());
    unsigned int ibin = 0;
    for (auto &bin : data_) {
      float xhi = axes_.front().GetUpperBinEdge(ibin);
      float xlo = axes_.front().GetLowerBinEdge(ibin);
      float x = xlo + ((xhi - xlo)/2);
      graph->SetPoint(ibin, x, bin.Mean());
      graph->SetPointError(ibin, 0, bin.Error());
      ibin++;
    }
    graph->SetTitle(this->GetName());
    graph->Draw(option);
  } else if (axes_.size()==2) {
    auto multigraph = new TMultiGraph();
    Qn::Axis axis = this->GetAxis(axis_name);
    for (unsigned int ibin = 0; ibin < axis.size(); ++ibin) {
      auto subdata = this->Select({axis.Name(), {axis.GetLowerBinEdge(ibin), axis.GetUpperBinEdge(ibin)}});
      auto subgraph = new TGraphErrors((int) subdata.size());
      unsigned int jbin = 0;
      for (auto &bin : subdata) {
        float xhi = subdata.axes_.front().GetUpperBinEdge(jbin);
        float xlo = subdata.axes_.front().GetLowerBinEdge(jbin);
        float x = xlo + ((xhi - xlo)/2);
        subgraph->SetPoint(jbin, x, bin.Mean());
        subgraph->SetPointError(jbin, 0, bin.Error());
        jbin++;
      }
      subgraph->SetTitle(Form("%.2f - %.2f", axis.GetLowerBinEdge(ibin), axis.GetUpperBinEdge(ibin)));
      subgraph->SetMarkerStyle(kFullCircle);
      multigraph->Add(subgraph);
    }
    multigraph->Draw(option);
  }
}

template<typename T>
DataContainer<T> operator+(DataContainer<T> a, const DataContainer<T> &b) {
  return a.Apply(b, [](const T &a, const T &b) { return a + b; });
}

template<typename T>
DataContainer<T> operator-(DataContainer<T> a, const DataContainer<T> &b) {
  return a.Apply(b, [](const T &a, const T &b) { return a - b; });
}

template<typename T>
DataContainer<T> operator*(DataContainer<T> a, const DataContainer<T> &b) {
  return a.Apply(b, [](const T &a, const T &b) { return a*b; });
}

template<typename T>
DataContainer<T> operator/(DataContainer<T> a, const DataContainer<T> &b) {
  return a.Apply(b, [](const T &a, const T &b) { return a/b; });
}

template<typename T>
DataContainer<T> operator*(DataContainer<T> a, double b) {
  return a.Map([b](T &a) { return a*b; });
}

template<typename T>
DataContainer<T> Sqrt(DataContainer<T> a) {
  return a.Map([](T &x) { return x.Sqrt(); });
}

using DataContainerF = DataContainer<float>;
using DataContainerFB = DataContainer<std::pair<bool, float>>;
using DataContainerQVector = DataContainer<Qn::QVector>;
using DataContainerProfile = DataContainer<Qn::Profile>;
using DataContainerSample = DataContainer<Qn::Sample>;
using DataContainerDataVector = DataContainer<std::vector<DataVector>>;
using DataContainerTH1F = DataContainer<TH1F>;
using DataContainerESE = DataContainer<Qn::EventShape>;

}
#endif
