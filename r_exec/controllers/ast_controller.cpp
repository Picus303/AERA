

#include "ast_controller.h"
#include "runtime/mem.h"
#include "construction/factory.h"
#include "attention/auto_focus.h"


namespace r_exec {

PASTController::PASTController(AutoFocusController *auto_focus, View *target) : ASTController<PASTController>(auto_focus, target) {

}

PASTController::~PASTController() {
}

void PASTController::reduce(View *v, _Fact *input) {

  switch (input->is_timeless_evidence(target_)) {
  case MATCH_SUCCESS_POSITIVE:
    kill();
    target_->invalidate();
    break;
  case MATCH_SUCCESS_NEGATIVE:
    kill();
    tpx_->signal(v);
#if 0 // Set 0 to not invalidate. Temporary solution to https://github.com/IIIM-IS/replicode/issues/162
    target_->invalidate();
#endif
    break;
  case MATCH_FAILURE:
    tpx_->store_input(v);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

HASTController::HASTController(AutoFocusController *auto_focus, View *target, _Fact *source) : ASTController<HASTController>(auto_focus, target), source_(source) {

}

HASTController::~HASTController() {
}

void HASTController::reduce(View *v, _Fact *input) {

  switch (input->is_timeless_evidence(target_)) {
  case MATCH_SUCCESS_POSITIVE:
    kill();
    break;
  case MATCH_SUCCESS_NEGATIVE:
    kill();
    tpx_->signal(v);
    target_->invalidate();
    source_->invalidate();
    break;
  case MATCH_FAILURE:
    tpx_->store_input(v);
    break;
  }
}
}
