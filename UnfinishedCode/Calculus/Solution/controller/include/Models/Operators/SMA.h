//
// Created by miuna on 4/28/2026.
//

#ifndef CALCULUS_SIMPLEMOVINGAVERAGE_H
#define CALCULUS_SIMPLEMOVINGAVERAGE_H

#include <span>
#include <vector>

#include "Operator.h"

namespace Calculus::inline Models::MarketData::Operators
{
    class SMA : public Operator
    {
        Q_OBJECT
        QML_ELEMENT

    public:
        Q_PROPERTY(int stride READ GetStride WRITE SetStride NOTIFY strideChanged)

        SMA(int FnStride, QObject* FoParent, const QString& FsName);
        explicit SMA(int FnStride = 14, QObject* FoParent = nullptr) : SMA(FnStride, FoParent, tr("SMA %1").arg(FnStride)) {}

        void Reset(const MarketDataModel& FvDataSet) override;
        void Clear() override;
        auto GetX() const noexcept -> std::span<const Time> override { return MvTimestamps; }
        auto GetY(int FnRow) const noexcept -> std::span<const double> override { return MvValues; }

        int GetStride() const noexcept { return (std::max)(1, MnStride); }
        void SetStride(int FnStride);

    signals:
        void strideChanged();

    private:
        int MnStride = 1;
        std::vector<Time> MvTimestamps;
        std::vector<double> MvValues;
    };
} // namespace Calculus::inline Models::MarketData::Operators

#endif // CALCULUS_SIMPLEMOVINGAVERAGE_H
