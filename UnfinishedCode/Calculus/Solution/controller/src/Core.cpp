#include <cmath>
#include <utility>
#include <ranges>
#include <QApplication>
#include <nlohmann/json.hpp>

#include "../include/Models/MarketData.h"
#include "Core.h"

namespace Calculus {

Backend::Backend(QObject* parent) :
  QObject(parent),
  MoModel(new MarketData::MarketDataModel(this)) {
  connect(QApplication::instance(), &QApplication::aboutToQuit, this, &Backend::quit);
}

void Backend::start() {
  MoThread = std::jthread{[this]{ run(); }};
}

void Backend::run() {
  using namespace std::chrono;
  using namespace std::chrono_literals;
  auto startTime = system_clock::now();
  auto now = startTime;

  do {
    auto diff = now - startTime;
    auto delta = duration_cast<duration<double>>(diff);
    newData(delta.count(), QRandomGenerator::global()->bounded(0, 11));
    std::this_thread::sleep_for(500ms);
    now = system_clock::now();
  } while (!MbQuit.load(std::memory_order_acquire));
}

void Backend::queryMarketData(QDateTime start, QDateTime end, QJSValue callback) {
  auto engine = QQmlEngine::contextForObject(this)->engine();

  if (!callback.isUndefined() && !callback.isCallable()) {
    engine->throwError(QJSValue::ErrorType::TypeError, "Invalid callback argument");
    return;
  }

  runAsync([this, callback](std::vector<MarketData::MarketPoint> values) {
    callback.call(QJSValueList{ toJSVariant(values) });
    MoModel->setData(std::move(values));
  }, &Backend::doQueryMarketData, this, start, end);
}

void Backend::quit() {
  qDebug() << "Quitting";
  MbQuit.store(true, std::memory_order_release);
}

auto Backend::doQueryMarketData(QDateTime start, QDateTime end) -> std::vector<MarketData::MarketPoint> {
  auto engine = QQmlEngine::contextForObject(this)->engine();
  using namespace std::chrono_literals;
  auto now = std::chrono::utc_clock::now();
  return std::vector<MarketData::MarketPoint> {
    { now - 10min, 500 },
    { now - 9min, 400 },
    { now - 8min, 800 },
    { now - 7min, 200 },
    { now - 6min, 100 },
    { now - 5min, 200 },
    { now - 4min, 100 },
    { now - 3min, 800 },
    { now - 2min, 500 },
    { now - 1min, 400 },
  };
}

} // namespace Calculus
