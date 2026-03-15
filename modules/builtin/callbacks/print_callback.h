
#ifndef builtin_print_callback_h
#define builtin_print_callback_h

#include <chrono>

#include "types.h"

namespace r_code {
class Code;
}

namespace aera::builtin {

bool print(std::chrono::microseconds relative_time,
  bool suspended,
  const char* msg,
  core::uint8 object_count,
  r_code::Code** objects);

}  // namespace aera::builtin

#endif
