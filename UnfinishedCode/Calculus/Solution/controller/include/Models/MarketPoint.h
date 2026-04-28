//
// Created by miuna on 4/26/2026.
//

#ifndef RADICALWARE_CALCULUS_MARKETPOINT_H
#define RADICALWARE_CALCULUS_MARKETPOINT_H

#include <cstdint>
#include <chrono>
#include <utility>

#include <QDateTime>
#include <QtQuick/QtQuick>

#include "Tools.h"

namespace Calculus::inline Models {

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

  friend QJSValue toJSValue(const MarketPoint& point);

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

#endif // RADICALWARE_CALCULUS_MARKETPOINT_H
