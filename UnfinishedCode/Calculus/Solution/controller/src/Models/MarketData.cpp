#include "Models/MarketData.h"
#include "Models/Operators/Identity.h"
#include "Models/Operators/SMA.h"

#include <algorithm>
#include <iostream>
#include <qxyseries.h>

namespace Calculus::inline Models::MarketData {

namespace {

using dseconds = std::chrono::duration<double>;
using dstime = std::chrono::time_point<Time::clock, dseconds>;

} // namespace

MarketDataModel::MarketDataModel() : MarketDataModel(nullptr) {
}

MarketDataModel::MarketDataModel(QObject *parent) :
  Identity(parent),
  _operators({
    this,
    new Operators::SMA(3, this)
  }) {
}

MarketDataModel::~MarketDataModel() = default;

QHash<int, QByteArray> MarketDataModel::roleNames() const {
  return QHash<int, QByteArray>{
    { static_cast<int>(Role::Time), "Time" },
    { static_cast<int>(Role::Value), "Value" }
  };
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
  return static_cast<int>(_points.size());
}

auto MarketDataModel::minX() const noexcept -> QVariant {
  return ToQVariant(_bounds.transform([](const Bounds& bounds) {
    return ToQDateTime(bounds.minX);
  }));
}

auto MarketDataModel::maxX() const noexcept -> QVariant {
  return ToQVariant(_bounds.transform([](const Bounds& bounds) {
    return ToQDateTime(bounds.maxX);
  }));
}

auto MarketDataModel::minY() const noexcept -> QVariant {
  return ToQVariant(_bounds.transform([](const Bounds& bounds) {
    return static_cast<double>(bounds.minY) / 100.0;
  }));
}

auto MarketDataModel::maxY() const noexcept -> QVariant {
  return ToQVariant(_bounds.transform([](const Bounds& bounds) {
    return static_cast<double>(bounds.maxY) / 100.0;
  }));
}

QList<Operator*> MarketDataModel::operators() const noexcept {
  return QList(_operators.begin(), _operators.end());
}

auto MarketDataModel::getSubRange(QDateTime minTime, QDateTime maxTime) const noexcept -> Subrange {
  using clock = Time::clock;
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

auto MarketDataModel::getBefore(Time time) const noexcept -> DataSet::const_iterator {
  return std::ranges::lower_bound(
    _points.begin(), _points.end(),
    time, std::less<>{}, &MarketPoint::getTime
  );
}

auto MarketDataModel::getPartialPoint(Time time) const -> std::optional<PartialMarketPoint> {
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
  // TODO: Move this to QML
  Operator::YBounds bounds{};
  using clock = Time::clock;
  auto minUtcTime = clock_cast<clock>(minTime.toStdSysMilliseconds());
  auto maxUtcTime = clock_cast<clock>(maxTime.toStdSysMilliseconds());

  for (auto& op : _operators) {
    auto numColumns = op->columnCount({});
    for (int i = 1; i <= numColumns; ++i) {
      auto opBounds = op->getYBounds(i - 1, minUtcTime, maxUtcTime);
      if (opBounds.has_value()) {
        if (!bounds.has_value()) {
          bounds = *opBounds;
        }
        else {
          bounds->min = (std::min)(opBounds->min, bounds->min);
          bounds->max = (std::max)(opBounds->max, bounds->max);
        }
      }
    }
  }

  if (bounds.has_value()) {
    return QJSValueList{ bounds->min, bounds->max };
  }
  return QJSValueList{};
}

void MarketDataModel::_recalcMinMax() noexcept {
  using namespace std::chrono_literals;
  if (std::ranges::empty(_points)) {
    _bounds = std::nullopt;
  } else {
    auto [minX, maxX] = std::ranges::minmax_element(_points, std::less<>{}, TupleGet<0>);
    auto [minY, maxY] = std::ranges::minmax_element(_points, std::less<>{}, TupleGet<1>);
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
  assert("not yet implemented: update children" && false);
  endInsertRows();
}

void MarketDataModel::setData(DataSet points) {
  auto prev = _bounds;
  std::ranges::sort(points, PointSorter{});
  _points = std::move(points);
  reset(*this);
  for (auto& op : _operators) {
    op->reset(*this);
  }

  _recalcMinMax();
  if (prev != _bounds) {
    emit boundsChanged();
  }
}

} // namespace Calculus
