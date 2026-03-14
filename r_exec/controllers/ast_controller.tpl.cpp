

#include "ast_controller.h"
#include "mem.h"
#include "factory.h"
#include "attention/auto_focus.h"

using namespace r_code;

namespace r_exec {

template<class U> ASTController<U>::ASTController(AutoFocusController *auto_focus, View *target) : OController(NULL) {

  target_ = (_Fact *)target->object_;
  tpx_ = new CTPX(auto_focus, target); // target is the premise, i.e. the tpx' target to be defeated.
  thz_timestamp_ = Now() - Utils::GetTimeTolerance();
}

template<class U> ASTController<U>::~ASTController() {
}

template<class U> void ASTController<U>::take_input(r_exec::View *input) {

  if (is_invalidated())
    return;

  if (input->object_->code(0).asOpcode() == Opcodes::Fact ||
    input->object_->code(0).asOpcode() == Opcodes::AntiFact) { // discard everything but facts and |facts.

    Goal *goal = ((_Fact *)input->object_)->get_goal();
    if (goal && goal->get_actor() == _Mem::Get()->get_self()) // ignore self's goals.
      return;
    if (input->get_ijt() >= thz_timestamp_) // input is too old.
      Controller::__take_input<U>(input);
  }
}

template<class U> void ASTController<U>::reduce(View *input) {

  if (is_invalidated())
    return;

  _Fact *input_object = input->object_;
  if (input_object->is_invalidated()) {
    return; }
  reductionCS_.enter();

  if (input_object == target_) {

    tpx_->store_input(input);
    reductionCS_.leave();
    return;
  }

  Pred *prediction = input_object->get_pred();
  if (prediction) {
    // Don't cancel if a model made a prediction of an anti-fact.
    if (prediction->get_target()->is_fact()) {
      switch (prediction->get_target()->is_timeless_evidence(target_)) {
      case MATCH_SUCCESS_POSITIVE:
      case MATCH_SUCCESS_NEGATIVE: // a model predicted the next value of the target.
        kill();
        break;
      case MATCH_FAILURE:
        break;
      }
    }

    reductionCS_.leave();
    return;
  }
  ((U *)this)->reduce(input, input_object);

  reductionCS_.leave();
}

template<class U> void ASTController<U>::kill() {

  invalidate();
  get_view()->force_res(0);
}
}
