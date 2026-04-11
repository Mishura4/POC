#include <thread>
#include <QDebug>
#include <QtGraphs/QtGraphs>
#include <QObject>
#include <QRandomGenerator>
#include <QtQml>

#include <RawMapping.h>

#include "Models/MarketData.h"

namespace Calculus {

class Backend : public QObject
{
  Q_OBJECT
  Q_PROPERTY(const MarketDataModel* model READ model CONSTANT)

public:
  Backend(QObject* parent = nullptr);

  void start();
  void run();

  Q_INVOKABLE void queryMarketData(QDateTime start, QDateTime end, QJSValue callback);

  const MarketDataModel* model() const noexcept { return MoModel; }

signals:
  void newData(qreal time, qreal value);

public slots:
  void quit();

private:
  auto doQueryMarketData(QDateTime start, QDateTime end) -> std::vector<MarketPoint>;

  template <typename T = QJSValue>
  auto toJSVariant(auto&& value, QQmlEngine* engine = nullptr);

  template <typename Callback, typename Fun, typename... Args>
    requires (std::invocable<Fun, Args...> && std::invocable<Callback, std::invoke_result_t<Fun, Args...>>)
  auto runAsync(Callback&& callback, Fun&& fun, Args&&... args) -> QFuture<void>;

  template <typename Fun, typename... Args>
    requires (std::invocable<Fun, Args...>)
  auto runAsync(QJSValue callback, Fun&& fun, Args&&... args) -> QFuture<void>;

  MarketDataModel* MoModel;
  std::atomic<bool> MbQuit = false;
  std::mutex MoMutex;
  std::jthread MoThread;
};

template <typename T>
auto Backend::toJSVariant(auto &&value, QQmlEngine *engine) {
  if (engine == nullptr)
    engine = QQmlEngine::contextForObject(this)->engine();

  if constexpr (!std::is_void_v<T>) {
    return T(QJSManagedValue(
        QVariant::fromValue(std::forward<decltype(value)>(value)), engine));
  } else {
    return QJSManagedValue(
        QVariant::fromValue(std::forward<decltype(value)>(value)), engine);
  }
}

template <typename Callback, typename Fun, typename... Args>
  requires (std::invocable<Fun, Args...> && std::invocable<Callback, std::invoke_result_t<Fun, Args...>>)
auto Backend::runAsync(Callback &&callback, Fun &&fun, Args &&...args) -> QFuture<void> {
  using arg_tuple = std::tuple<std::remove_cvref_t<Args>...>;
  return QtConcurrent::run(
    [this, cb = std::forward<Callback>(callback), fn = std::forward<Fun>(fun),
     argt = arg_tuple(std::forward<Args>(args)...)]() mutable {
      if constexpr (std::is_void_v<std::invoke_result_t<Fun, Args...>>) {
        std::apply(std::forward<Fun>(fn), std::move(argt)); // Invoke fun on async thread
        QMetaObject::invokeMethod(
          this,
          [cb = std::forward<Callback>(cb)] {
            std::invoke(cb); // Run callback on app thread
          },
          Qt::QueuedConnection
        );
      } else {
        // Invoke fun on async thread (in capture)
        QMetaObject::invokeMethod(
          this,
          [cb = std::forward<Callback>(cb),
           ret = std::apply(std::forward<Fun>(fn), std::move(argt))] {
            std::invoke(cb, ret); // Run callback on app thread
          },
          Qt::QueuedConnection
        );
      }
    }
  );
}

template <typename Fun, typename... Args>
  requires (std::invocable<Fun, Args...>)
auto Backend::runAsync(QJSValue callback, Fun &&fun, Args&&... args) -> QFuture<void> {
  return runAsync(
    [this, cb = std::move(callback)]<typename... CbArgs>(CbArgs&& ...cbArgs) {
      auto engine = QQmlEngine::contextForObject(this)->engine();
      cb.call(QJSValueList{ this->toJSVariant(std::forward<CbArgs>(cbArgs), engine)... });
    },
    std::forward<Fun>(fun), std::forward<Args>(args)...
  );
}

} // namespace Calculus
