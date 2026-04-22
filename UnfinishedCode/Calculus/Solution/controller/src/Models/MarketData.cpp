#include "Models/MarketData.h"

#include <iostream>
#include <algorithm>

namespace Calculus {

namespace {

using dseconds = std::chrono::duration<double>;
using dstime = std::chrono::time_point<MarketPoint::Time::clock, dseconds>;

QJSValue toJSValue(MarketPoint::Value value) {
  return { static_cast<double>(value) / 100.0 };
}

template <typename R = void>
auto lerp(std::floating_point auto factor, auto low, decltype(low) high) {
  if constexpr (std::is_void_v<R>) {
    return factor * (high - low) + low;
  } else {
    return static_cast<R>(factor * (high - low) + low);
  }
}

template <typename R = double>
auto invlerp(auto value, decltype(value) low, decltype(value) high) {
  if constexpr (std::is_void_v<R>) {
    return (value - low) / (high - low);
  } else {
    return static_cast<R>(value - low) / static_cast<R>(high - low);
  }
}

}

template <typename NewPoints>
void MarketDataModel::_updateMinMax(const NewPoints &range) {
  if (std::ranges::empty(range))
    return;

  auto prevMinX = _minXindex;
  auto prevMaxX = _maxXindex;
  auto prevMinY = _minYindex;
  auto prevMaxY = _maxYindex;

  auto [minX, maxX] = std::ranges::minmax_element(range, std::less<>{}, tuple_get<0>);
  _minXindex = static_cast<int>(std::ranges::distance(_points.begin(), minX));
  _maxXindex = static_cast<int>(std::ranges::distance(_points.begin(), maxX));

  auto [minY, maxY] = std::ranges::minmax_element(range, std::less<>{}, tuple_get<1>);
  _minYindex = static_cast<int>(std::ranges::distance(_points.begin(), minY));
  _maxXindex = static_cast<int>(std::ranges::distance(_points.begin(), maxY));
}

MarketPoint::MarketPoint(QDateTime time, Value value) noexcept :
  MoTime(clock_cast<std::chrono::utc_clock>(time.toStdSysMilliseconds())),
  MnValue(value)
{
}

auto MarketPoint::time() const -> QDateTime {
  return QDateTime::fromStdTimePoint(clock_cast<std::chrono::system_clock>(getTime()));
}

PartialMarketPoint::PartialMarketPoint(MarketPoint before, MarketPoint after, Time time) noexcept :
  MarketPoint(time, lerp(
    invlerp<dseconds>(time, before.getTime(), after.getTime()),
    before.getValue(), after.getValue())
  ),
  MoBefore(before), MoAfter(after) {

}

MarketDataModel::MarketDataModel(QObject *parent) noexcept
  : QAbstractTableModel(parent) {}

QHash<int, QByteArray> MarketDataModel::roleNames() const {
  return QHash<int, QByteArray>{
    {static_cast<int>(Role::Time), "Time"},
    {static_cast<int>(Role::Value), "Value"}
  };
}

QVariant MarketDataModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return {};

  if (index.row() < 0 || index.row() >= _points.size())
    return {};

  switch (index.column()) {
    case 0:
      return _points[index.row()].time();
    case 1:
      return _points[index.row()].value();
    default:
      return {};
  }
}

auto MarketDataModel::begin() noexcept -> iterator {
  return _points.begin();
}

auto MarketDataModel::begin() const noexcept -> const_iterator {
  return _points.begin();
}

auto MarketDataModel::end() noexcept -> iterator {
  return _points.end();
}

auto MarketDataModel::end() const noexcept -> const_iterator {
  return _points.end();
}

auto MarketDataModel::size() const noexcept -> int {
  return rowCount({});
}

auto MarketDataModel::minX() const noexcept -> QVariant {
  return _minXindex < 0 ? QVariant() : _points[_minXindex].time();
}

auto MarketDataModel::maxX() const noexcept -> QVariant {
  return _maxXindex < 0 ? QVariant() : _points[_maxXindex].time();
}

auto MarketDataModel::minY() const noexcept -> QVariant {
  return _minYindex < 0 ? QVariant() : _points[_minYindex].value();
}

auto MarketDataModel::maxY() const noexcept -> QVariant {
  return _maxYindex < 0 ? QVariant() : _points[_maxYindex].value();
}

auto MarketDataModel::getSubRange(QDateTime minTime, QDateTime maxTime) const noexcept -> Subrange {
  using clock = MarketPoint::Time::clock;
  auto minUtcTime = clock_cast<clock>(minTime.toStdSysMilliseconds());
  auto maxUtcTime = clock_cast<clock>(maxTime.toStdSysMilliseconds());
  auto begin = std::ranges::lower_bound(
    _points.begin(), _points.end(),
    minUtcTime, std::less{}, &MarketPoint::getTime
  );
  auto end = std::ranges::upper_bound(
    begin, _points.end(),
    maxUtcTime, std::less{}, &MarketPoint::getTime
  );
  return Subrange{ begin, end };
}

