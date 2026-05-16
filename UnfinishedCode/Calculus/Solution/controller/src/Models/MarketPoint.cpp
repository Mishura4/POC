//
// Created by miuna on 4/26/2026.
//

#include "Models/MarketPoint.h"

namespace Calculus::inline Models::MarketData {

MarketPoint::MarketPoint(QDateTime FoTime, Value FnValue) noexcept :
  MoTime(clock_cast<std::chrono::utc_clock>(FoTime.toStdSysMilliseconds())),
  MnValue(FnValue)
{
}

auto MarketPoint::time() const -> QDateTime {
  return ToQDateTime(GetTime());
}

double MarketPoint::GetPartialValue(MarketPoint FoBefore, MarketPoint FoAfter, Time FoTime) noexcept {
  using dduration = std::chrono::duration<double, Time::period>;
  return Lerp(
    InvLerp(FoTime, FoBefore.GetTime(), FoAfter.GetTime()),
    FoBefore.GetValue(), FoAfter.GetValue()
  );
}

PartialMarketPoint::PartialMarketPoint(MarketPoint FoBefore, MarketPoint FoAfter, Time FoTime) noexcept :
  MarketPoint(FoTime, static_cast<Value>(std::round(GetPartialValue(FoBefore, FoAfter, FoTime)))),
  MoBefore(FoBefore), MoAfter(FoAfter) {
}

}
