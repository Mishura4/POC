//
// Created by miuna on 4/28/2026.
//

#include <ranges>

#include "Models/MarketData.h"
#include "Models/Operators/Identity.h"

namespace Calculus::inline Models::MarketData::Operators {

Identity::Identity(QObject *parent, const QString& name) :
  Operator(parent, name, 1)
{
}

void Identity::reset(const MarketDataModel &dataSet)
{
  auto timestamps = std::vector(std::from_range, dataSet | std::views::transform(&MarketPoint::getTime));
  auto values = std::vector(std::from_range, dataSet | std::views::transform([](const MarketPoint& point) {
    return static_cast<double>(point.getValue()) / 100.0;
  }));

  beginResetModel();
  MoTimestamps = std::move(timestamps);
  MoValues = std::move(values);
  setSize(static_cast<int>(std::ranges::size(MoTimestamps)));
  endResetModel();
}

}
