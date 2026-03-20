#include "Core.h"
#include <QApplication.h>
#include <QLineSeries>
#include <QQmlApplicationEngine>
#include <QQmlContext>

inline constexpr char MODULE_URL[] = "com.github.Radicalware.Calculus";

int main(int argc, char *argv[]) {
  QApplication app(argc, argv); // Use QApplication instead of QGuiApplication
  QQmlApplicationEngine engine;

  engine.load(QUrl(QStringLiteral("qrc:/view/Main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;
  return app.exec();
}
