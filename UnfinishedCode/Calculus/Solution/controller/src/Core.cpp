#include <cmath>
#include <utility>
#include <ranges>
#include <QLineSeries>

#include "Core.h"

LineChart::LineChart(QObject *parent): QLineSeries(parent) {
  _setTestPoints();
}

LineChart::LineChart(QLineSeriesPrivate &dd, QObject *parent) : QLineSeries(dd, parent) {
  _setTestPoints();
}

void LineChart::_clearPoints() {
  clear();
  MnMinY = 0;
  MnMaxY = 0;
}

void LineChart::_addPointNoFlush(qreal x, qreal y) {
  if (!std::isfinite(x) || !std::isfinite(y))
      return;
  
  append(x, y);
  MnMinX = std::min(MnMinX, x);
  MnMaxX = std::max(MnMaxX, x);
  MnMinY = std::min(MnMinY, y);
  MnMaxY = std::max(MnMaxY, y);
}

void LineChart::_flushPoints() {
  qDebug() << "Chart range: Min {" << MnMinX << ", " << MnMinY << "}, Max {" << MnMaxX << ", " << MnMaxY << '}';
}

void LineChart::_setTestPoints() {
  setPoints(std::views::iota(0, 100) | std::views::transform([](int i) {
    qreal x = i; // X values from 0 to 99
    qreal y = QRandomGenerator::global()->bounded(
        0, 11); // Y values between 0 and 10
    return std::pair{x, y};
  }));
}
