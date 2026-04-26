#ifndef RADICALWARE_CALCULUS_MARKETDATA_H_
#define RADICALWARE_CALCULUS_MARKETDATA_H_
#include <cstdint>
#include <chrono>
#include <utility>
#include <QDateTime>
#include <QtQuick/QtQuick>
#include "Tools.h"

namespace Calculus {

class MarketPoint {
public:
  using Value = uint64_t;
  using Time = std::chrono::utc_time<std::chrono::milliseconds>;
  using QmlValue = qreal;
  using QmlTime = QDateTime;

private:
  Q_GADGET
  Q_PROPERTY(QmlTime time READ time)
  Q_PROPERTY(QmlValue value READ value)
  QML_VALUE_TYPE(marketData)

public:
  constexpr MarketPoint() noexcept = default;
  MarketPoint(QDateTime time, Value value) noexcept;

  template <typename Clock, typename Duration>
  constexpr MarketPoint(std::chrono::time_point<Clock, Duration> time, Value value) noexcept :
    MoTime(clock_cast<std::chrono::utc_clock>(time_point_cast<Time::duration>(time))),
    MnValue(value)
  {}

  auto time() const -> QmlTime;

  auto getTime() const noexcept -> Time {
    return MoTime;
  }

  auto value() const noexcept -> QmlValue {
    return static_cast<QmlValue>(MnValue) / 100;
  }

  auto getValue() const noexcept -> Value {
    return MnValue;
  }

  template <size_t I>
  requires (I == 0)
  friend auto get(const MarketPoint& point) noexcept -> Time {
    return point.getTime();
  }

  template <size_t I>
  requires (I == 1)
  friend auto get(const MarketPoint& point) noexcept -> Value {
    return point.getValue();
  }

  static double GetPartialValue(MarketPoint before, MarketPoint after, MarketPoint::Time time) noexcept;

private:
  Time      MoTime;
  Value     MnValue;
};

class PartialMarketPoint : public MarketPoint {
public:
  PartialMarketPoint(MarketPoint before, MarketPoint after, MarketPoint::Time time) noexcept;

private:
  MarketPoint MoBefore;
  MarketPoint MoAfter;
};

}

template <>
struct std::tuple_size<Calculus::MarketPoint> {
  static constexpr size_t value = 2;
};

template <>
struct std::tuple_element<0, Calculus::MarketPoint> {
  using type = Calculus::MarketPoint::Time;
};

template <>
struct std::tuple_element<1, Calculus::MarketPoint> {
  using type = Calculus::MarketPoint::Value;
};

namespace Calculus {

class MarketDataModel : public QAbstractTableModel {
  Q_OBJECT
  Q_PROPERTY(QVariant minX READ minX NOTIFY boundsChanged)
  Q_PROPERTY(QVariant maxX READ maxX NOTIFY boundsChanged)
  Q_PROPERTY(QVariant minY READ minY NOTIFY boundsChanged)
  Q_PROPERTY(QVariant maxY READ maxY NOTIFY boundsChanged)

  using Points = std::vector<MarketPoint>;

  struct PointSorter {
    static constexpr bool operator()(const MarketPoint& a, const MarketPoint& b) noexcept {
      return a.getTime() < b.getTime();
    }
  };

public:
  using value_type = typename Points::value_type;
  using reference = typename Points::reference;
  using iterator = typename Points::iterator;
  using const_iterator = typename Points::const_iterator;
  using X = std::tuple_element_t<0, value_type>;
  using Y = std::tuple_element_t<1, value_type>;
  using QmlX = value_type::QmlTime;
  using QmlY = value_type::QmlValue;

  enum class Role {
    Time,
    Value,
    SMA
  };

  explicit MarketDataModel(QObject* parent = nullptr) noexcept;

  void addPoint(MarketPoint point);
  template <typename Range>
  void setPoints(Range &&range) {
    auto points = Points(std::from_range, std::forward<Range>(range));
    setPoints(Points(std::from_range, std::forward<Range>(range)));
  }
  void setPoints(Points points);

  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex &parent) const override {
    return static_cast<int>(_points.size());
  }
  int columnCount(const QModelIndex &parent) const override { return 4; }
  QVariant data(const QModelIndex &index, int role) const override;

  auto begin() noexcept -> iterator;
  auto begin() const noexcept -> const_iterator;
  auto end() noexcept -> iterator;
  auto end() const noexcept -> const_iterator;
  auto size() const noexcept -> int;

  QVariant minX() const noexcept;
  QVariant maxX() const noexcept;
  QVariant minY() const noexcept;
  QVariant maxY() const noexcept;

  Q_INVOKABLE QJSValue getMinY(QDateTime minTime, QDateTime maxTime) const;
  Q_INVOKABLE QJSValue getMaxY(QDateTime minTime, QDateTime maxTime) const;
  // Q_INVOKABLE QVariant getPartialPoint(QDateTime time) const;
  Q_INVOKABLE QJSValueList getBoundsY(QDateTime minTime, QDateTime maxTime) const;

  auto getMinX() const noexcept -> std::optional<X>;
  auto getMaxX() const noexcept -> std::optional<X>;
  auto getMinY() const noexcept -> std::optional<Y>;
  auto getMaxY() const noexcept -> std::optional<Y>;

signals:
  void boundsChanged();

private:
  using Subrange = std::ranges::subrange<Points::const_iterator>;

  auto getBefore(MarketPoint::Time time) const noexcept -> Points::const_iterator;
  auto getSimpleMovingAverage(Points::const_iterator where, ptrdiff_t span = 14) const noexcept -> std::optional<double>;
  auto getPartialPoint(MarketPoint::Time time) const -> std::optional<PartialMarketPoint>;
  auto getSubRange(QDateTime minTime, QDateTime maxTime) const noexcept -> Subrange;

  void _recalcMinMax() noexcept;
  template <typename NewPoints>
  void _updateMinMax(const NewPoints& range);

  Points _points;
  int _minXindex = -1;
  int _maxXindex = -1;
  int _minYindex = -1;
  int _maxYindex = -1;
};

} // namespace Calculus

#endif // RADICALWARE_CALCULUS_MARKETDATA_H_
