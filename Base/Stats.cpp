// Flow Vector Correction Framework
//
// Copyright (C) 2018  Lukas Kreis, Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <Stats.h>

#include "Stats.h"

namespace Qn {
using STAT = Qn::Stats::Status;

Stats Merge(const Stats &lhs, const Stats &rhs) {
  Stats result;
  if (((lhs.status_==STAT::REFERENCE && rhs.status_==STAT::OBSERVABLE)
      || (lhs.status_==STAT::OBSERVABLE && rhs.status_==STAT::REFERENCE)) ||
      (lhs.status_==STAT::REFERENCE && rhs.status_==STAT::REFERENCE) ||
      (lhs.status_==STAT::OBSERVABLE && rhs.status_==STAT::OBSERVABLE)) {
    result.profile_ = Profile::MergeNormal(lhs.profile_, rhs.profile_);
    result.subsamples_ = SubSamples::MergeNormal(lhs.subsamples_, rhs.subsamples_);
  }
  if (lhs.status_==STAT::POINTAVERAGE || rhs.status_==STAT::POINTAVERAGE) {
    result.profile_ = Profile::MergePointAverage(lhs.profile_, rhs.profile_);
    result.subsamples_ = SubSamples::MergePointAverage(lhs.subsamples_, rhs.subsamples_);
  }
  result.status_ = lhs.status_;
  result.bits_ = lhs.bits_;
  return result;
}

Stats operator+(const Stats &lhs, const Stats &rhs) {
  Stats result;
  if (((lhs.status_==STAT::REFERENCE && rhs.status_==STAT::OBSERVABLE)
      || (lhs.status_==STAT::OBSERVABLE && rhs.status_==STAT::REFERENCE)) ||
      (lhs.status_==STAT::REFERENCE && rhs.status_==STAT::REFERENCE) ||
      (lhs.status_==STAT::OBSERVABLE && rhs.status_==STAT::OBSERVABLE)) {
    result.profile_ = Profile::AdditionNormal(lhs.profile_, rhs.profile_);
    result.subsamples_ = SubSamples::AdditionNormal(lhs.subsamples_, rhs.subsamples_);
  }
  if (lhs.status_==STAT::POINTAVERAGE || rhs.status_==STAT::POINTAVERAGE) {
    result.profile_ = Profile::AdditionPointAverage(lhs.profile_, rhs.profile_);
    result.subsamples_ = SubSamples::AdditionPointAverage(lhs.subsamples_, rhs.subsamples_);
  }
  result.status_ = lhs.status_;
  result.bits_ = lhs.bits_;
  return result;
}

Stats operator-(const Stats &lhs, const Stats &rhs) {
  Stats result;
  if (((lhs.status_==STAT::REFERENCE && rhs.status_==STAT::OBSERVABLE)
      || (lhs.status_==STAT::OBSERVABLE && rhs.status_==STAT::REFERENCE)) ||
      (lhs.status_==STAT::REFERENCE && rhs.status_==STAT::REFERENCE) ||
      (lhs.status_==STAT::OBSERVABLE && rhs.status_==STAT::OBSERVABLE)) {
    result.profile_ = Profile::SubtractionNormal(lhs.profile_, rhs.profile_);
    result.subsamples_ = SubSamples::SubtractionNormal(lhs.subsamples_, rhs.subsamples_);
  }
  if (lhs.status_==STAT::POINTAVERAGE || rhs.status_==STAT::POINTAVERAGE) {
    result.profile_ = Profile::SubtractionPointAverage(lhs.profile_, rhs.profile_);
    result.subsamples_ = SubSamples::SubtractionPointAverage(lhs.subsamples_, rhs.subsamples_);
  }
  result.status_ = lhs.status_;
  result.bits_ = lhs.bits_;
  return result;
}

Stats operator*(const Stats &lhs, const Stats &rhs) {
  Stats result;
  if (((lhs.status_==STAT::REFERENCE && rhs.status_==STAT::OBSERVABLE)
      || (lhs.status_==STAT::OBSERVABLE && rhs.status_==STAT::REFERENCE)) ||
      (lhs.status_==STAT::REFERENCE && rhs.status_==STAT::REFERENCE) ||
      (lhs.status_==STAT::OBSERVABLE && rhs.status_==STAT::OBSERVABLE)) {
    result.profile_ = Profile::MultiplicationNormal(lhs.profile_, rhs.profile_);
    result.subsamples_ = SubSamples::MultiplicationNormal(lhs.subsamples_, rhs.subsamples_);
  }
  if (lhs.status_==STAT::POINTAVERAGE || rhs.status_==STAT::POINTAVERAGE) {
    result.profile_ = Profile::MultiplicationPointAverage(lhs.profile_, rhs.profile_);
    result.subsamples_ = SubSamples::MultiplicationPointAverage(lhs.subsamples_, rhs.subsamples_);
  }
  result.status_ = lhs.status_;
  result.bits_ = lhs.bits_;
  return result;
}

Stats operator*(const Stats &lhs, double rhs) {
  Stats result;
  if (lhs.status_==STAT::REFERENCE || lhs.status_==STAT::OBSERVABLE) {
    result.profile_ = Profile::ScaleNormal(lhs.profile_, rhs);
    result.subsamples_ = SubSamples::ScaleNormal(lhs.subsamples_, rhs);
  }
  if (lhs.status_==STAT::POINTAVERAGE) {
    result.profile_ = Profile::ScalePointAverage(lhs.profile_, rhs);
    result.subsamples_ = SubSamples::ScalePointAverage(lhs.subsamples_, rhs);
  }
  result.status_ = lhs.status_;
  result.bits_ = lhs.bits_;
  return result;
}

Stats operator*(double lhs, const Stats &rhs) {
  Stats result;
  if (rhs.status_==STAT::REFERENCE || rhs.status_==STAT::OBSERVABLE) {
    result.profile_ = Profile::ScaleNormal(rhs.profile_, lhs);
    result.subsamples_ = SubSamples::ScaleNormal(rhs.subsamples_, lhs);
  }
  if (rhs.status_==STAT::POINTAVERAGE) {
    result.profile_ = Profile::ScalePointAverage(rhs.profile_, lhs);
    result.subsamples_ = SubSamples::ScalePointAverage(rhs.subsamples_, lhs);
  }
  result.status_ = rhs.status_;
  result.bits_ = rhs.bits_;
  return result;
}

Stats operator/(const Stats &num, const Stats &den) {
  Stats result;
  if (((num.status_==STAT::REFERENCE && den.status_==STAT::OBSERVABLE)
      || (num.status_==STAT::OBSERVABLE && den.status_==STAT::REFERENCE)) ||
      (num.status_==STAT::REFERENCE && den.status_==STAT::REFERENCE) ||
      (num.status_==STAT::OBSERVABLE && den.status_==STAT::OBSERVABLE)) {
    result.profile_ = Profile::DivisionNormal(num.profile_, den.profile_);
    result.subsamples_ = SubSamples::DivisionNormal(num.subsamples_, den.subsamples_);
  }
  if (num.status_==STAT::POINTAVERAGE || den.status_==STAT::POINTAVERAGE) {
    result.profile_ = Profile::DivisionPointAverage(num.profile_, den.profile_);
    result.subsamples_ = SubSamples::DivisionPointAverage(num.subsamples_, den.subsamples_);
  }
  result.status_ = num.status_;
  result.bits_ = num.bits_;
  return result;
}

Stats Sqrt(const Stats &stats) {
  Stats result;
  if (stats.status_==STAT::REFERENCE || stats.status_==STAT::OBSERVABLE) {
    result.profile_ = Profile::SqrtNormal(stats.profile_);
    result.subsamples_ = SubSamples::SqrtNormal(stats.subsamples_);
  }
  if (stats.status_==STAT::POINTAVERAGE) {
    result.profile_ = Profile::SqrtPointAverage(stats.profile_);
    result.subsamples_ = SubSamples::SqrtPointAverage(stats.subsamples_);
  }
  return result;
}

void Stats::Print() {
  std::cout << std::endl;
  std::cout << "-----Bits------" << std::endl;
  std::cout << std::bitset<32>(bits_) << std::endl;
  std::cout << "CORRELATEDERRORS " << (bits_ & BIT(16) ? 1 : 0) << std::endl;
  std::cout << "PRODAVGWEIGHTS   " << (bits_ & BIT(17) ? 1 : 0) << std::endl;
  std::cout << "--SubSampling--" << std::endl;
  subsamples_.Print(profile_.Mean());
  std::cout << "----Profile----" << std::endl;
  profile_.Print();
}

}