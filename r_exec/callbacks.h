

#ifndef callbacks_h
#define callbacks_h

#include <unordered_map>
#include "overlay.h"


namespace r_exec {

class r_exec_dll Callbacks {
public:
  typedef bool (*Callback)(std::chrono::microseconds relative_time, bool suspended, const char *msg, uint8 object_count, r_code::Code **objects);
private:
  static std::unordered_map<std::string, Callback> Callbacks_;
public:
  static void Register(std::string &callback_name, Callback callback);
  static Callback Get(std::string &callback_name);
};
}


#endif
