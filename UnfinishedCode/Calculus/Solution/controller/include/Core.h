#include <QDebug>
#include <QLineSeries>
#include <QObject>
#include <QRandomGenerator>

class LineChart : public QObject {
  Q_OBJECT
public:
  explicit LineChart(QObject *parent = nullptr) : QObject(parent) {
    series = new QLineSeries();
    // Randomize 100 line points
    for (int i = 0; i < 100; ++i) {
      qreal x = i; // X values from 0 to 99
      qreal y = QRandomGenerator::global()->bounded(
          0, 11); // Y values between 0 and 10
      series->append(x, y);
      //qDebug() << "Data point added: (" << x << "," << y << ")";
    }
  }

  Q_INVOKABLE QLineSeries *getSeries() const { return series; }

  Q_INVOKABLE qreal maxX() const {
    if (series->points().isEmpty()) {
      qDebug() << "Series is empty, returning default maxX value 0.";
      return 0; // Or another appropriate default value
    }
    qreal maxXValue = 0;
    for (const QPointF &point : series->points()) {
      if (!std::isnan(point.x()) && !std::isinf(point.x())) {
        maxXValue = std::max(maxXValue, point.x());
      }
    }
    //qDebug() << "Calculated maxX:" << maxXValue;
    return maxXValue;
  }

  Q_INVOKABLE qreal maxY() const {
    if (series->points().isEmpty()) {
      qDebug() << "Series is empty, returning default maxY value 0.";
      return 0; // Or another appropriate default value
    }
    qreal maxYValue = 0;
    for (const QPointF &point : series->points()) {
      if (!std::isnan(point.y()) && !std::isinf(point.y())) {
        maxYValue = std::max(maxYValue, point.y());
      }
    }
    //qDebug() << "Calculated maxY:" << maxYValue;
    return maxYValue;
  }

private:
  QLineSeries *series;
};
