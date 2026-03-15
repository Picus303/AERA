
#include <sstream>
#include "print_callback.h"

#include "runtime/mem.h"

using namespace std;
using namespace std::chrono;
using namespace r_code;

namespace aera::builtin {

bool print(microseconds relative_time, bool suspended, const char* msg, core::uint8 object_count, Code** objects) { // return true to resume the executive (applies when called from a suspend call, i.e. suspended==true).

  ostringstream out;
  out << Utils::ToString_s_ms_us(Timestamp(relative_time), Timestamp(seconds(0))) << ": " << msg << std::endl;
  for (uint8 i = 0; i < object_count; ++i)
    objects[i]->trace(out);

  // Assume that printing a single string is more-or-less atomic.
  std::cout << out.str();
  return true;
}

}
