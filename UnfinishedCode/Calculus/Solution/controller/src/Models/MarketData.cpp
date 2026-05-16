#include "Models/MarketData.h"
#include "Models/Operators/Identity.h"
#include "Models/Operators/SMA.h"

#include <algorithm>
#include <iostream>
#include <qxyseries.h>

namespace Calculus::inline Models::MarketData
{
    namespace
    {
        using dseconds = std::chrono::duration<double>;
        using dstime = std::chrono::time_point<Time::clock, dseconds>;
    } // namespace

    MarketDataModel::MarketDataModel() : MarketDataModel(nullptr) {}

    MarketDataModel::MarketDataModel(QObject* FoParent) :
        Identity(FoParent), MvOperators({ this, new Operators::SMA(3, this) })
    {
    }

    MarketDataModel::~MarketDataModel() = default;

    auto MarketDataModel::begin() noexcept -> iterator { return MvPoints.begin(); }

    auto MarketDataModel::begin() const noexcept -> const_iterator { return MvPoints.begin(); }

    auto MarketDataModel::end() noexcept -> iterator { return MvPoints.end(); }

    auto MarketDataModel::end() const noexcept -> const_iterator { return MvPoints.end(); }

    auto MarketDataModel::size() const noexcept -> int { return static_cast<int>(MvPoints.size()); }

    auto MarketDataModel::MinX() const noexcept -> QVariant
    {
        return ToQVariant(MoBounds.transform([](const Bounds& FoBounds) { return ToQDateTime(FoBounds.MoMinX); }));
    }

    auto MarketDataModel::MaxX() const noexcept -> QVariant
    {
        return ToQVariant(MoBounds.transform([](const Bounds& FoBounds) { return ToQDateTime(FoBounds.MoMaxX); }));
    }

    auto MarketDataModel::MinY() const noexcept -> QVariant
    {
        return ToQVariant(MoBounds.transform([](const Bounds& FoBounds)
                                             { return static_cast<double>(FoBounds.MnMinY) / 100.0; }));
    }

    auto MarketDataModel::MaxY() const noexcept -> QVariant
    {
        return ToQVariant(MoBounds.transform([](const Bounds& FoBounds)
                                             { return static_cast<double>(FoBounds.MnMaxY) / 100.0; }));
    }

    QList<Operator*> MarketDataModel::Operators() const noexcept
    {
        return QList(MvOperators.begin(), MvOperators.end());
    }

    auto MarketDataModel::GetSubRange(QDateTime FoMinTime, QDateTime FoMaxTime) const noexcept -> Subrange
    {
        using clock = Time::clock;
        auto LoMinUtcTime = clock_cast<clock>(FoMinTime.toStdSysMilliseconds());
        auto LoMaxUtcTime = clock_cast<clock>(FoMaxTime.toStdSysMilliseconds());
        auto LoBegin = std::ranges::lower_bound(
            MvPoints.begin(), MvPoints.end(), LoMinUtcTime, std::less{}, &MarketPoint::GetTime
        );
        auto LoEnd
            = std::ranges::upper_bound(LoBegin, MvPoints.end(), LoMaxUtcTime, std::less{}, &MarketPoint::GetTime);
        return Subrange{ LoBegin, LoEnd };
    }

    QJSValue MarketDataModel::GetMinY(QDateTime FoMinTime, QDateTime FoMaxTime) const
    {
        auto LvSubrange = GetSubRange(FoMinTime, FoMaxTime);
        if (std::ranges::empty(LvSubrange))
        {
            return QJSValue{};
        }
        else
        {
            auto LnMin = std::ranges::min(LvSubrange | std::views::transform(&MarketPoint::GetValue));
            return static_cast<double>(LnMin / 100);
        }
    }

    QJSValue MarketDataModel::GetMaxY(QDateTime FoMinTime, QDateTime FoMaxTime) const
    {
        auto LvSubrange = GetSubRange(FoMinTime, FoMaxTime);
        if (std::ranges::empty(LvSubrange))
        {
            return QJSValue{};
        }
        else
        {
            auto LnMax = std::ranges::max(LvSubrange | std::views::transform(&MarketPoint::GetValue));
            return static_cast<double>(LnMax / 100);
        }
    }

