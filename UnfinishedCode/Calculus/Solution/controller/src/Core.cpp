#include <cmath>
#include <utility>
#include <ranges>
#include <QApplication>

#include "Core.h"

Backend::Backend(QApplication &app) :
  QObject(&app),
  MoApp(&app) {
  connect(&app, &QApplication::aboutToQuit, this, &Backend::quit);
}

void Backend::start() {
  MoThread = std::jthread{[this]{ run(); }};
}

void Backend::run() {
  using namespace std::chrono;
  using namespace std::chrono_literals;
  auto startTime = steady_clock::now();
  auto now = startTime;

  do {
    auto diff = now - startTime;
    auto delta = duration_cast<duration<double>>(diff);
    newData(delta.count(), QRandomGenerator::global()->bounded(0, 11));
    std::this_thread::sleep_for(500ms);
    now = steady_clock::now();
  } while (!MbQuit.load(std::memory_order_acquire));
}

void Backend::quit() {
  qDebug() << "Quitting";
  MbQuit.store(true, std::memory_order_release);
}
