#ifndef RADICALWARE_CALCULUS_MARKETDATA_H_
#define RADICALWARE_CALCULUS_MARKETDATA_H_

#include <cstdint>
#include <chrono>
#include <utility>

#include <QDateTime>
#include <QtQuick/QtQuick>

#include "Tools.h"
#include "MarketPoint.h"

namespace Calculus::inline Models {

class MarketDataModel : public QAbstractTableModel {
  Q_OBJECT
  Q_PROPERTY(QVariant minX READ minX NOTIFY boundsChanged)
  Q_PROPERTY(QVariant maxX READ maxX NOTIFY boundsChanged)
  Q_PROPERTY(QVariant minY READ minY NOTIFY boundsChanged)
  Q_PROPERTY(QVariant maxY READ maxY NOTIFY boundsChanged)

  struct PointSorter {
    static constexpr bool operator()(const MarketPoint& a, const MarketPoint& b) noexcept {
      return a.getTime() < b.getTime();
    }
  };

public:
  using DataSet = std::vector<MarketPoint>;
  using value_type = typename DataSet::value_type;
  using reference = typename DataSet::reference;
  using iterator = typename DataSet::iterator;
  using const_iterator = typename DataSet::const_iterator;
  using X = std::tuple_element_t<0, value_type>;
  using Y = std::tuple_element_t<1, value_type>;
  using QmlX = value_type::QmlTime;
  using QmlY = value_type::QmlValue;

  class Operator {
  public:
    Operator() noexcept = default;
    Operator(const Operator&) noexcept = default;
    Operator(Operator&&) noexcept = default;
    Operator& operator=(const Operator&) noexcept = default;
    Operator& operator=(Operator&&) noexcept = default;
    virtual ~Operator() = default;

    virtual auto getX(const MarketDataModel& dataSet, X x) const noexcept -> std::optional<X> = 0;
    virtual auto getY(const MarketDataModel& dataSet, Y y, int row = 0) noexcept -> std::optional<Y> = 0;
    virtual auto rowCount() const noexcept -> int = 0;
  };

  enum class Role {
    Time,
    Value,
    SMA
  };

  explicit MarketDataModel(QObject* parent = nullptr) noexcept;

  void addPoint(MarketPoint point);
  template <typename Range>
  void setData(Range &&range) {
    auto points = DataSet(std::from_range, std::forward<Range>(range));
    setData(DataSet(std::from_range, std::forward<Range>(range)));
  }
  void setData(DataSet points);

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
  using Subrange = std::ranges::subrange<DataSet::const_iterator>;

  auto getBefore(MarketPoint::Time time) const noexcept -> DataSet::const_iterator;
  auto getSimpleMovingAverage(DataSet::const_iterator where, ptrdiff_t span = 14) const noexcept -> std::optional<double>;
  auto getPartialPoint(MarketPoint::Time time) const -> std::optional<PartialMarketPoint>;
  auto getSubRange(QDateTime minTime, QDateTime maxTime) const noexcept -> Subrange;

  void _recalcMinMax() noexcept;
  struct Bounds {
    X minX{};
    X maxX{};
    Y minY{};
    Y maxY{};

    constexpr friend auto operator==(Bounds lhs, Bounds rhs) noexcept -> bool = default;
  };

  DataSet _points;
  std::vector<std::unique_ptr<Operator>> _operators;
  std::optional<Bounds> _bounds;
};

} // namespace Calculus

#endif // RADICALWARE_CALCULUS_MARKETDATA_H_