    auto MarketDataModel::GetBefore(Time FoTime) const noexcept -> DataSet::const_iterator
    {
        return std::ranges::lower_bound(MvPoints.begin(), MvPoints.end(), FoTime, std::less<>{}, &MarketPoint::GetTime);
    }

    auto MarketDataModel::GetPartialPoint(Time FoTime) const -> std::optional<PartialMarketPoint>
    {
        auto LoBefore = GetBefore(FoTime);
        if (LoBefore == std::ranges::end(MvPoints))
            return std::nullopt;

        auto LoIt = LoBefore;
        while (LoIt->GetTime() <= FoTime)
        {
            ++LoIt;

            if (LoIt == MvPoints.end())
                return std::nullopt;
        }
        return std::optional<PartialMarketPoint>{ std::in_place, *LoBefore, *LoIt, FoTime };
    }

    QJSValueList MarketDataModel::GetBoundsY(QDateTime FoMinTime, QDateTime FoMaxTime) const
    {
        // TODO: Move this to QML - we need to filter lines that aren't displayed
        Operator::YBounds LoBounds{};
        using clock = Time::clock;
        auto LoMinUtcTime = clock_cast<clock>(FoMinTime.toStdSysMilliseconds());
        auto LoMaxUtcTime = clock_cast<clock>(FoMaxTime.toStdSysMilliseconds());

        for (auto& LoOperator : MvOperators)
        {
            auto LnNumColumns = LoOperator->columnCount({});
            for (int LoColumn = 1; LoColumn <= LnNumColumns; ++LoColumn)
            {
                auto LoOperatorBounds = LoOperator->GetYBounds(LoColumn - 1, LoMinUtcTime, LoMaxUtcTime);
                if (LoOperatorBounds.has_value())
                {
                    if (!LoBounds.has_value())
                    {
                        LoBounds = *LoOperatorBounds;
                    }
                    else
                    {
                        LoBounds->min = (std::min)(LoOperatorBounds->min, LoBounds->min);
                        LoBounds->max = (std::max)(LoOperatorBounds->max, LoBounds->max);
                    }
                }
            }
        }

        if (LoBounds.has_value())
        {
            return QJSValueList{ LoBounds->min, LoBounds->max };
        }
        return QJSValueList{};
    }

    void MarketDataModel::RecalcMinMax() noexcept
    {
        using namespace std::chrono_literals;
        if (std::ranges::empty(MvPoints))
        {
            MoBounds = std::nullopt;
        }
        else
        {
            auto [LoMinX, LoMaxX] = std::ranges::minmax_element(MvPoints, std::less<>{}, TupleGet<0>);
            auto [LoMinY, LoMaxY] = std::ranges::minmax_element(MvPoints, std::less<>{}, TupleGet<1>);
            MoBounds = Bounds{ .MoMinX = LoMinX->GetTime(),
                               .MoMaxX = LoMaxX->GetTime(),
                               .MnMinY = LoMinY->GetValue(),
                               .MnMaxY = LoMaxY->GetValue() };
        }
    }

    void MarketDataModel::AddPoint(MarketPoint FoPoint)
    {
        auto LoLowerBound = std::ranges::lower_bound(MvPoints, FoPoint, PointSorter{});
        auto LnRow = static_cast<int>(std::ranges::distance(MvPoints.begin(), LoLowerBound));
        MvPoints.reserve(MvPoints.size() + 1); // Reserve so that adding a point doesn't throw
        beginInsertRows(QModelIndex(), LnRow, LnRow + 1);
        MvPoints.insert(LoLowerBound, FoPoint); // Doesn't throw -- we reserved above
        assert("not yet implemented: update children" && false);
        endInsertRows();
    }

    void MarketDataModel::SetData(DataSet FvPoints)
    {
        auto LoPreviousBounds = MoBounds;
        std::ranges::sort(FvPoints, PointSorter{});
        MvPoints = std::move(FvPoints);
        Reset(*this);
        for (auto& LoOperator : MvOperators)
        {
            LoOperator->Reset(*this);
        }

        RecalcMinMax();
        if (LoPreviousBounds != MoBounds)
        {
            emit boundsChanged();
        }
    }
} // namespace Calculus::inline Models::MarketData
