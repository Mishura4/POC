//
// Created by miuna on 4/11/2026.
//

#ifndef CALCULUS_TOOLS_H
#define CALCULUS_TOOLS_H

#include <functional>

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

}

#endif //CALCULUS_TOOLS_H
