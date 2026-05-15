//
// Created by miuna on 4/26/2026.
//

#include "Models/MarketPoint.h"

namespace Calculus::inline Models::MarketData {

MarketPoint::MarketPoint(QDateTime time, Value value) noexcept :
  MoTime(clock_cast<std::chrono::utc_clock>(time.toStdSysMilliseconds())),
  MnValue(value)
{
}

auto MarketPoint::time() const -> QDateTime {
  return ToQDateTime(getTime());
}

double MarketPoint::GetPartialValue(MarketPoint before, MarketPoint after, Time time) noexcept {
  using dduration = std::chrono::duration<double, Time::period>;
  return Lerp(
    InvLerp(time, before.getTime(), after.getTime()),
    before.getValue(), after.getValue()
  );
}

PartialMarketPoint::PartialMarketPoint(MarketPoint before, MarketPoint after, Time time) noexcept :
  MarketPoint(time, static_cast<Value>(std::round(GetPartialValue(before, after, time)))),
  MoBefore(before), MoAfter(after) {
}

}
