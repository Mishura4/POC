#include <QApplication.h>
#include <QLineSeries>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <thread>
#include "Core.h"

int main(int LnArgc, char* LvArgs[])
{
    QApplication LoApp(LnArgc, LvArgs);
    Calculus::Backend LoBackend(&LoApp);
    QQmlApplicationEngine LoEngine;

    auto LoContext = LoEngine.rootContext();

    LoEngine.rootContext()->setContextProperty("backend", &LoBackend);
    QQmlEngine::setContextForObject(&LoBackend, LoContext);
    LoEngine.load(QUrl(QStringLiteral("qrc:/view/Main.qml")));
    if (LoEngine.rootObjects().isEmpty())
        return -1;

    LoBackend.Start();
    return LoApp.exec();
}
