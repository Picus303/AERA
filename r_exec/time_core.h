

#ifndef time_core_h
#define time_core_h

#include "../core/utils.h"
#include "time_job.h"


using namespace core;

namespace r_exec {

class DelegatedCore :
  public Thread {
private:
  Timer timer_;
  Timestamp target_time_;
  std::chrono::microseconds time_to_wait_;
  P<TimeJob> job_;
public:
  static thread_ret thread_function_call Wait(void *args);

  DelegatedCore(Timestamp target_time, std::chrono::microseconds time_to_wait, TimeJob *j);
  ~DelegatedCore();
};

class r_exec_dll TimeCore :
  public Thread {
public:
  static thread_ret thread_function_call Run(void *args);

  TimeCore();
  ~TimeCore();
};
}


#endif
