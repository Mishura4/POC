#include "Models/MarketData.h"

#include <iostream>
#include <algorithm>

namespace Calculus::inline Models {

namespace {

using dseconds = std::chrono::duration<double>;
using dstime = std::chrono::time_point<MarketPoint::Time::clock, dseconds>;

}

MarketDataModel::MarketDataModel(QObject *parent) noexcept
  : QAbstractTableModel(parent) {}

QHash<int, QByteArray> MarketDataModel::roleNames() const {
  return QHash<int, QByteArray>{
    {static_cast<int>(Role::Time), "Time"},
    {static_cast<int>(Role::Value), "Value"},
    {static_cast<int>(Role::SMA), "SMA"}
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
    case 2:
      return index.row() >= 3 ? _points[index.row()].time() : QVariant();
    case 3:
      return toQVariant(getSimpleMovingAverage(_points.begin() + index.row(), 3).transform([](double n) { return n / 100; }));
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
  return toQVariant(_bounds.transform([](const Bounds& bounds) {
    return toQDateTime(bounds.minX);
  }));
}

auto MarketDataModel::maxX() const noexcept -> QVariant {
  return toQVariant(_bounds.transform([](const Bounds& bounds) {
    return toQDateTime(bounds.maxX);
  }));
}

auto MarketDataModel::minY() const noexcept -> QVariant {
  return toQVariant(_bounds.transform([](const Bounds& bounds) {
    return static_cast<double>(bounds.minY) / 100.0;
  }));
}

auto MarketDataModel::maxY() const noexcept -> QVariant {
  return toQVariant(_bounds.transform([](const Bounds& bounds) {
    return static_cast<double>(bounds.maxY) / 100.0;
  }));
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
    return static_cast<double>(min / 100);
  }
}

QJSValue MarketDataModel::getMaxY(QDateTime minTime, QDateTime maxTime) const {
  auto subrange = getSubRange(minTime, maxTime);
  if (std::ranges::empty(subrange)) {
    return QJSValue{};
  } else {
    auto max = std::ranges::max(subrange | std::views::transform(&MarketPoint::getValue));
    return static_cast<double>(max / 100);
  }
}

auto MarketDataModel::getBefore(MarketPoint::Time time) const noexcept -> DataSet::const_iterator {
  return std::ranges::lower_bound(
    _points.begin(), _points.end(),
    time, std::less<>{}, &MarketPoint::getTime
  );
}

auto MarketDataModel::getSimpleMovingAverage(DataSet::const_iterator where, ptrdiff_t points) const noexcept -> std::optional<double> {
  if (points <= 0 || std::distance(_points.begin(), where) < points) {
    return std::nullopt;
  } else {
    auto range = std::ranges::subrange(where - points + 1, where + 1);
    return std::ranges::fold_left(range | std::views::transform(&MarketPoint::getValue), double{}, [](double a, MarketPoint::Value b) -> double {
      return a + static_cast<double>(b);
    }) / std::abs(points);
  }
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

  auto minValue = static_cast<double>(itMin->getValue());
  auto maxValue = static_cast<double>(itMax->getValue());
  if (subrange.begin() != _points.begin()) {
    auto before = std::ranges::prev(subrange.begin());
    if (before->getValue() < minValue) {
      auto partial = MarketPoint::GetPartialValue(*before, *subrange.begin(), minUtcTime);
      minValue = minValue < partial ? minValue : partial;
    }
  }

  if (itMax == subrange.end()) // Special case when we are looking only at partial points
    maxValue = minValue;

  if (subrange.end() != _points.end()) {
    auto after = subrange.end();
    auto last = std::ranges::prev(after);
    if (after->getValue() > maxValue) {
      auto partial = MarketPoint::GetPartialValue(*last, *after, maxUtcTime);
      maxValue = maxValue > partial ? maxValue : partial;
    }
  }

  auto dMin = minValue / 100.0;
  auto dMax = maxValue / 100.0;
  return QJSValueList{ dMin, dMax };
}

void MarketDataModel::_recalcMinMax() noexcept {
  using namespace std::chrono_literals;
  if (std::ranges::empty(_points)) {
    _bounds = std::nullopt;
  } else {
    auto [minX, maxX] = std::ranges::minmax_element(_points, std::less<>{}, tuple_get<0>);
    auto [minY, maxY] = std::ranges::minmax_element(_points, std::less<>{}, tuple_get<1>);
    _bounds = Bounds {
      .minX = minX->getTime(),
      .maxX = maxX->getTime(),
      .minY = minY->getValue(),
      .maxY = maxY->getValue()
    };
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

void MarketDataModel::setData(DataSet points) {
  auto prev = _bounds;
  std::ranges::sort(points, PointSorter{});
  beginResetModel();
  _points = std::move(points);
  endResetModel();

  _recalcMinMax();
  if (prev != _bounds) {
    emit boundsChanged();
  }
}

} // namespace Calculus
