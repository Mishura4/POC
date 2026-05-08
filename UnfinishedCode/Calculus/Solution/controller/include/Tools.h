//
// Created by miuna on 4/11/2026.
//

#ifndef CALCULUS_TOOLS_H
#define CALCULUS_TOOLS_H

#include <functional>

#include <QVariant>
#include <QDateTime>

namespace Calculus {

template <typename Fun>
struct onScopeExit {
  Fun MoFun;

  ~onScopeExit() {
    std::invoke(MoFun);
  }
};

template <typename Fun>
onScopeExit(Fun fun) -> onScopeExit<Fun>;

template <size_t N>
struct tuple_get_t {
  template <typename T>
  static constexpr decltype(auto) operator()(T&& tuple) noexcept {
    using std::get;
    return get<N>(std::forward<T>(tuple));
  }
};

template <size_t N>
inline constexpr auto tuple_get = tuple_get_t<N>{};

template <typename T, template <typename> typename RetTransform = std::type_identity>
class projection {
public:
  template <typename... Args>
  requires(std::constructible_from<T, Args...>)
  constexpr projection(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) :
    proj(std::forward<Args>(args)...) {
  }

  template <typename Self, typename U>
  constexpr decltype(auto) operator()(this Self&& self, U&& u)
  noexcept(std::is_nothrow_invocable_v<decltype(std::forward_like<Self>(self.proj)), U>) {
    using ret_type = decltype(std::invoke(std::forward_like<Self>(self.proj), std::forward<U>(u)));
    return static_cast<typename RetTransform<ret_type>::type>(
      std::invoke(std::forward_like<Self>(self.proj),std::forward<U>(u))
    );
  }

private:
  T proj;
};

inline constexpr auto project = []<typename T>(T&& proj) {
  return projection<std::remove_cvref_t<T>>{ std::forward<T>(proj) };
};

inline constexpr auto project_value = []<typename T>(T&& proj) {
  return projection<std::remove_cvref_t<T>, std::remove_cvref>{ std::forward<T>(proj) };
};

template <typename R = void>
auto lerp(std::floating_point auto factor, auto low, decltype(low) high) {
  if constexpr (std::is_void_v<R>) {
    return factor * (high - low) + low;
  } else {
    return static_cast<R>(factor * (high - low) + low);
  }
}

template <typename R = double>
auto invlerp(auto value, decltype(value) low, decltype(value) high) {
  if constexpr (std::is_void_v<R>) {
    return (value - low) / (high - low);
  } else {
    return static_cast<R>(value - low) / static_cast<R>(high - low);
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
auto invlerp(
  std::chrono::duration<Rep1, Ratio1> value,
  std::chrono::duration<Rep2, Ratio2> low,
  std::chrono::duration<Rep3, Ratio3> high
) -> R {
  if constexpr (std::is_void_v<R>) {
    return (value - low) / (high - low);
  } else {
    auto a = duration_cast<std::chrono::duration<R, Ratio1>>(value);
    auto b = duration_cast<std::chrono::duration<R, Ratio2>>(low);
    auto c = duration_cast<std::chrono::duration<R, Ratio3>>(high);
    return (a - b) / (c - b);
  }
}

template <
  typename R = double,
  typename Clock = std::chrono::system_clock,
  typename Dur1 = Clock::duration,
  typename Dur2 = Clock::duration,
  typename Dur3 = Clock::duration
>
auto invlerp(
  std::chrono::time_point<Clock, Dur1> value,
  std::chrono::time_point<Clock, Dur2> low,
  std::chrono::time_point<Clock, Dur3> high
) -> R {
  return invlerp(value.time_since_epoch(), low.time_since_epoch(), high.time_since_epoch());
}

template <typename T>
QVariant toQVariant(std::optional<T> value) {
  if (value.has_value())
    return *value;
  else
    return QVariant{};
}

template <typename Duration, typename Clock>
QDateTime toQDateTime(std::chrono::time_point<Duration, Clock> time) {
  return QDateTime::fromStdTimePoint(clock_cast<std::chrono::system_clock>(time));
}

template <typename Bound, typename Range, typename Compare = std::less<>, typename Proj = std::identity>
auto getRangeWindow(Range &&range, const std::optional<Bound>& min, const std::optional<Bound>& max, Compare comp = {}, Proj proj = {}) noexcept ->
  std::ranges::borrowed_subrange_t<Range>
{
  auto begin = min.has_value() ? std::ranges::lower_bound(range, *min, comp, proj) : std::ranges::begin(range);
  auto end = max.has_value() ? std::ranges::lower_bound(range, *max, comp, proj) : std::ranges::end(range);
  return std::ranges::borrowed_subrange_t<Range>{ begin, end };
}

template <typename Bound, typename Range, typename Compare = std::less<>, typename Proj = std::identity>
auto getRangeWindow(Range &&range, const Bound& min, const std::optional<Bound>& max, Compare comp = {}, Proj proj = {}) noexcept ->
  std::ranges::borrowed_subrange_t<Range>
{
  auto begin = std::ranges::lower_bound(range, *min, comp, proj);
  auto end = max.has_value() ? std::ranges::lower_bound(range, *max, comp, proj) : std::ranges::end(range);
  return std::ranges::borrowed_subrange_t<Range>{ begin, end };
}

template <typename Bound, typename Range, typename Compare = std::less<>, typename Proj = std::identity>
auto getRangeWindow(Range &&range, const std::optional<Bound>& min, const Bound& max, Compare comp = {}, Proj proj = {}) noexcept ->
  std::ranges::borrowed_subrange_t<Range>
{
  auto begin = min.has_value() ? std::ranges::lower_bound(range, *min, comp, proj) : std::ranges::begin(range);
  auto end = std::ranges::lower_bound(range, max, comp, proj);
  return std::ranges::borrowed_subrange_t<Range>{ begin, end };
}

template <typename Bound, typename Range, typename Compare = std::less<>, typename Proj = std::identity>
auto getRangeWindow(Range &&range, const Bound& min, const Bound& max, Compare comp = {}, Proj proj = {}) noexcept ->
  std::ranges::borrowed_subrange_t<Range>
{
  auto begin = std::ranges::lower_bound(range, min, comp, proj);
  auto end = std::ranges::lower_bound(range, max, comp, proj);
  return std::ranges::borrowed_subrange_t<Range>{ begin, end };
}

}

#endif //CALCULUS_TOOLS_H
