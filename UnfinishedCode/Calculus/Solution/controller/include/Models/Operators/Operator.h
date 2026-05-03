//
// Created by miuna on 4/28/2026.
//

#ifndef CALCULUS_OPERATOR_H
#define CALCULUS_OPERATOR_H

#include <vector>
#include <span>

#include "MarketPoint.h"

namespace Calculus::inline Models::MarketData {

class MarketDataModel;

class Operator {
public:
  Operator(const Operator&) noexcept = default;
  Operator(Operator&&) noexcept = default;

  Operator& operator=(const Operator&) noexcept = default;
  Operator& operator=(Operator&&) noexcept = default;
  virtual ~Operator() = default;

  virtual void reset(const MarketDataModel& dataSet) = 0;
  virtual auto getX() const noexcept -> std::span<const Time> = 0;
  virtual auto getY(int row) const noexcept -> std::span<const double> = 0;

  constexpr auto rowCount() const noexcept -> int { return MnRowCount; }
  constexpr auto startOffset() const noexcept -> int { return MnStartOffset; }
  constexpr auto size() const noexcept -> int { return MnSize; }

protected:
  constexpr Operator(int rowCount, int startOffset = 0, int size = 0) noexcept :
    MnRowCount(rowCount),
    MnStartOffset(startOffset),
    MnSize(size)
  {}

  constexpr void setSize(int newSize) noexcept { MnSize = newSize; }

private:
  int MnRowCount;
  int MnStartOffset = 0;
  int MnSize = 0;
};

}

#endif //CALCULUS_OPERATOR_H
