#include <QApplication>
#include <cmath>
#include <nlohmann/json.hpp>
#include <ranges>
#include <utility>
#include <generator>

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
        using RandomEngine = std::mt19937;
        using RandomNumber = RandomEngine::result_type;
        constexpr auto LnGenerateCount = 100;
        auto LoEngine = QQmlEngine::contextForObject(this)->engine();
        using namespace std::chrono_literals;
        auto LoTime = std::chrono::utc_clock::now();
        auto LoRandom = RandomEngine{};
        auto LoValues = std::array<RandomNumber, LnGenerateCount>{};
        LoRandom.seed(42);
        std::ranges::generate(LoValues, LoRandom);

        return LoValues
            | std::views::transform([](RandomNumber FnValue) {
                constexpr auto LnDMax = static_cast<double>(std::numeric_limits<RandomEngine::result_type>::max());
                auto LnDValue = static_cast<double>(FnValue) / LnDMax;
                return 50 + (LnDValue * LnDValue * LnDValue) * 50;
            })
            | std::views::enumerate
            | std::views::reverse
            | std::views::transform([LoTime](auto FoTuple) {
                auto [LoI, LoValue] = FoTuple;
                return MarketData::MarketPoint{ LoTime - std::chrono::minutes(LoI), LoValue };
            })
           | std::ranges::to<std::vector<MarketData::MarketPoint>>();
    }
} // namespace Calculus
