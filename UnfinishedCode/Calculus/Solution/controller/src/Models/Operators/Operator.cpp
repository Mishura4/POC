//
// Created by miuna on 4/28/2026.
//

#include <chrono>

#include "Models/Operators/Operator.h"

#include "MarketData.h"
#include "Tools.h"

namespace Calculus::inline Models::MarketData
{
    // auto Operator::getYBounds(int row) const noexcept ->
    // std::optional<std::ranges::minmax_result<double>> {  }

    Operator::Operator(
        QObject* FoParent, const QString& FsName, int FnColumnCount
    ) : QAbstractTableModel(FoParent),
        MnColumnCount(FnColumnCount)
    {
        setObjectName(FsName);
    }

    void Operator::SetModel(MarketDataModel* FoDataModel)
    {
        if (FoDataModel == MoDataModel)
            return;

        if (MoDataModel != nullptr)
        {
            QObject::disconnect(MoDataModel, &QAbstractItemModel::dataChanged, this, &Operator::onDataChanged);
            QObject::disconnect(MoDataModel, &QAbstractItemModel::modelReset, this, static_cast<void (Operator::*)()>(&Operator::Reset));
        }
        MoDataModel = FoDataModel;
        if (MoDataModel == nullptr)
        {
            Clear();
        }
        else
        {
            QObject::connect(MoDataModel, &QAbstractItemModel::dataChanged, this, &Operator::onDataChanged);
            QObject::connect(MoDataModel, &QAbstractItemModel::modelReset, this, static_cast<void (Operator::*)()>(&Operator::Reset));
            Reset(*MoDataModel);
        }
        emit modelChanged();
    }

    QVariant Operator::data(const QModelIndex& FoIndex, int FnRole) const
    {
        auto LnRowIndex = FoIndex.row();
        if (LnRowIndex < 0 || LnRowIndex >= rowCount(FoIndex.parent()))
        {
            return QVariant();
        }

        auto LnColumnIndex = FoIndex.column();
        if (LnColumnIndex < 0 || LnColumnIndex >= columnCount(FoIndex.parent()))
        {
            return QVariant();
        }

        if (LnColumnIndex == 0)
        {
            return ToQDateTime(GetX()[LnRowIndex]);
        }

        return GetY(LnColumnIndex - 1)[LnRowIndex];
    }

