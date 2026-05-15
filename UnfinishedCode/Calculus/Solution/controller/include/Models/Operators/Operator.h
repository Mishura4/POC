//
// Created by miuna on 4/28/2026.
//

#ifndef CALCULUS_OPERATOR_H
#define CALCULUS_OPERATOR_H

#include <vector>
#include <span>
#include <tuple>
#include <optional>

#include "MarketPoint.h"

namespace Calculus::inline Models::MarketData {

class MarketDataModel;

class Operator : public QAbstractTableModel {
  Q_OBJECT

public:
  using X = Time;
  using Y = double;
  using Point = std::tuple<X, Y>;
  using XColumn = std::span<const X>;
  using YColumn = std::span<const Y>;
  using XSubrange = std::ranges::subrange<XColumn::const_iterator>;
  using YBounds = std::optional<std::ranges::minmax_result<Y>>;

  virtual void reset(const MarketDataModel& dataSet) = 0;
  virtual auto getX() const noexcept -> XColumn = 0;
  virtual auto getY(int column) const noexcept -> YColumn = 0;
  virtual auto getYBounds(int column, std::optional<Time> minX, std::optional<Time> maxX) const noexcept -> YBounds;
  Q_INVOKABLE QJSValueList getYBounds(int column, QDateTime minX, QDateTime maxX) const noexcept;
  Q_INVOKABLE virtual QVariant pointClosestTo(QDateTime time) const noexcept;
  Q_INVOKABLE virtual QVariant operator[](int index) const;
  Q_INVOKABLE QVariant at(int index) const;

  static auto getTimeRange(const XColumn& values, Time min, Time max) noexcept -> XSubrange;

  int rowCount(const QModelIndex &parent) const override { return MnSize; }
  int columnCount(const QModelIndex &parent) const override { return MnColumnCount + 1; }
  QVariant data(const QModelIndex &index, int role) const override;

protected:
  explicit Operator(QObject* parent, const QString& name, int columnCount, int startOffset = 0, int size = 0);

  constexpr void setSize(int newSize) noexcept { MnSize = newSize; }

private:
  int MnColumnCount;
  int MnStartOffset = 0;
  int MnSize = 0;
};

template <typename Tuple, typename Getter = TupleGetter<0>, typename X = std::invoke_result_t<Getter, Tuple>>
static Tuple InterpolateTuple(X x, Tuple low, Tuple high, Getter getter = {}) noexcept {
  auto factor = InvLerp(x, getter(low), getter(high));
  return Tuple{
    lerp(factor, tuple_get<0>(low), tuple_get<0>(high)),
    lerp(factor, tuple_get<0>(high), tuple_get<1>(high)),
  };
}

}

#endif //CALCULUS_OPERATOR_H
