//
// Created by miuna on 4/28/2026.
//

#ifndef CALCULUS_SIMPLEMOVINGAVERAGE_H
#define CALCULUS_SIMPLEMOVINGAVERAGE_H

#include <vector>
#include <span>

#include "Operator.h"

namespace Calculus::inline Models::MarketData::Operators {

class SMA final : public Operator {
public:
  SMA(int stride, QObject *parent, const QString name);
  explicit SMA(int stride, QObject* parent = nullptr) :
    SMA(stride, parent, tr("SMA %1").arg(stride))
  {
  }

  void reset(const MarketDataModel& dataSet) override;
  auto getX() const noexcept -> std::span<const Time> override { return MoTimestamps; }
  auto getY(int row) const noexcept -> std::span<const double> override { return MoValues; }

private:
  int MnStride = 1;
  std::vector<Time> MoTimestamps;
  std::vector<double> MoValues;
};

}

#endif //CALCULUS_SIMPLEMOVINGAVERAGE_H
