

#include "callbacks.h"

using namespace std;

namespace r_exec {

unordered_map<std::string, Callbacks::Callback> Callbacks::Callbacks_;

void Callbacks::Clear() {

  Callbacks_.clear();
}

void Callbacks::Register(const std::string& callback_name, Callback callback) {

  Callbacks_[callback_name] = callback;
}

Callbacks::Callback Callbacks::Get(const std::string& callback_name) {

  unordered_map<std::string, Callback>::const_iterator it = Callbacks_.find(callback_name);
  if (it != Callbacks_.end())
    return it->second;
  return NULL;
}
}
