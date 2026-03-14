

#include "p_monitor.h"
#include "mem.h"
#include "mdl_controller.h"

using namespace r_code;

namespace r_exec {

Monitor::Monitor(MDLController *controller,
  BindingMap *bindings,
  Fact *target,
  Code* mk_rdx) : _Object(), controller_(controller) {

  bindings_ = bindings;
  target_ = target;
  mk_rdx_ = mk_rdx;
}

bool Monitor::is_alive() const {

  return !controller_->is_invalidated() && controller_->is_activated() && !target_->is_invalidated();
}
}
