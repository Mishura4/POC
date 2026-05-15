//
// Created by miuna on 4/11/2026.
//

#ifndef CALCULUS_TOOLS_H
#define CALCULUS_TOOLS_H

#include <functional>

#include <QDateTime>
#include <QVariant>

namespace Calculus
{

    template <typename Fun>
    struct OnScopeExit
    {
        Fun MfFun;

        ~OnScopeExit() { std::invoke(MfFun); }
    };

    template <typename Fun>
    OnScopeExit(Fun FfFun) -> OnScopeExit<Fun>;

    template <size_t N>
    struct TupleGetter
    {
        template <typename T>
        static constexpr decltype(auto) operator()(T&& FxTuple) noexcept
        {
            using std::get;
            return get<N>(std::forward<T>(FxTuple));
        }
    };

    template <size_t N>
    inline constexpr auto TupleGet = TupleGetter<N>{};

    template <typename T, template <typename> typename RetTransform = std::type_identity>
    class Projection
    {
    public:
        template <typename... Args>
            requires(std::constructible_from<T, Args...>)
        constexpr Projection(Args&&... FxArgs) noexcept(std::is_nothrow_constructible_v<T, Args...>) :
            MfProjection(std::forward<Args>(FxArgs)...)
        {
        }

        template <typename Self, typename U>
        constexpr decltype(auto) operator()(this Self&& FxSelf, U&& FxArg)
            noexcept(std::is_nothrow_invocable_v<decltype(std::forward_like<Self>(FxSelf.MfProjection)), U>)
        {
            using ret_type =
                decltype(std::invoke(std::forward_like<Self>(FxSelf.MfProjection), std::forward<U>(FxArg)));
            return static_cast<typename RetTransform<ret_type>::type>(
                std::invoke(std::forward_like<Self>(FxSelf.MfProjection), std::forward<U>(FxArg)));
        }

    private:
        T MfProjection;
    };

    inline constexpr auto Project = []<typename T>(T&& FfProjection)
    { return Projection<std::remove_cvref_t<T>>{std::forward<T>(FfProjection)}; };

    inline constexpr auto ProjectValue = []<typename T>(T&& FfProjection)
    { return Projection<std::remove_cvref_t<T>, std::remove_cvref>{std::forward<T>(FfProjection)}; };

    template <typename R = void>
    auto Lerp(std::floating_point auto FnFactor, auto FxLow, decltype(FxLow) FnHigh)
    {
        if constexpr (std::is_void_v<R>)
        {
            return FnFactor * (FnHigh - FxLow) + FxLow;
        }
        else
        {
            return static_cast<R>(FnFactor * (FnHigh - FxLow) + FxLow);
        }
    }

    template <typename R = double>
    auto InvLerp(auto FxValue, decltype(FxValue) FxLow, decltype(FxValue) FxHigh)
    {
        if constexpr (std::is_void_v<R>)
        {
            return (FxValue - FxLow) / (FxHigh - FxLow);
        }
        else
        {
            return static_cast<R>(FxValue - FxLow) / static_cast<R>(FxHigh - FxLow);
        }
    }

    template <
        typename R = double,
        typename Rep1 = long long,
        typename Ratio1 = std::milli,
        typename Rep2 = long long,
        typename Ratio2 = std::milli,
        typename Rep3 = long long,
        typename Ratio3 = std::milli
    >
    auto InvLerp(
        std::chrono::duration<Rep1, Ratio1> FoValue,
        std::chrono::duration<Rep2, Ratio2> FoLow,
        std::chrono::duration<Rep3, Ratio3> FoHigh) -> R
    {
        if constexpr (std::is_void_v<R>)
        {
            return (FoValue - FoLow) / (FoHigh - FoLow);
        }
        else
        {
            auto LoA = duration_cast<std::chrono::duration<R, Ratio1>>(FoValue);
            auto LoB = duration_cast<std::chrono::duration<R, Ratio2>>(FoLow);
            auto LoC = duration_cast<std::chrono::duration<R, Ratio3>>(FoHigh);
            return (LoA - LoB) / (LoC - LoB);
        }
    }