    void Operator::onDataChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles
    )
    {
        Reset(*MoDataModel);
    }

    void Operator::Reset()
    {
        if (MoDataModel)
            Reset(*MoDataModel);
        else
            Clear();
    }

    auto Operator::GetYBounds(
        int FnColumn, std::optional<Time> FoMinX, std::optional<Time> FoMaxX
    ) const noexcept -> YBounds
    {
        if (FnColumn < 0 || FnColumn >= columnCount({}) - 1)
        {
            return std::nullopt;
        }

        // Zip time & value column, binary search the window
        auto LvZipped = std::views::zip(GetX(), GetY(FnColumn));
        constexpr auto LfGetTime = TupleGet<0>;
        constexpr auto LfGetValue = TupleGet<1>;
        auto [LoBegin, LoEnd] = GetRangeWindow(LvZipped, FoMinX, FoMaxX, std::less{}, LfGetTime);
        auto [LoIteratorMin, LoIteratorMax]
            = std::ranges::minmax_element(LoBegin, LoEnd, std::less<>{}, LfGetValue);

        auto LnMinValue = LoIteratorMin == LoEnd ? std::optional<Y>{} : LfGetValue(*LoIteratorMin);
        auto LnMaxValue = LoIteratorMax == LoEnd ? std::optional<Y>{} : LfGetValue(*LoIteratorMax);
        if (FoMinX.has_value() && LoBegin != LvZipped.begin())
        {
            auto LoIteratorBefore = std::ranges::prev(LoBegin);
            auto LnInterpolateFactor
                = InvLerp(*FoMinX, LfGetTime(*LoIteratorBefore), LfGetTime(*LoBegin));
            auto LnInterpolatedValue
                = Lerp(LnInterpolateFactor, LfGetValue(*LoIteratorBefore), LfGetValue(*LoBegin));
            LnMaxValue = LnMaxValue.has_value() ? (std::max)(LnInterpolatedValue, *LnMaxValue)
                                                : LnInterpolatedValue;
            LnMinValue = LnMinValue.has_value() ? (std::min)(LnInterpolatedValue, *LnMinValue)
                                                : LnInterpolatedValue;
        }

        if (FoMaxX.has_value() && LoEnd != LvZipped.end())
        {
            auto LoIteratorAfter = LoEnd;
            auto LoIteratorLast = std::ranges::prev(LoIteratorAfter);
            auto LnInterpolateFactor
                = InvLerp(*FoMaxX, LfGetTime(*LoIteratorLast), LfGetTime(*LoIteratorAfter));
            auto LnInterpolatedValue = Lerp(
                LnInterpolateFactor, LfGetValue(*LoIteratorLast), LfGetValue(*LoIteratorAfter)
            );
            LnMaxValue = LnMaxValue.has_value() ? (std::max)(LnInterpolatedValue, *LnMaxValue)
                                                : LnInterpolatedValue;
            LnMinValue = LnMinValue.has_value() ? (std::min)(LnInterpolatedValue, *LnMinValue)
                                                : LnInterpolatedValue;
        }

        if (!LnMinValue.has_value() || !LnMaxValue.has_value())
        {
            assert(LvZipped.empty()); // right?
            return std::nullopt;
        }
        return YBounds::value_type{ .min = *LnMinValue, .max = *LnMaxValue };
    }

    auto Operator::GetYBounds(int FnColumn, QDateTime FoMinX, QDateTime FoMaxX) const noexcept
        -> QJSValueList
    {
        auto LoMinUtcTime = clock_cast<Time::clock>(FoMinX.toStdSysMilliseconds());
        auto LoMaxUtcTime = clock_cast<Time::clock>(FoMaxX.toStdSysMilliseconds());
        auto LoBounds = GetYBounds(FnColumn, LoMinUtcTime, LoMaxUtcTime);
        if (LoBounds.has_value())
        {
            return QJSValueList{ LoBounds->min, LoBounds->max };
        }
        return QJSValueList();
    }

    QVariant Operator::PointClosestTo(QDateTime FoTime) const noexcept
    {
        auto LoUtcTime = clock_cast<Time::clock>(FoTime.toStdSysMilliseconds());
        auto LvAllTimes = GetX();
        auto LoIteratorHigh = std::ranges::lower_bound(LvAllTimes, LoUtcTime, std::less{});
        if (LoIteratorHigh == LvAllTimes.end())
        {
            return LvAllTimes.empty() ? QVariant{}
                                      : QVariant::fromValue(*std::prev(LoIteratorHigh));
        }

        auto LoIteratorLow = LoIteratorHigh;
        while (*LoIteratorLow == *LoIteratorHigh && LoIteratorLow != LvAllTimes.begin())
        {
            LoIteratorLow = std::prev(LoIteratorLow);
        }
        auto LoIteratorClosest = LoIteratorHigh;
        if (*LoIteratorHigh - LoUtcTime > abs(LoUtcTime - *LoIteratorLow))
        {
            LoIteratorClosest = LoIteratorLow;
        }
        return At(static_cast<int>(std::distance(LvAllTimes.begin(), LoIteratorClosest)));
    }

    QVariant Operator::operator[](int FnIndex) const
    {
        return QVariant::fromValue(MarketPoint(GetX()[FnIndex], GetY(0)[FnIndex]));
    }

    QVariant Operator::At(int FnIndex) const
    {
        if (FnIndex < 0 || FnIndex >= rowCount({}))
        {
            throw std::out_of_range("Index out of range");
        }
        return (*this)[FnIndex];
    }
} // namespace Calculus::inline Models::MarketData
