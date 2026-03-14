

#ifndef callbacks_h
#define callbacks_h

#include <chrono>
#include <string>
#include <unordered_map>

#include "dll.h"
#include "types.h"

namespace r_code {
class Code;
}


namespace r_exec {

class r_exec_dll Callbacks {
public:
  typedef bool (*Callback)(std::chrono::microseconds relative_time, bool suspended, const char* msg, core::uint8 object_count, r_code::Code** objects);
private:
  static std::unordered_map<std::string, Callback> Callbacks_;
public:
  static void Register(const std::string& callback_name, Callback callback);
  static Callback Get(const std::string& callback_name);
};
}


#endif
