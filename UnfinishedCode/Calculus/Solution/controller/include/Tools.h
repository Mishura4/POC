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
inline constexpr auto tuple_get = []<typename T>(T&& tuple) -> decltype(auto) {
  using std::get;
  return get<N>(tuple);
};

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

}

#endif //CALCULUS_TOOLS_H
