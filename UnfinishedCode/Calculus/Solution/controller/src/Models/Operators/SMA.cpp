//
// Created by miuna on 4/28/2026.
//

#include "Models/Operators/SMA.h"

namespace Calculus::inline Models::Operators {

void SMA::reset(const MarketDataModel &dataSet) {
  MoTimestamps.clear();
  MoValues.clear();

  auto n = (std::abs)(MnStride);
  bool forward = MnStride >= 0;
  auto slide = dataSet | std::views::slide(n);
  MoTimestamps.append_range(slide | std::views::transform([forward](const auto& window) {
    if (forward) {
      return window.back().getTime();
    } else {
      return window.front().getTime();
    }
  }));
  MoValues.append_range(slide | std::views::transform([n](const auto& window) -> double {
    return std::ranges::fold_left(window, double{}, [](double left, MarketPoint right) {
      return left + static_cast<double>(right.getValue()) / 100.0;
    }) / n;
  }));
  setSize(static_cast<int>(MoTimestamps.size()));
}

} // Calculus