#include "Models/MarketData.h"

#include <iostream>

namespace Calculus {

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

void MarketDataModel::_recalcMinMax() {
  using namespace std::chrono_literals;
  auto prevMinX = minX();
  auto prevMaxX = maxX();
  auto prevMinY = minY();
  auto prevMaxY = maxY();
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
  if (prevMinX != _minXindex || prevMaxX != _maxXindex || prevMinY != _minYindex || prevMaxY != _maxYindex) {
    emit boundsChanged();
  }
}

void MarketDataModel::addPoint(MarketPoint point) {
  int row = static_cast<int>(_points.size());
  _points.reserve(_points.size() + 1);
  beginInsertRows(QModelIndex(), row, row + 1);
  _points.push_back(point);
  endInsertRows();
}

void MarketDataModel::setPoints(Points points) {
  beginResetModel();
  auto onExit = onScopeExit{[this] {
    endResetModel();
  }};
  _points = std::move(points);
  _recalcMinMax();
}

} // namespace Calculus
