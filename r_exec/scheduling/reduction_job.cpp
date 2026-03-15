

#include "reduction_job.h"
#include "runtime/mem.h"


namespace r_exec {

uint32 _ReductionJob::job_count_ = 0;

_ReductionJob::_ReductionJob() : _Object() {
  // Increment without thread lock. It's only for tracing.
  job_id_ = ++job_count_;
}

void _ReductionJob::register_latency(Timestamp now) {
  _Mem::Get()->register_reduction_job_latency(now - ijt_);
}

////////////////////////////////////////////////////////////

bool ShutdownReductionCore::update(Timestamp now) {

  return false;
}

////////////////////////////////////////////////////////////

bool AsyncInjectionJob::update(Timestamp now) {

  _Mem::Get()->inject(input_);
  return true;
}
}
