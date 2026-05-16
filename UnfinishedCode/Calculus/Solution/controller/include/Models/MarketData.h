#ifndef RADICALWARE_CALCULUS_MARKETDATA_H_
#define RADICALWARE_CALCULUS_MARKETDATA_H_

#include <chrono>
#include <cstdint>
#include <utility>

#include <QDateTime>
#include <QtQuick/QtQuick>

#include "MarketPoint.h"
#include "Operators/Identity.h"
#include "Tools.h"

namespace Calculus::inline Models::MarketData
{
    class MarketDataModel : public Operators::Identity
    {
        Q_OBJECT
        Q_PROPERTY(QVariant minX READ MinX NOTIFY boundsChanged)
        Q_PROPERTY(QVariant maxX READ MaxX NOTIFY boundsChanged)
        Q_PROPERTY(QVariant minY READ MinY NOTIFY boundsChanged)
        Q_PROPERTY(QVariant maxY READ MaxY NOTIFY boundsChanged)

        struct PointSorter
        {
            static constexpr bool operator()(const MarketPoint& FoA, const MarketPoint& FoB) noexcept
            {
                return FoA.GetTime() < FoB.GetTime();
            }
        };

    public:
        using DataSet = std::vector<MarketPoint>;
        using value_type = typename DataSet::value_type;
        using reference = typename DataSet::reference;
        using iterator = typename DataSet::iterator;
        using const_iterator = typename DataSet::const_iterator;
        using X = std::tuple_element_t<0, value_type>;
        using Y = std::tuple_element_t<1, value_type>;
        using QmlX = value_type::QmlTime;
        using QmlY = value_type::QmlValue;

        enum class Role
        {
            Time,
            Value,
            SMA
        };

        MarketDataModel();
        explicit MarketDataModel(QObject* FoParent);
        ~MarketDataModel();

        void AddPoint(MarketPoint FoPoint);
        template <typename Range>
        void SetData(Range&& FvRange)
        {
            SetData(DataSet(std::from_range, std::forward<Range>(FvRange)));
        }
        void SetData(DataSet FvPoints);

        // https://en.cppreference.com/cpp/ranges/range
        auto begin() noexcept -> iterator;
        auto begin() const noexcept -> const_iterator;
        auto end() noexcept -> iterator;
        auto end() const noexcept -> const_iterator;
        auto size() const noexcept -> int;

        QVariant MinX() const noexcept;
        QVariant MaxX() const noexcept;
        QVariant MinY() const noexcept;
        QVariant MaxY() const noexcept;
        QList<Operator*> Operators() const noexcept;

        Q_INVOKABLE QJSValue GetMinY(QDateTime FoMinTime, QDateTime FoMaxTime) const;
        Q_INVOKABLE QJSValue GetMaxY(QDateTime FoMinTime, QDateTime FoMaxTime) const;
        // Q_INVOKABLE QVariant getPartialPoint(QDateTime time) const;
        Q_INVOKABLE QJSValueList GetBoundsY(QDateTime FoMinTime, QDateTime FoMaxTime) const;

        auto GetMinX() const noexcept -> std::optional<X>;
        auto GetMaxX() const noexcept -> std::optional<X>;
        auto GetMinY() const noexcept -> std::optional<Y>;
        auto GetMaxY() const noexcept -> std::optional<Y>;

    signals:
        void boundsChanged();

    private:
        using Subrange = std::ranges::subrange<DataSet::const_iterator>;

        auto GetBefore(Time FoTime) const noexcept -> DataSet::const_iterator;
        auto GetPartialPoint(Time FoTime) const -> std::optional<PartialMarketPoint>;
        auto GetSubRange(QDateTime FoMinTime, QDateTime FoMaxTime) const noexcept -> Subrange;

        void RecalcMinMax() noexcept;
        struct Bounds
        {
            X MoMinX{};
            X MoMaxX{};
            Y MnMinY{};
            Y MnMaxY{};

            constexpr friend auto operator==(Bounds FoLhs, Bounds FoRhs) noexcept -> bool = default;
        };

        DataSet MvPoints;
        std::vector<Operator*> MvOperators;
        std::optional<Bounds> MoBounds;
    };
} // namespace Calculus::inline Models::MarketData

#endif // RADICALWARE_CALCULUS_MARKETDATA_H_
