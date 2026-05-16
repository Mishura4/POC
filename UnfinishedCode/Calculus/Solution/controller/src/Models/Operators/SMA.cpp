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
        Operator(FoParent, FsName, 1, (std::max)(0, FnStride - 1)), MnStride(FnStride)
    {
    }

    void SMA::Reset(const MarketDataModel& FvDataSet)
    {
        auto LnStride = (std::abs)(MnStride);
        bool LbForwardStride = MnStride >= 0;
        auto LvSlide = FvDataSet | std::views::slide(LnStride);
        auto LvTimestamps = LvSlide |
                            std::views::transform([LbForwardStride](const auto& LvWindow) {
                                if (LbForwardStride)
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
            std::views::transform([LnStride](const auto& LvWindow) -> double {
                return std::ranges::fold_left(
                           LvWindow,
                           double{},
                           [](double LnLeft, MarketPoint LoRight) {
                               return LnLeft + static_cast<double>(LoRight.GetValue()) / 100.0;
                           }
                       ) /
                       LnStride;
            }) |
            std::ranges::to<std::vector<double>>();
        beginResetModel();
        MvTimestamps = std::move(LvTimestamps);
        MvValues = std::move(LvValues);
        SetSize(static_cast<int>(MvTimestamps.size()));
        endResetModel();
    }
} // namespace Calculus::inline Models::MarketData::Operators