    template <
        typename R = double,
        typename Clock = std::chrono::system_clock,
        typename Dur1 = Clock::duration,
        typename Dur2 = Clock::duration,
        typename Dur3 = Clock::duration
    >
    auto InvLerp(
        std::chrono::time_point<Clock, Dur1> FoValue,
        std::chrono::time_point<Clock, Dur2> FoLow,
        std::chrono::time_point<Clock, Dur3> FoHigh) -> R
    {
        return InvLerp(FoValue.time_since_epoch(), FoLow.time_since_epoch(), FoHigh.time_since_epoch());
    }

    template <typename T>
    QVariant ToQVariant(std::optional<T> FxValue)
    {
        if (FxValue.has_value())
            return *FxValue;
        else
            return QVariant{};
    }

    template <typename Duration, typename Clock>
    QDateTime ToQDateTime(std::chrono::time_point<Duration, Clock> FoTime)
    {
        return QDateTime::fromStdTimePoint(clock_cast<std::chrono::system_clock>(FoTime));
    }

    template <typename Bound, typename Range, typename Compare = std::less<>, typename Proj = std::identity>
    auto GetRangeWindow(
        Range&& FxRange,
        const std::optional<Bound>& FxMin,
        const std::optional<Bound>& FxMax,
        Compare FfComp = {},
        Proj FfProj = {}) noexcept -> std::ranges::borrowed_subrange_t<Range>
    {
        auto LoBegin =
            FxMin.has_value() ? std::ranges::lower_bound(FxRange, *FxMin, FfComp, FfProj) : std::ranges::begin(FxRange);
        auto LoEnd =
            FxMax.has_value() ? std::ranges::lower_bound(FxRange, *FxMax, FfComp, FfProj) : std::ranges::end(FxRange);
        return std::ranges::borrowed_subrange_t<Range>{LoBegin, LoEnd};
    }

    template <typename Bound, typename Range, typename Compare = std::less<>, typename Proj = std::identity>
    auto GetRangeWindow(
        Range&& FxRange,
        const Bound& FxMin,
        const std::optional<Bound>& FxMax,
        Compare FfComp = {},
        Proj FfProj = {}) noexcept -> std::ranges::borrowed_subrange_t<Range>
    {
        auto LoBegin = std::ranges::lower_bound(FxRange, *FxMin, FfComp, FfProj);
        auto LoEnd =
            FxMax.has_value() ? std::ranges::lower_bound(FxRange, *FxMax, FfComp, FfProj) : std::ranges::end(FxRange);
        return std::ranges::borrowed_subrange_t<Range>{LoBegin, LoEnd};
    }

    template <typename Bound, typename Range, typename Compare = std::less<>, typename Proj = std::identity>
    auto GetRangeWindow(
        Range&& FxRange,
        const std::optional<Bound>& FxMin,
        const Bound& FxMax,
        Compare FfComp = {},
        Proj FfProj = {}) noexcept -> std::ranges::borrowed_subrange_t<Range>
    {
        auto LoBegin =
            FxMin.has_value() ? std::ranges::lower_bound(FxRange, *FxMin, FfComp, FfProj) : std::ranges::begin(FxRange);
        auto LoEnd = std::ranges::lower_bound(FxRange, FxMax, FfComp, FfProj);
        return std::ranges::borrowed_subrange_t<Range>{LoBegin, LoEnd};
    }

    template <typename Bound, typename Range, typename Compare = std::less<>, typename Proj = std::identity>
    auto GetRangeWindow(
        Range&& FxRange, const Bound& FxMin, const Bound& FxMax, Compare FfComp = {}, Proj FfProj = {}) noexcept
        -> std::ranges::borrowed_subrange_t<Range>
    {
        auto LoBegin = std::ranges::lower_bound(FxRange, FxMin, FfComp, FfProj);
        auto LoEnd = std::ranges::lower_bound(FxRange, FxMax, FfComp, FfProj);
        return std::ranges::borrowed_subrange_t<Range>{LoBegin, LoEnd};
    }

} // namespace Calculus

#endif // CALCULUS_TOOLS_H
