//
// Created by miuna on 4/28/2026.
//

#ifndef CALCULUS_SIMPLEMOVINGAVERAGE_H
#define CALCULUS_SIMPLEMOVINGAVERAGE_H

#include <vector>
#include <span>

#include "Models/MarketData.h"

namespace Calculus::inline Models::Operators {

class SMA final : public MarketDataModel::Operator {
public:
  constexpr SMA(int stride) noexcept :
    Operator(1, (std::max)(0, stride - 1)),
    MnStride(stride) {
  }

  void reset(const MarketDataModel& dataSet) override;
  auto getX() const noexcept -> std::span<const MarketPoint::Time> override { return MoTimestamps; }
  auto getY(int row) const noexcept -> std::span<const double> override { return MoValues; }

private:
  int MnStride = 1;
  std::vector<MarketPoint::Time> MoTimestamps;
  std::vector<double> MoValues;
};

}

#endif //CALCULUS_SIMPLEMOVINGAVERAGE_H
