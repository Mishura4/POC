#include "Core.h"
#include <QApplication.h>
#include <QLineSeries>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <thread>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  Calculus::Backend backend(&app);
  QQmlApplicationEngine engine;

  auto context = engine.rootContext();

  engine.rootContext()->setContextProperty("backend", &backend);
  QQmlEngine::setContextForObject(&backend, context);
  engine.load(QUrl(QStringLiteral("qrc:/view/Main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;
  
  backend.start();
  return app.exec();
}
