//
// Created by miuna on 4/26/2026.
//

#ifndef RADICALWARE_CALCULUS_MARKETPOINT_H
#define RADICALWARE_CALCULUS_MARKETPOINT_H

#include <chrono>
#include <cstdint>
#include <utility>

#include <QDateTime>
#include <QtQuick/QtQuick>

#include "Tools.h"

namespace Calculus::inline Models::MarketData
{
    using Time = std::chrono::utc_time<std::chrono::milliseconds>;
    using Value = int64_t;

    class MarketPoint
    {
    public:
        using QmlValue = qreal;
        using QmlTime = QDateTime;

    private:
        Q_GADGET
        Q_PROPERTY(QmlTime time READ time)
        Q_PROPERTY(QmlValue value READ value)
        QML_VALUE_TYPE(marketData)

    public:
        constexpr MarketPoint() noexcept = default;
        MarketPoint(QDateTime FoTime, Value FnValue) noexcept;

        template <typename Clock, typename Duration>
        constexpr MarketPoint(std::chrono::time_point<Clock, Duration> time, Value FnValue) noexcept :
            MoTime(clock_cast<Time::clock>(time_point_cast<Time::duration>(time))),
            MnValue(FnValue)
        {
        }

        template <typename Clock, typename Duration>
        constexpr MarketPoint(
            std::chrono::time_point<Clock, Duration> FoTime, std::floating_point auto FnValue
        ) noexcept :
            MoTime(clock_cast<Time::clock>(time_point_cast<Time::duration>(FoTime))),
            MnValue(static_cast<Value>(std::floor(FnValue * 100)))
        {
        }

        auto time() const -> QmlTime;

        auto GetTime() const noexcept -> Time { return MoTime; }

        auto value() const noexcept -> QmlValue { return static_cast<QmlValue>(MnValue) / 100; }

        auto GetValue() const noexcept -> Value { return MnValue; }

        // std::get counterpart
        template <size_t I>
            requires(I == 0)
        friend auto get(const MarketPoint& FoPoint) noexcept -> Time
        {
            return FoPoint.GetTime();
        }

        // std::get counterpart
        template <size_t I>
            requires(I == 1)
        friend auto get(const MarketPoint& FoPoint) noexcept -> Value
        {
            return FoPoint.GetValue();
        }

        static double GetPartialValue(MarketPoint FoBefore, MarketPoint FoAfter, Time FoTime) noexcept;

        friend QJSValue ToJSValue(const MarketPoint& FoPoint);

    private:
        Time MoTime;
        Value MnValue;
    };

    class PartialMarketPoint : public MarketPoint
    {
    public:
        PartialMarketPoint(MarketPoint FoBefore, MarketPoint FoAfter, Time FoTime) noexcept;

    private:
        MarketPoint MoBefore;
        MarketPoint MoAfter;
    };

} // namespace Calculus::inline Models::MarketData

template <>
struct std::tuple_size<Calculus::MarketData::MarketPoint>
{
    static constexpr size_t value = 2;
};

template <>
struct std::tuple_element<0, Calculus::MarketData::MarketPoint>
{
    using type = Calculus::MarketData::Time;
};

template <>
struct std::tuple_element<1, Calculus::MarketData::MarketPoint>
{
    using type = Calculus::MarketData::Value;
};

template <typename C>
struct std::formatter<Calculus::MarketData::MarketPoint, C>
{
    constexpr auto parse(auto& ctx)
    {
        auto it = ctx.begin();
        while (it != ctx.end() && *it != '}')
        {
            ++it;
        }
        return it;
    }

    auto format(const Calculus::MarketData::MarketPoint p, auto& ctx) const
    {
        return std::format_to(ctx.out(), "{{{}, {}}}", p.GetTime(), p.value());
    }
};

#endif // RADICALWARE_CALCULUS_MARKETPOINT_H
