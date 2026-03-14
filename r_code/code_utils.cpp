

#include "../submodules/CoreLibrary/CoreLibrary/utils.h"
#include "object.h"

#include <math.h>

using namespace std;
using namespace std::chrono;

namespace r_code {

Timestamp Utils::TimeReference = Timestamp(microseconds(0));
microseconds Utils::BasePeriod = microseconds(0);
float32 Utils::FloatTolerance = 0;
microseconds Utils::TimeTolerance = microseconds(0);

Timestamp Utils::GetTimeReference() { return TimeReference; }
microseconds Utils::GetBasePeriod() { return BasePeriod; }
uint32 Utils::GetFloatTolerance() { return FloatTolerance; }
microseconds Utils::GetTimeTolerance() { return TimeTolerance; }

void Utils::SetReferenceValues(microseconds base_period, float32 float_tolerance, microseconds time_tolerance) {

  BasePeriod = base_period;
  FloatTolerance = float_tolerance;
  TimeTolerance = time_tolerance;
}

void Utils::SetTimeReference(Timestamp time_reference) {

  TimeReference = time_reference;
}

bool Utils::Equal(float32 l, float32 r) {

  if (l == r)
    return true;
  return fabs(l - r) < FloatTolerance;
}

bool Utils::Synchronous(Timestamp l, Timestamp r) {

  return abs(l - r) < TimeTolerance;
}

Timestamp Utils::GetTimestamp(const Atom *iptr) {

  return Timestamp(microseconds(GetInt64(iptr, 1)));
}

void Utils::SetTimestamp(Atom *iptr, Timestamp timestamp) {

  iptr[0] = Atom::Timestamp();
  SetInt64(iptr, 1, duration_cast<microseconds>(timestamp.time_since_epoch()).count());
}

void Utils::SetTimestampStruct(Code *object, uint16 index, Timestamp timestamp) {

  object->code(index) = Atom::Timestamp();
  // This will resize the code array if needed.
  object->code(index + 2) = 0;
  SetInt64(&object->code(0), index + 1, duration_cast<microseconds>(timestamp.time_since_epoch()).count());
}

string Utils::ToString_s_ms_us(Timestamp timestamp, Timestamp time_reference) {
  auto duration = timestamp - time_reference;
  uint64 t = abs(duration_cast<microseconds>(duration).count());

  uint64 us = t % 1000;
  uint64 ms = t / 1000;
  uint64 s = ms / 1000;
  ms = ms % 1000;

  std::string result = (duration < microseconds(0) ? "-" : "");
  result += std::to_string(s);
  result += "s:";
  result += std::to_string(ms);
  result += "ms:";
  result += std::to_string(us);
  result += "us";

  return result;
}

void Utils::SetDurationStruct(Code *object, uint16 index, microseconds duration) {
  object->resize_code(index + 3);
  object->code(index) = Atom::Duration();
  SetInt64(&object->code(0), index + 1, duration.count());
}

string Utils::ToString_us(microseconds duration) {
  uint64 us = abs(duration_cast<microseconds>(duration).count());

  std::string sign = (duration < microseconds(0) ? "-" : "");
  if (us % 1000 != 0)
    return sign + std::to_string(us) + "us";
  else {
    uint64 ms = us / 1000;
    if (ms % 1000 != 0)
      return sign + std::to_string(ms) + "ms";
    else {
      uint64 s = ms / 1000;
      return sign + std::to_string(s) + "s";
    }
  }
}

std::string Utils::GetString(const Atom *iptr) {

  std::string s;
  char buffer[255];
  uint8 char_count = (iptr[0].atom_ & 0x000000FF);
  memcpy(buffer, iptr + 1, char_count);
  buffer[char_count] = 0;
  s += buffer;
  return s;
}

void Utils::SetString(Atom *iptr, const std::string &s) {

  uint8 l = (uint8)s.length();
  uint8 index = 0;
  iptr[index] = Atom::String(l);
  uint32 st = 0;
  int8 shift = 0;
  for (uint8 i = 0; i < l; ++i) {

    st |= s[i] << shift;
    shift += 8;
    if (shift == 32) {

      iptr[++index] = st;
      st = 0;
      shift = 0;
    }
  }
  if (l % 4)
    iptr[++index] = st;
}

int32 Utils::GetResilience(Timestamp now, microseconds time_to_live, uint64 upr) {

  if (time_to_live.count() == 0 || upr == 0)
    return 1;
  auto deadline = now + time_to_live;
  uint64 last_upr = duration_cast<microseconds>(now - TimeReference).count() / upr;
  uint64 next_upr = duration_cast<microseconds>(deadline - TimeReference).count() / upr;
  if (duration_cast<microseconds>(deadline - TimeReference).count() % upr > 0)
    ++next_upr;
  return next_upr - last_upr;
}

int32 Utils::GetResilience(float32 resilience, float32 origin_upr, float32 destination_upr) {

  if (origin_upr == 0)
    return 1;
  if (destination_upr <= origin_upr)
    return 1;
  float32 r = origin_upr / destination_upr;
  float32 res = resilience * r;
  if (res < 1)
    return 1;
  return res;
}

std::string Utils::RelativeTime(Timestamp t) {

  return ToString_s_ms_us(t, TimeReference);
}

bool Utils::has_reference(const Atom* code, uint16 index) {
  Atom atom = code[index];

  switch (atom.getDescriptor()) {
  case Atom::R_PTR:
    return true;
  case Atom::I_PTR:
    return has_reference(code, atom.asIndex());
  case Atom::C_PTR:
  case Atom::SET:
  case Atom::OBJECT:
  case Atom::S_SET:
  case Atom::MARKER:
  case Atom::OPERATOR:
  case Atom::TIMESTAMP:
  case Atom::DURATION:
  case Atom::GROUP: {
    uint16 count = atom.getAtomCount();
    for (uint16 i = 1; i <= count; ++i) {
      if (has_reference(code, index + i))
        return true;
    }
    return false;
  }
  default:
    return false;
  }
}

}
