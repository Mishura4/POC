#include <QDebug>
#include <QLineSeries>
#include <QObject>
#include <QRandomGenerator>

#include <RawMapping.h>

class LineChart : public QLineSeries {
  Q_OBJECT
  Q_PROPERTY(qreal maxX READ maxX)
  Q_PROPERTY(qreal maxY READ maxY)
  QML_ELEMENT

public:
  explicit LineChart(QObject *parent = nullptr);

  /**
   * @brief Clear all points and replaces them with a range of points
   * @param range Range of {x, y} tuples to set the line chart to
   */
  void setPoints(std::ranges::input_range auto&& range);

  Q_INVOKABLE qreal maxX() const noexcept { return MnMaxX; }
  Q_INVOKABLE qreal maxY() const noexcept { return MnMaxY; }

protected:
    LineChart(QLineSeriesPrivate &dd, QObject *parent = nullptr);

private:
  /**
   * @brief Internal function to remove all points from the line chart
   */
  void _clearPoints();

  /**
   * @brief Internal function to add a point to the line chart
   * @param x X coordinate
   * @param y Y coordinate
   */
  void _addPointNoFlush(qreal x, qreal y);

  /**
   * @brief Update variables and/or log information after adding points
   */
  void _flushPoints();

  void _setTestPoints();

  qreal MnMinX{0};
  qreal MnMaxX{0};
  qreal MnMinY{0};
  qreal MnMaxY{0};
};

void LineChart::setPoints(std::ranges::input_range auto&& range) {
  // Ensure we flush at scope exit even if an exception occurs
  struct ScopeExit {
    void operator()(LineChart* chart) const {
      chart->_flushPoints();
    }
  };
  auto onExit = std::unique_ptr<LineChart, ScopeExit>(this);

  _clearPoints();

  for (const auto& [x, y] : range) {
    _addPointNoFlush(static_cast<qreal>(x), static_cast<qreal>(y));
  }
}
