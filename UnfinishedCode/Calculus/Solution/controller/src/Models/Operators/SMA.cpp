//
// Created by miuna on 4/28/2026.
//

#include <ranges>

#include "Models/MarketData.h"
#include "Models/MarketPoint.h"
#include "Models/Operators/SMA.h"

namespace Calculus::inline Models::MarketData::Operators
{
    SMA::SMA(int FnStride, QObject* FoParent, const QString& FsName) :
        Operator(FoParent, FsName, 1),  MnStride(FnStride)
    {
    }

    void SMA::Reset(const MarketDataModel& FvDataSet)
    {
        auto LnStride = GetStride();
        auto LnSlide = (std::abs)(LnStride);
        bool LbForwardSlide = LnStride >= 0;
        auto LvSlide = FvDataSet | std::views::slide(LnSlide);
        auto LvTimestamps = LvSlide |
                            std::views::transform([LbForwardSlide](const auto& LvWindow) {
                                if (LbForwardSlide)
                                {
                                    return LvWindow.back().GetTime();
                                }
                                else
                                {
                                    return LvWindow.front().GetTime();
                                }
                            }) |
                            std::ranges::to<std::vector<Time>>();
        auto LvValues =
            LvSlide |
            std::views::transform([LnSlide](const auto& LvWindow) -> double {
                return std::ranges::fold_left(
                           LvWindow,
                           double{},
                           [](double LnLeft, MarketPoint LoRight) {
                               return LnLeft + static_cast<double>(LoRight.GetValue()) / 100.0;
                           }
                       ) /
                       LnSlide;
            }) |
            std::ranges::to<std::vector<double>>();
        beginResetModel();
        MvTimestamps = std::move(LvTimestamps);
        MvValues = std::move(LvValues);
        SetSize(static_cast<int>(MvTimestamps.size()));
        endResetModel();
    }

    void SMA::Clear()
    {
        if (MvTimestamps.empty() && MvValues.empty())
            return;

        beginResetModel();
        MvTimestamps.clear();
        MvValues.clear();
        SetSize(0);
        endResetModel();
    }

    void SMA::SetStride(int FnStride)
    {
        auto LnPrevStride = GetStride();
        MnStride = FnStride;
        if (LnPrevStride == GetStride())
            return;

        emit strideChanged();
        if (auto* LoModel = GetModel(); LoModel != nullptr)
            Reset(*LoModel);
    }
} // namespace Calculus::inline Models::MarketData::Operators
