//
// Created by miuna on 4/28/2026.
//

#ifndef CALCULUS_OPERATORS_IDENTITY_H
#define CALCULUS_OPERATORS_IDENTITY_H

#include <span>
#include <vector>

#include "Operators/Operator.h"

namespace Calculus::inline Models::MarketData::Operators
{
    class Identity : public Operator
    {
        Q_OBJECT
        QML_ELEMENT

    public:
        Identity(QObject* FoParent = nullptr, const QString& FsName = tr("Identity"));

        void Reset(const MarketDataModel& FvDataSet) override;
        void Clear() override;
        auto GetX() const noexcept -> std::span<const Time> override { return MvTimestamps; }
        auto GetY(int FnRow) const noexcept -> std::span<const double> override { return MvValues; }

    private:
        // TODO: We don't necessarily need to copy here maybe
        std::vector<Time> MvTimestamps;
        std::vector<double> MvValues;
    };
} // namespace Calculus::inline Models::MarketData::Operators

#endif // CALCULUS_OPERATORS_IDENTITY_H
