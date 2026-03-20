#include "Core.h"
#include <QApplication.h>
#include <QLineSeries>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <thread>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv); // Use QApplication instead of QGuiApplication
  QQmlApplicationEngine engine;
  Backend backend(app);

  backend.start();
  engine.rootContext()->setContextProperty("backend", &backend);
  engine.load(QUrl(QStringLiteral("qrc:/view/Main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
