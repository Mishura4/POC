//
// Created by miuna on 4/28/2026.
//

#ifndef CALCULUS_OPERATORS_IDENTITY_H
#define CALCULUS_OPERATORS_IDENTITY_H

#include <vector>
#include <span>

#include "Operators/Operator.h"
#include "MarketData.h"

namespace Calculus::inline Models::MarketData::Operators {

class Identity : public Operator {
public:
  Identity(QObject* parent = nullptr, const QString& name = tr("Identity"));

  void reset(const MarketDataModel& dataSet) override;
  auto getX() const noexcept -> std::span<const Time> override { return MoTimestamps; }
  auto getY(int row) const noexcept -> std::span<const double> override { return MoValues; }

private:
  // TODO: We don't necessarily need to copy here maybe
  std::vector<Time> MoTimestamps;
  std::vector<double> MoValues;
};

}

#endif // CALCULUS_OPERATORS_IDENTITY_H
