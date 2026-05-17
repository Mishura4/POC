//
// Created by miuna on 4/28/2026.
//

#include <ranges>

#include "Models/MarketData.h"
#include "Models/Operators/Identity.h"

namespace Calculus::inline Models::MarketData::Operators
{
    Identity::Identity(QObject* FoParent, const QString& FsName) : Operator(FoParent, FsName, 1) {}

    void Identity::Reset(const MarketDataModel& FvDataSet)
    {
        auto LvTimestamps = std::vector(
            std::from_range, FvDataSet | std::views::transform(&MarketPoint::GetTime)
        );
        auto LvValues = std::vector(
            std::from_range, FvDataSet | std::views::transform([](const MarketPoint& point) {
                                 return static_cast<double>(point.GetValue()) / 100.0;
                             })
        );

        beginResetModel();
        MvTimestamps = std::move(LvTimestamps);
        MvValues = std::move(LvValues);
        SetSize(static_cast<int>(std::ranges::size(MvTimestamps)));
        endResetModel();
    }

    void Identity::Clear()
    {
        if (MvTimestamps.empty() && MvValues.empty())
            return;

        beginResetModel();
        MvTimestamps.clear();
        MvValues.clear();
        SetSize(0);
        endResetModel();
    }
} // namespace Calculus::inline Models::MarketData::Operators
