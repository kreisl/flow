//
// Created by Lukas Kreis on 14.11.17.
//

#ifndef FLOW_QVECTOR_H
#define FLOW_QVECTOR_H
#include <iostream>
#include <array>
#include <functional>
#include <complex>
#include "QnCorrections/QnCorrectionsQnVector.h"
#include "Rtypes.h"
//#include "Base/DataContainer.h"

namespace Qn {

struct QVec {
  QVec() = default;
  QVec(float x, float y) : x(x), y(y) {}
  float x{0};
  float y{0};
  friend QVec operator+(QVec a, QVec b);
  friend QVec operator-(QVec a, QVec b);
  friend QVec operator/(QVec a, float s);
  friend QVec operator*(QVec a, float s);

  friend float norm(QVec a);
};

inline QVec operator+(QVec a, QVec b) { return {a.x + b.x, a.y + b.y}; }
inline QVec operator-(QVec a, QVec b) { return {a.x - b.x, a.y - b.y}; }
inline QVec operator/(QVec a, float s) { return {a.x/s, a.y/s}; }
inline QVec operator*(QVec a, float s) { return {a.x*s, a.y*s}; }
inline float norm(QVec a) { return sqrt(a.x*a.x + a.y*a.y); }

class QVector {

 public:

  enum class Normalization : short {
    NOCALIB,
    QOVERSQRTM,
    QOVERM,
    QOVERNORMQ
  };

  QVector() = default;
  virtual ~QVector() = default;

  QVector(Normalization norm, int n, float sum, std::array<QVec, 4> q) :
      norm_(norm),
      n_(n),
      sum_weights_(sum),
      q_(q) {}

  QVector(Normalization norm, const QnCorrectionsQnVector *vector) :
      norm_(norm) {
    if (vector) {
      n_ = vector->GetN();
      sum_weights_ = vector->GetSumOfWeights();
      for (int i = 0; i < 4; i++) {
        if (n_==0 || isnan(vector->Qx(i)) || isnan(vector->Qy(i))) {
          q_[i] = QVec(0, 0);
//          n_ = 0;
//          sum_weights_ = 0;
        } else {
          q_[i] = QVec(vector->Qx(i), vector->Qy(i));
        }
      }
    } else {
      n_ = 0;
      sum_weights_ = 0;
      for (int i = 0; i < 4; i++) {
        q_[i] = QVec(NAN, NAN);
      }
    }
  }

  float x(const int i) const { return q_[i].x; }
  float n() const { return n_; }
  float y(const int i) const { return q_[i].y; }
  float mag(const int i) const { return sqrt(q_[i].x*q_[i].x + q_[i].y*q_[i].y); }
  float sumweights() const { return sum_weights_; }
  Normalization GetNorm() const { return norm_; }

  friend QVector operator+(QVector a, QVector b);

  void Add(const QVector &a) {
    *this + a;
  }

  QVector Normal(Normalization norm) const;

  QVector DeNormal() const;

 private:
  Normalization norm_ = Normalization::NOCALIB;
  int n_ = 0;
  float sum_weights_ = 0.0;
  std::array<QVec, 4> q_;
  /// \cond CLASSIMP
 ClassDef(QVector, 6);
  /// \endcond
};

}

////

#endif //FLOW_QVECTOR_H
