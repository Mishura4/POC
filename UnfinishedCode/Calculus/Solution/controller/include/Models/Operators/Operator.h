//
// Created by miuna on 4/28/2026.
//

#ifndef CALCULUS_OPERATOR_H
#define CALCULUS_OPERATOR_H

#include <optional>
#include <span>
#include <tuple>
#include <vector>

#include "MarketPoint.h"

namespace Calculus::inline Models::MarketData
{
    class MarketDataModel;

    class Operator : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        Q_PROPERTY(MarketDataModel* model READ GetModel WRITE SetModel NOTIFY modelChanged)

        using X = Time;
        using Y = double;
        using Point = std::tuple<X, Y>;
        using XColumn = std::span<const X>;
        using YColumn = std::span<const Y>;
        using XSubrange = std::ranges::subrange<XColumn::const_iterator>;
        using YBounds = std::optional<std::ranges::minmax_result<Y>>;

        void SetModel(MarketDataModel* FoDataModel);
        auto GetModel() const noexcept -> MarketDataModel* { return MoDataModel; }

        virtual void Reset(const MarketDataModel& FvDataSet) = 0;
        virtual void Clear() = 0;
        virtual auto GetX() const noexcept -> XColumn = 0;
        virtual auto GetY(int FnColumn) const noexcept -> YColumn = 0;
        virtual auto GetYBounds(int FnColumn, std::optional<Time> FoMinX, std::optional<Time> FoMaxX) const noexcept
            -> YBounds;
        Q_INVOKABLE QJSValueList GetYBounds(int FnColumn, QDateTime FoMinX, QDateTime FoMaxX) const noexcept;
        Q_INVOKABLE virtual QVariant PointClosestTo(QDateTime FoTime) const noexcept;
        Q_INVOKABLE virtual QVariant operator[](int FnIndex) const;
        Q_INVOKABLE QVariant At(int FnIndex) const;

        static auto GetTimeRange(const XColumn& FvValues, Time FoMin, Time FoMax) noexcept -> XSubrange;

        // Qt overrides
        int rowCount(const QModelIndex& FoParent) const override { return MnSize; }
        int columnCount(const QModelIndex& FoParent) const override { return MnColumnCount + 1; }
        QVariant data(const QModelIndex& FoIndex, int FnRole) const override;

    protected slots:
        virtual void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>());
        void Reset();

    signals:
        void modelChanged();

    protected:
        explicit Operator(QObject* FoParent, const QString& FsName, int FnColumnCount);

        constexpr void SetSize(int FnNewSize) noexcept { MnSize = FnNewSize; }

    private:
        MarketDataModel* MoDataModel = nullptr;
        int MnColumnCount = 0;
        int MnSize = 0;
    };
} // namespace Calculus::inline Models::MarketData

#endif // CALCULUS_OPERATOR_H
