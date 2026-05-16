#include <QApplication>
#include <cmath>
#include <nlohmann/json.hpp>
#include <ranges>
#include <utility>

#include "../include/Models/MarketData.h"
#include "Core.h"

namespace Calculus
{
    Backend::Backend(QObject* FoParent) : QObject(FoParent), MoModel(new MarketData::MarketDataModel(this))
    {
        connect(QApplication::instance(), &QApplication::aboutToQuit, this, &Backend::Quit);
    }

    void Backend::Start()
    {
        MoThread = std::jthread{ [this] { Run(); } };
    }

    void Backend::Run()
    {
        using namespace std::chrono;
        using namespace std::chrono_literals;
        auto LoStartTime = system_clock::now();
        auto LoNow = LoStartTime;

        do
        {
            auto LnDiff = LoNow - LoStartTime;
            auto LnDelta = duration_cast<duration<double>>(LnDiff);
            newData(LnDelta.count(), QRandomGenerator::global()->bounded(0, 11));
            std::this_thread::sleep_for(500ms);
            LoNow = system_clock::now();
        }
        while (!MbQuit.load(std::memory_order_acquire));
    }

    void Backend::QueryMarketData(QDateTime FoStart, QDateTime FoEnd, QJSValue FoCallback)
    {
        auto LoEngine = QQmlEngine::contextForObject(this)->engine();

        if (!FoCallback.isUndefined() && !FoCallback.isCallable())
        {
            LoEngine->throwError(QJSValue::ErrorType::TypeError, "Invalid callback argument");
            return;
        }

        RunAsync(
            [this, FoCallback](std::vector<MarketData::MarketPoint> FvValues)
            {
                FoCallback.call(QJSValueList{ ToJSVariant(FvValues) });
                MoModel->SetData(std::move(FvValues));
            },
            &Backend::DoQueryMarketData,
            this,
            FoStart,
            FoEnd
        );
    }

    void Backend::Quit()
    {
        qDebug() << "Quitting";
        MbQuit.store(true, std::memory_order_release);
    }

    auto Backend::DoQueryMarketData(QDateTime FoStart, QDateTime FoEnd) -> std::vector<MarketData::MarketPoint>
    {
        auto LoEngine = QQmlEngine::contextForObject(this)->engine();
        using namespace std::chrono_literals;
        auto LoTime = std::chrono::utc_clock::now();
        return std::vector<MarketData::MarketPoint>{
            { LoTime - 10min, 500 }, { LoTime - 9min, 400 }, { LoTime - 8min, 800 }, { LoTime - 7min, 200 },
            { LoTime - 6min, 100 },  { LoTime - 5min, 200 }, { LoTime - 4min, 100 }, { LoTime - 3min, 800 },
            { LoTime - 2min, 500 },  { LoTime - 1min, 400 },
        };
    }
} // namespace Calculus
