#include <QDebug>
#include <QObject>
#include <QRandomGenerator>
#include <QtGraphs/QtGraphs>
#include <QtQml>
#include <thread>

#include <RawMapping.h>

#include "Models/MarketData.h"

namespace Calculus
{
    class Backend : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(const MarketData::MarketDataModel* model READ Model CONSTANT)

    public:
        Backend(QObject* FoParent = nullptr);

        void Start();
        void Run();

        Q_INVOKABLE void QueryMarketData(QDateTime FoStart, QDateTime FoEnd, QJSValue FoCallback = {});

        const MarketData::MarketDataModel* Model() const noexcept { return MoModel; }

    signals:
        void newData(qreal FoTime, qreal FoValue);

    public slots:
        void Quit();

    private:
        auto DoQueryMarketData(QDateTime FoStart, QDateTime FoEnd) -> std::vector<MarketData::MarketPoint>;

        template <typename T = QJSValue>
        auto ToJSVariant(auto&& FxValue, QQmlEngine* FoEngine = nullptr);

        template <typename Callback, typename Fun, typename... Args>
            requires(std::invocable<Fun, Args...> && std::invocable<Callback, std::invoke_result_t<Fun, Args...>>)
        auto RunAsync(Callback&& FfCallback, Fun&& FfTask, Args&&... FxArgs) -> QFuture<void>;

        template <typename Fun, typename... Args>
            requires(std::invocable<Fun, Args...>)
        auto RunAsync(QJSValue FfCallback, Fun&& FfTask, Args&&... FxArgs) -> QFuture<void>;

        MarketData::MarketDataModel* MoModel;
        std::atomic<bool> MbQuit = false;
        std::mutex MoMutex;
        std::jthread MoThread;
    };

    template <typename T>
    auto Backend::ToJSVariant(auto&& FxValue, QQmlEngine* FoEngine)
    {
        if (FoEngine == nullptr)
            FoEngine = QQmlEngine::contextForObject(this)->engine();

        if constexpr (!std::is_void_v<T>)
        {
            return T(QJSManagedValue(QVariant::fromValue(std::forward<decltype(FxValue)>(FxValue)), FoEngine));
        }
        else
        {
            return QJSManagedValue(QVariant::fromValue(std::forward<decltype(FxValue)>(FxValue)), FoEngine);
        }
    }

    template <typename Callback, typename Fun, typename... Args>
        requires(std::invocable<Fun, Args...> && std::invocable<Callback, std::invoke_result_t<Fun, Args...>>)
    auto Backend::RunAsync(Callback&& FfCallback, Fun&& FfTask, Args&&... FxArgs) -> QFuture<void>
    {
        using arg_tuple = std::tuple<std::remove_cvref_t<Args>...>;
        return QtConcurrent::run(
            [this,
             LfCallback = std::forward<Callback>(FfCallback),
             LfTask = std::forward<Fun>(FfTask),
             LoArgTuple = arg_tuple(std::forward<Args>(FxArgs)...)]() mutable
            {
                if constexpr (std::is_void_v<std::invoke_result_t<Fun, Args...>>)
                {
                    std::apply(std::forward<Fun>(LfTask), std::move(LoArgTuple)); // Invoke fun on async thread
                    QMetaObject::invokeMethod(
                        this,
                        [LfCallback2 = std::forward<Callback>(LfCallback)]
                        {
                            std::invoke(LfCallback2); // Run callback on app thread
                        },
                        Qt::QueuedConnection);
                }
                else
                {
                    // Invoke fun on async thread (in capture)
                    QMetaObject::invokeMethod(
                        this,
                        [LfCallback2 = std::forward<Callback>(LfCallback), LoResult = std::apply(std::forward<Fun>(LfTask), std::move(LoArgTuple))]
                        {
                            std::invoke(LfCallback2, LoResult); // Run callback on app thread
                        },
                        Qt::QueuedConnection);
                }
            });
    }

    template <typename Fun, typename... Args>
        requires(std::invocable<Fun, Args...>)
    auto Backend::RunAsync(QJSValue FfCallback, Fun&& FfTask, Args&&... FxArgs) -> QFuture<void>
    {
        return runAsync(
            [this, LfCallback = std::move(FfCallback)]<typename... CbArgs>(CbArgs&&... cbArgs)
            {
                auto LoEngine = QQmlEngine::contextForObject(this)->engine();
                LfCallback.call(QJSValueList{ this->ToJSVariant(std::forward<CbArgs>(cbArgs), LoEngine)... });
            },
            std::forward<Fun>(FfTask),
            std::forward<Args>(FxArgs)...);
    }
} // namespace Calculus
