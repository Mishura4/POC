#include "Core.h"
#include <QApplication.h>
#include <QLineSeries>
#include <QQmlApplicationEngine>
#include <QQmlContext>



int main(int argc, char *argv[]) {
  QApplication app(argc, argv); // Use QApplication instead of QGuiApplication
  QQmlApplicationEngine engine;

  LineChart lineChart;
  engine.rootContext()->setContextProperty("lineChart", &lineChart);

  engine.load(QUrl(QStringLiteral("qrc:/view/Main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;
  return app.exec();
}
