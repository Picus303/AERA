

#include "reduction_core.h"
#include "mem.h"
#include "init.h"


namespace r_exec {

thread_ret thread_function_call ReductionCore::Run(void *args) {

  ReductionCore *_this = ((ReductionCore *)args);

  bool run = true;
  while (run) {

    P<_ReductionJob> j = _Mem::Get()->pop_reduction_job();
    if (j == NULL)
      break;
    run = j->update(Now());
    j = NULL;
  }

  thread_ret_val(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ReductionCore::ReductionCore() : Thread() {
}

ReductionCore::~ReductionCore() {
}
}
