#include "Models/MarketData.h"
#include "Models/Operators/Identity.h"
#include "Models/Operators/SMA.h"

#include <algorithm>
#include <iostream>

namespace Calculus::inline Models::MarketData {

namespace {

using dseconds = std::chrono::duration<double>;
using dstime = std::chrono::time_point<Time::clock, dseconds>;

} // namespace

MarketDataModel::MarketDataModel() : MarketDataModel(nullptr) {
}

MarketDataModel::MarketDataModel(QObject *parent) :
  QAbstractTableModel(parent),
  _operators(std::from_range, std::to_array<std::unique_ptr<Operator>>({
    std::make_unique<Operators::Identity>(),
    std::make_unique<Operators::SMA>(3),
  }) | std::views::as_rvalue) {
}

MarketDataModel::~MarketDataModel() = default;

QHash<int, QByteArray> MarketDataModel::roleNames() const {
  return QHash<int, QByteArray>{
    {static_cast<int>(Role::Time), "Time"},
    {static_cast<int>(Role::Value), "Value"},
    {static_cast<int>(Role::SMA), "SMA"}
  };
}

int MarketDataModel::columnCount(const QModelIndex &parent) const {
  return std::ranges::fold_left(_operators, int{2}, [](int a, std::unique_ptr<Operator> const& op) {
    return a + 1 + op->rowCount();
  });
}

QVariant MarketDataModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return {};

  if (index.row() < 0 || index.row() >= _points.size())
    return {};

  auto col = index.column();
  for (auto&& [ i, op] : std::views::enumerate(_operators)) {
    auto opRows = 1 + op->rowCount();
    if (col < opRows) {
      auto x = index.row() - op->startOffset();
      if (x >= op->size()) {
        qDebug().nospace() << "Bad X for operator " << i << ": x=" << x << " >= size=" << op->size();
        return {};
      }
      if (x < 0) {
        return {};
      }
      if (col == 0)
        return toQDateTime(op->getX()[x]);
      else
        return op->getY(col - 1)[x];
    }
    col -= opRows;
  }
  qDebug() << "Bad column " << index.column() << " -- max is " << columnCount({});
  return {};
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

QVariant MarketDataModel::pointClosestTo(QDateTime time) const noexcept {
  auto utcTime = clock_cast<Time::clock>(time.toStdSysMilliseconds());
  auto high = std::ranges::lower_bound(_points, utcTime, std::less{}, &MarketPoint::getTime);
  if (high == _points.end()) {
    return _points.empty() ? QVariant{} : QVariant::fromValue(*std::prev(high));
  }

  auto low = high;
  while (low->getTime() == high->getTime() && low != _points.begin()) {
    low = std::prev(low);
  }
  if (high->getTime() - utcTime <= abs(utcTime - low->getTime())) {
    return QVariant::fromValue(*high);
  } else {
    return QVariant::fromValue(*low);
  }
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
  using clock = Time::clock;
  auto minUtcTime = clock_cast<clock>(minTime.toStdSysMilliseconds());
  auto maxUtcTime = clock_cast<clock>(maxTime.toStdSysMilliseconds());
  auto subrange = getSubRange(minTime, maxTime);

  auto [itMin, itMax] = std::ranges::minmax_element(subrange, std::less<>{}, &MarketPoint::getValue);
  if (itMin == _points.end())
    return {};

  auto minValue = static_cast<double>(itMin->getValue());
  auto maxValue = static_cast<double>(itMax->getValue());
  if (subrange.begin() != _points.begin()) {
    auto before = std::ranges::prev(subrange.begin());
    auto partial = MarketPoint::GetPartialValue(*before, *subrange.begin(), minUtcTime);
    maxValue = (std::max)(partial, maxValue);
    minValue = (std::min)(partial, minValue);
  }

  if (subrange.end() != _points.end()) {
    auto after = subrange.end();
    auto last = std::ranges::prev(after);
    auto partial = MarketPoint::GetPartialValue(*last, *after, maxUtcTime);
    maxValue = (std::max)(partial, maxValue);
    minValue = (std::min)(partial, minValue);
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
  for (auto& op : _operators) {
    op->reset(*this);
  }
  endResetModel();

  _recalcMinMax();
  if (prev != _bounds) {
    emit boundsChanged();
  }
}

} // namespace Calculus
