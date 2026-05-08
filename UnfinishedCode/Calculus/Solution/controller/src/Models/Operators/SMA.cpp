//
// Created by miuna on 4/28/2026.
//

#include <ranges>

#include "Models/Operators/SMA.h"
#include "Models/MarketPoint.h"
#include "Models/MarketData.h"

namespace Calculus::inline Models::MarketData::Operators {

void SMA::reset(const MarketDataModel &dataSet) {

  auto n = (std::abs)(MnStride);
  bool forward = MnStride >= 0;
  auto slide = dataSet | std::views::slide(n);
  auto timestamps = slide
  | std::views::transform([forward](const auto& window) {
    if (forward) {
      return window.back().getTime();
    } else {
      return window.front().getTime();
    }
  })
  | std::ranges::to<std::vector<Time>>();
  auto values = slide | std::views::transform([n](const auto& window) -> double {
    return std::ranges::fold_left(window, double{}, [](double left, MarketPoint right) {
      return left + static_cast<double>(right.getValue()) / 100.0;
    }) / n;
  })
  | std::ranges::to<std::vector<double>>();
  beginResetModel();
  MoTimestamps = std::move(timestamps);
  MoValues = std::move(values);
  setSize(static_cast<int>(MoTimestamps.size()));
  endResetModel();
}

} // Calculus