QJSValue MarketDataModel::getMinY(QDateTime minTime, QDateTime maxTime) const {
  auto subrange = getSubRange(minTime, maxTime);
  if (std::ranges::empty(subrange)) {
    return QJSValue{};
  } else {
    auto min = std::ranges::min(subrange | std::views::transform(&MarketPoint::getValue));
    return toJSValue(min);
  }
}

QJSValue MarketDataModel::getMaxY(QDateTime minTime, QDateTime maxTime) const {
  auto subrange = getSubRange(minTime, maxTime);
  if (std::ranges::empty(subrange)) {
    return QJSValue{};
  } else {
    auto max = std::ranges::max(subrange | std::views::transform(&MarketPoint::getValue));
    return toJSValue(max);
  }
}

auto MarketDataModel::getBefore(MarketPoint::Time time) const noexcept -> Points::const_iterator {
  return std::ranges::lower_bound(
    _points.begin(), _points.end(),
    time, std::less<>{}, &MarketPoint::getTime
  );
}

auto MarketDataModel::getPartialPoint(MarketPoint::Time time) const -> std::optional<PartialMarketPoint> {
  auto before = getBefore(time);
  if (before == std::ranges::end(_points))
    return std::nullopt;

  auto it = before;
  while (it->getTime() <= time) {
    ++it;

    if (it == _points.end())
      return std::nullopt;
  }
  return std::optional<PartialMarketPoint>{ std::in_place, *before, *it, time };
}

QJSValueList MarketDataModel::getBoundsY(QDateTime minTime, QDateTime maxTime) const {
  using clock = MarketPoint::Time::clock;
  auto minUtcTime = clock_cast<clock>(minTime.toStdSysMilliseconds());
  auto maxUtcTime = clock_cast<clock>(maxTime.toStdSysMilliseconds());
  auto subrange = getSubRange(minTime, maxTime);

  auto [itMin, itMax] = std::ranges::minmax_element(subrange, std::less<>{}, &MarketPoint::getTime);
  if (itMin == _points.end())
    return {};

  auto min = *itMin;
  auto max = *itMax;
  if (subrange.begin() != _points.begin()) {
    auto before = std::ranges::prev(subrange.begin());
    if (before->getValue() < min.getValue()) {
      auto partial = PartialMarketPoint(*before, *subrange.begin(), minUtcTime);
      min = min.getValue() < partial.getValue() ? min : partial;
    }
  }

  if (itMax == subrange.end()) // Special case when we are looking only at partial points
    max = min;

  if (subrange.end() != _points.end()) {
    auto after = subrange.end();
    auto last = std::ranges::prev(after);
    if (after->getValue() > max.getValue()) {
      auto partial = PartialMarketPoint(*last, *after, maxUtcTime);
      max = max.getValue() > partial.getValue() ? max : partial;
    }
  }

  double dMin = static_cast<double>(min.getValue() / 100.0);
  double dMax = static_cast<double>(max.getValue() / 100.0);
  return QJSValueList{ dMin, dMax };
}

void MarketDataModel::_recalcMinMax() noexcept {
  using namespace std::chrono_literals;
  if (std::ranges::empty(_points)) {
    _maxXindex = -1;
    _minXindex = -1;
    _minYindex = -1;
    _maxYindex = -1;
  } else {
    auto [minX, maxX] = std::ranges::minmax_element(_points, std::less<>{}, tuple_get<0>);
    _minXindex = static_cast<int>(std::ranges::distance(_points.begin(), minX));
    _maxXindex = static_cast<int>(std::ranges::distance(_points.begin(), maxX));

    auto [minY, maxY] = std::ranges::minmax_element(_points, std::less<>{}, tuple_get<1>);
    _minYindex = static_cast<int>(std::ranges::distance(_points.begin(), minY));
    _maxYindex = static_cast<int>(std::ranges::distance(_points.begin(), maxY));
  }
}

void MarketDataModel::addPoint(MarketPoint point) {
  auto it = std::ranges::lower_bound(_points, point, PointSorter{});
  int row = static_cast<int>(std::ranges::distance(_points.begin(), it));
  _points.reserve(_points.size() + 1); // Reserve so that adding a point doesn't throw
  beginInsertRows(QModelIndex(), row, row + 1);
  _points.insert(it, point); // Doesn't throw -- we reserved above
  endInsertRows();
}

void MarketDataModel::setPoints(Points points) {
  auto prevMinX = minX();
  auto prevMaxX = maxX();
  auto prevMinY = minY();
  auto prevMaxY = maxY();
  std::ranges::sort(points, PointSorter{});
  beginResetModel();
  _points = std::move(points);
  endResetModel();

  _recalcMinMax();
  if (prevMinX != minX() || prevMaxX != maxX() || prevMinY != minY() || prevMaxY != maxY()) {
    emit boundsChanged();
  }
}

} // namespace Calculus
