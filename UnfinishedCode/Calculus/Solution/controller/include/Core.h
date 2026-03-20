#include <thread>
#include <QDebug>
#include <QtGraphs/QtGraphs>
#include <QObject>
#include <QRandomGenerator>
#include <QtQml>

#include <RawMapping.h>

class Backend : public QObject
{
  Q_OBJECT

public:
  Backend(QApplication& app);

  void start();

  void run();

signals:
  void newData(qreal time, qreal value);

public slots:
  void quit();

private:
  QApplication *MoApp;
  std::atomic<bool> MbQuit = false;
  std::jthread MoThread;
};
