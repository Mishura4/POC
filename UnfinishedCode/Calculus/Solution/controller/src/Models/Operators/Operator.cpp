//
// Created by miuna on 4/28/2026.
//

#include <chrono>

#include "Models/Operators/Operator.h"
#include "Tools.h"

namespace Calculus::inline Models::MarketData {

// auto Operator::getYBounds(int row) const noexcept ->
// std::optional<std::ranges::minmax_result<double>> {  }

Operator::Operator(QObject* parent, const QString& name, int columnCount, int startOffset, int size) :
  QAbstractTableModel(parent),
  MnColumnCount(columnCount),
  MnStartOffset(startOffset),
  MnSize(size) {
  setObjectName(name);
}

QVariant Operator::data(const QModelIndex &index, int role) const {
  auto row = index.row();
  if (row < 0 || row >= rowCount(index.parent())) {
    return QVariant();
  }

  auto column = index.column();
  if (column < 0 || column >= columnCount(index.parent())) {
    return QVariant();
  }

  if (column == 0) {
    return ToQDateTime(getX()[row]);
  }

  return getY(column - 1)[row];
}

auto Operator::getYBounds(int column, std::optional<Time> minX, std::optional<Time> maxX) const noexcept -> YBounds
{
  if (column < 0 || column >= columnCount({}) - 1) {
    return std::nullopt;
  }

  // Zip time & value column, binary search the window
  auto zipped = std::views::zip(getX(), getY(column));
  constexpr auto getTime = TupleGet<0>;
  constexpr auto getValue = TupleGet<1>;
  auto [begin, end] = GetRangeWindow(zipped, minX, maxX, std::less{}, getTime);
  auto [itMin, itMax] = std::ranges::minmax_element(begin, end, std::less<>{}, getValue);

  auto minValue = itMin == end ? std::optional<Y>{} : getValue(*itMin);
  auto maxValue = itMax == end ? std::optional<Y>{} : getValue(*itMax);
  if (minX.has_value() && begin != zipped.begin()) {
    auto before = std::ranges::prev(begin);
    auto factor = InvLerp(*minX, getTime(*before), getTime(*begin));
    auto partial = Lerp(factor, getValue(*before), getValue(*begin));
    maxValue = maxValue.has_value() ? (std::max)(partial, *maxValue) : partial;
    minValue = minValue.has_value() ? (std::min)(partial, *minValue) : partial;
  }

  if (maxX.has_value() && end != zipped.end()) {
    auto after = end;
    auto last = std::ranges::prev(after);
    auto factor = InvLerp(*maxX, getTime(*last), getTime(*after));
    auto partial = Lerp(factor, getValue(*last), getValue(*after));
    maxValue = maxValue.has_value() ? (std::max)(partial, *maxValue) : partial;
    minValue = minValue.has_value() ? (std::min)(partial, *minValue) : partial;
  }

  if (!minValue.has_value() || !maxValue.has_value()) {
    assert(zipped.empty()); // right?
    return std::nullopt;
  }
  return YBounds::value_type{ .min = *minValue, .max = *maxValue };
}

auto Operator::getYBounds(int column, QDateTime minX, QDateTime maxX) const noexcept -> QJSValueList {
  auto minUtcTime = clock_cast<Time::clock>(minX.toStdSysMilliseconds());
  auto maxUtcTime = clock_cast<Time::clock>(maxX.toStdSysMilliseconds());
  auto result = getYBounds(column, minUtcTime, maxUtcTime);
  if (result.has_value()) {
    return QJSValueList{ result->min, result->max };
  }
  return QJSValueList();
}

QVariant Operator::pointClosestTo(QDateTime time) const noexcept {
  auto utcTime = clock_cast<Time::clock>(time.toStdSysMilliseconds());
  auto values = getX();
  auto high = std::ranges::lower_bound(values, utcTime, std::less{});
  if (high == values.end()) {
    return values.empty() ? QVariant{} : QVariant::fromValue(*std::prev(high));
  }

  auto low = high;
  while (*low == *high && low != values.begin()) {
    low = std::prev(low);
  }
  auto closest = high;
  if (*high - utcTime > abs(utcTime - *low)) {
    closest = low;
  }
  return at(static_cast<int>(std::distance(values.begin(), closest)));
}

QVariant Operator::operator[](int index) const {
  return QVariant::fromValue(MarketPoint(getX()[index], getY(0)[index]));
}

QVariant Operator::at(int index) const {
  if (index < 0 || index >= rowCount({})) {
    throw std::out_of_range("Index out of range");
  }
  return (*this)[index];
}

} // namespace Calculus::inline Models::MarketData
