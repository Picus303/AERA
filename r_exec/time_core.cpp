

#include "time_core.h"
#include "mem.h"
#include "init.h"

using namespace std::chrono;

namespace r_exec {

thread_ret thread_function_call TimeCore::Run(void *args) {

  TimeCore *_this = ((TimeCore *)args);

  bool run = true;
  while (run) {

    P<TimeJob> j = _Mem::Get()->pop_time_job();
    if (j == NULL)
      break;
    if (!j->is_alive()) {

      j = NULL;
      continue;
    }

    Timestamp target = j->target_time_;
    Timestamp next_target(microseconds(0));
    if (target.time_since_epoch().count() == 0) // 0 means ASAP. Control jobs (shutdown) are caught here.
      run = j->update(next_target);
    else {

      auto time_to_wait = duration_cast<microseconds>(target - Now());
      if (time_to_wait.count() == 0) // right on time: do the job.
        run = j->update(next_target);
      else if (time_to_wait.count() > 0) { // early: spawn a delegate to wait for the due time; delegate will die when done.

        DelegatedCore *d = new DelegatedCore(j->target_time_, time_to_wait, j);
        d->start(DelegatedCore::Wait);
        _Mem::Get()->register_time_job_latency(time_to_wait);
        next_target = Timestamp(seconds(0));
      } else { // late: do the job and report.

        run = j->update(next_target);
        j->report(-time_to_wait);
      }
    }

    while (next_target.time_since_epoch().count() && run) {

      if (!j->is_alive())
        break;

      auto time_to_wait = duration_cast<microseconds>(next_target - Now());
      next_target = Timestamp(seconds(0));
      if (time_to_wait.count() == 0) // right on time: do the job.
        run = j->update(next_target);
      else if (time_to_wait.count() > 0) { // early: spawn a delegate to wait for the due time; delegate will die when done.
                                  // the delegate will handle the next target when it is known (call to update()).
        DelegatedCore *d = new DelegatedCore(next_target, time_to_wait, j);
        d->start(DelegatedCore::Wait);
      } else { // late: do the job and report.

        run = j->update(next_target);
        j->report(-time_to_wait);
      }
    }
    j = NULL;
  }

  thread_ret_val(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TimeCore::TimeCore() : Thread() {
}

TimeCore::~TimeCore() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

thread_ret thread_function_call DelegatedCore::Wait(void *args) {

  _Mem::Get()->start_core();
  DelegatedCore *_this = ((DelegatedCore *)args);

  auto time_to_wait = _this->time_to_wait_;
  auto target_time = _this->target_time_;

wait: _this->timer_.start(time_to_wait);
  _this->timer_.wait();

  if (!_this->job_->is_alive())
    goto end;

  if (_Mem::Get()->check_state() == _Mem::RUNNING) { // checks for shutdown that could have happened during the wait on timer.

    while (Now() < target_time); // early, we have to wait; on Windows: timers resolution in ms => poll.
    target_time = Timestamp(seconds(0));
    _this->job_->update(target_time);
  }

redo: if (target_time.time_since_epoch().count()) {

  if (!_this->job_->is_alive())
    goto end;
  if (_Mem::Get()->check_state() != _Mem::RUNNING) // checks for shutdown that could have happened during the last update().
    goto end;

  time_to_wait = duration_cast<microseconds>(target_time - Now());
  if (time_to_wait.count() == 0) { // right on time: do the job.

    _this->job_->update(target_time);
    goto redo;
  } else if (time_to_wait.count() < 0) { // late.

    _this->job_->update(target_time);
    _this->job_->report(-time_to_wait);
    goto redo;
  } else
    goto wait;
}

    end: _Mem::Get()->shutdown_core();
      delete _this;
      thread_ret_val(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DelegatedCore::DelegatedCore(Timestamp target_time, microseconds time_to_wait, TimeJob *j) : Thread(), target_time_(target_time), time_to_wait_(time_to_wait), job_(j) {
}

DelegatedCore::~DelegatedCore() {
}
}
