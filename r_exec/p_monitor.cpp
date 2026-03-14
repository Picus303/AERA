

#include "p_monitor.h"
#include "mem.h"
#include "mdl_controller.h"

using namespace std::chrono;
using namespace r_code;

namespace r_exec {

PMonitor::PMonitor(MDLController *controller,
  BindingMap *bindings,
  Fact *prediction,
  Code* mk_rdx,
  bool rate_failures) : Monitor(controller, bindings, prediction, mk_rdx), rate_failures_(rate_failures) { // prediction is f0->pred->f1->obj; not simulated.

  prediction_target_ = prediction->get_pred()->get_target(); // f1.
  auto now = Now();

  bindings->reset_fwd_timings(prediction_target_);

  MonitoringJob<PMonitor> *j = new MonitoringJob<PMonitor>(this, prediction_target_->get_before() + Utils::GetTimeTolerance());
  _Mem::Get()->push_time_job(j);
}

PMonitor::~PMonitor() {
}

bool PMonitor::reduce(_Fact *input) { // input is always an actual fact.

  if (target_->is_invalidated()) {
    return true; }

  if (target_->get_pred()->grounds_invalidated(input)) { // input is a counter-evidence for one of the antecedents: abort.

    target_->invalidate();
    return true;
  }

  Pred *prediction = input->get_pred();
  if (prediction) {

    switch (prediction->get_target()->is_evidence(prediction_target_)) {
    case MATCH_SUCCESS_POSITIVE: // predicted confirmation, skip.
      return false;
    case MATCH_SUCCESS_NEGATIVE:
      if (prediction->get_target()->get_cfd() > prediction_target_->get_cfd()) {

        target_->invalidate(); // a predicted counter evidence is stronger than the target, invalidate and abort: don't rate the model.
        return true;
      } else
        return false;
    case MATCH_FAILURE:
      return false;
    }
  } else {
    //uint32 oid=input->get_oid();
    switch (((Fact *)input)->is_evidence(prediction_target_)) {
    case MATCH_SUCCESS_POSITIVE:
      controller_->register_pred_outcome(target_, mk_rdx_, true, input, input->get_cfd(), rate_failures_);
      return true;
    case MATCH_SUCCESS_NEGATIVE:
      if (rate_failures_)
        controller_->register_pred_outcome(target_, mk_rdx_, false, input, input->get_cfd(), rate_failures_);
      return true;
    case MATCH_FAILURE:
      return false;
    }
  }

  return false;
}

void PMonitor::update(Timestamp &next_target) { // executed by a time core, upon reaching the expected time of occurrence of the target of the prediction.

  if (!target_->is_invalidated()) {

    // Received nothing matching the target's object so far (neither positively nor negatively).
    // It is only a failure if the target is a fact. If the target is an anti-fact then reaching this
    // point means that the object was not observed *as expected* (not a failure).
    // TODO: If the model correctly predicts an anti-fact that the object won't be observed, then should we register
    // a success for the model? If yes then what should "evidence" point? Maybe nil?
    if (rate_failures_ && target_->get_pred()->get_target()->is_fact())
      controller_->register_pred_outcome(target_, mk_rdx_, false, NULL, 1, rate_failures_);
  }
  controller_->remove_monitor(this);
  next_target = Timestamp(seconds(0));
}
}
