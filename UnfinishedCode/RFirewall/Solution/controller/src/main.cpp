#include <QGuiApplication>
#include <QQuickStyle>
#include <QLoggingCategory>

#ifdef _WIN32
#include <windows.h>
#endif

#define _uint_

#include "Core.h"
#include "Nexus.h"

#ifdef _WIN32
int main(int argc, char *argv[]) {
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
#endif
  int LnReValue = 0;
  Begin();
  Nexus<>::Start();
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  // Silence noisy libpng iCCP warnings from Qt image IO
  QLoggingCategory::setFilterRules(QStringLiteral(
      "qt.gui.imageio.warning=false\n"
      "qt.imageio.png.warning=false"));
  QGuiApplication LoApp(argc, argv);
  // Use a non-native QQC2 style so customizations (e.g., background) work
  // Options include: "Basic", "Fusion", "Material", "Imagine", "Universal"
  QQuickStyle::setStyle("Basic");
  Core LoCore;

  if (LoCore.Initialize())
    LnReValue = LoApp.exec();
  else
    LnReValue = -1;

  RescuePrint();
  LnReValue = Nexus<>::Stop();
  return LnReValue;
}