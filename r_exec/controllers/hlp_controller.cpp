

#include "hlp_controller.h"
#include "hlp_overlay.h"
#include "mem.h"

using namespace r_code;

namespace r_exec {

HLPController::HLPController(_View *view) : OController(view), strong_requirement_count_(0), weak_requirement_count_(0), requirement_count_(0) {

  bindings_ = new HLPBindingMap();

  Code *object = get_unpacked_object();
  bindings_->init_from_hlp(object, get_object()); // init a binding map from the patterns.

  has_tpl_args_ = object->code(object->code(HLP_TPL_ARGS).asIndex()).getAtomCount() > 0;
  ref_count_ = 0;
  last_match_time_ = Now();
}

HLPController::~HLPController() {
}

void HLPController::invalidate() {

  invalidated_ = 1;
  controllers_.clear();
}

void HLPController::add_requirement(bool strong) {

  reductionCS_.enter();
  if (strong)
    ++strong_requirement_count_;
  else
    ++weak_requirement_count_;
  ++requirement_count_;
  reductionCS_.leave();
}

void HLPController::remove_requirement(bool strong) {

  reductionCS_.enter();
  if (strong)
    --strong_requirement_count_;
  else
    --weak_requirement_count_;
  --requirement_count_;
  reductionCS_.leave();
}

uint32 HLPController::get_requirement_count(uint32 &weak_requirement_count, uint32 &strong_requirement_count) {

  uint32 r_c;
  reductionCS_.enter();
  r_c = requirement_count_;
  weak_requirement_count = weak_requirement_count_;
  strong_requirement_count = strong_requirement_count_;
  reductionCS_.leave();
  return r_c;
}

uint32 HLPController::get_requirement_count() {

  uint32 r_c;
  reductionCS_.enter();
  r_c = requirement_count_;
  reductionCS_.leave();
  return r_c;
}

uint16 HLPController::get_out_group_count() const {

  return get_object()->code(get_object()->code(HLP_OUT_GRPS).asIndex()).getAtomCount();
}

Code *HLPController::get_out_group(uint16 i) const {

  Code *hlp = get_object();
  uint16 out_groups_index = hlp->code(HLP_OUT_GRPS).asIndex() + 1; // first output group index.
  return hlp->get_reference(hlp->code(out_groups_index + i).asIndex());
}

bool HLPController::evaluate_bwd_guards(HLPBindingMap *bm, bool narrow_fwd_timings) {

  bool saved_fwd_timings = false;
  Timestamp save_fwd_after, save_fwd_before;
  if (narrow_fwd_timings && bm->has_fwd_after() && bm->has_fwd_before()) {
    // The forward timings have previously-bound values from matching the requirement.
    save_fwd_after = bm->get_fwd_after();
    save_fwd_before = bm->get_fwd_before();
    saved_fwd_timings = true;
  }

  bool result = HLPOverlay::EvaluateBWDGuards(this, bm);

  if (saved_fwd_timings) {
    // Call match_fwd_timings to narrow the forward timings set by the backward guards to the previously-bound values.
    if (!bm->match_fwd_timings(save_fwd_after, save_fwd_before))
      // The backward guards updated the forward timings outside the previously-bound values. (We don't expect this.)
      return false;
  }

  return result;
}

bool HLPController::inject_prediction(Fact *prediction, float32 confidence) const { // prediction is simulated: f->pred->f->target.

  Code *primary_host = get_host();
  float32 sln_thr = primary_host->code(GRP_SLN_THR).asFloat();
  if (confidence > sln_thr) { // do not inject if cfd is too low.

    View *view = new View(View::SYNC_ONCE, Now(), confidence, 1, primary_host, primary_host, prediction); // SYNC_ONCE,res=1.
    _Mem::Get()->inject(view);
    return true;
  }

  return false;
}

MatchResult HLPController::check_evidences(_Fact *target, _Fact *&evidence) {

  MatchResult r = MATCH_FAILURE;
  evidences_.CS_.enter();
  auto now = Now();
  r_code::list<EvidenceEntry>::const_iterator e;
  for (e = evidences_.list_.begin(); e != evidences_.list_.end();) {

    if ((*e).is_too_old(now)) // garbage collection.
      e = evidences_.list_.erase(e);
    else {

      if ((r = (*e).evidence_->is_evidence(target)) != MATCH_FAILURE) {

        evidence = (*e).evidence_;
        evidences_.CS_.leave();
        return r;
      }
      ++e;
    }
  }
  evidence = NULL;
  evidences_.CS_.leave();
  return r;
}

MatchResult HLPController::check_predicted_evidences(_Fact *target, _Fact *&evidence) {

  MatchResult r = MATCH_FAILURE;
  predicted_evidences_.CS_.enter();
  auto now = Now();
  r_code::list<PredictedEvidenceEntry>::const_iterator e;
  for (e = predicted_evidences_.list_.begin(); e != predicted_evidences_.list_.end();) {

    if ((*e).is_too_old(now)) // garbage collection.
      e = predicted_evidences_.list_.erase(e);
    else {

      if ((r = (*e).evidence_->is_evidence(target)) != MATCH_FAILURE) {

        if (target->get_cfd() < (*e).evidence_->get_cfd()) {

          evidence = (*e).evidence_;
          predicted_evidences_.CS_.leave();
          return r;
        } else
          r = MATCH_FAILURE;
      }
      ++e;
    }
  }
  evidence = NULL;
  predicted_evidences_.CS_.leave();
  return r;
}

bool HLPController::become_invalidated() {

  if (is_invalidated())
    return true;

  for (uint16 i = 0; i < controllers_.size(); ++i) {

    if (controllers_[i] != NULL && controllers_[i]->is_invalidated()) {

      kill_views();
      return true;
    }
  }

  if (has_tpl_args()) {

    if (refCount_ == 1 && ref_count_ > 1) { // the ctrler was referenced by others, is no longer and has tpl args: it will not be able to predict: kill.

      kill_views();
      return true;
    } else
      ref_count_ = refCount_;
  }

  return false;
}

bool HLPController::is_orphan() {

  if (has_tpl_args_) {

    if (get_requirement_count() == 0) {

      kill_views();
      return true;
    }
  }
  return false;
}

void HLPController::inject_notification_into_out_groups(Code* origin, Code* marker) const {
  uint16 out_group_count = get_out_group_count();
  for (uint16 i = 0; i < out_group_count; ++i) {

    Group* out_group = (Group*)get_out_group(i);
    View* view = new NotificationView(origin, out_group, marker);
    _Mem::Get()->inject_notification(view, true);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HLPController::EvidenceEntry::EvidenceEntry() : evidence_(NULL) {
}

HLPController::EvidenceEntry::EvidenceEntry(_Fact *evidence) : evidence_(evidence) {

  load_data(evidence);
}

HLPController::EvidenceEntry::EvidenceEntry(_Fact *evidence, _Fact *payload) : evidence_(evidence) {

  load_data(payload);
}

void HLPController::EvidenceEntry::load_data(_Fact *evidence) {

  after_ = evidence->get_after();
  before_ = evidence->get_before();
  confidence_ = evidence->get_cfd();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HLPController::PredictedEvidenceEntry::PredictedEvidenceEntry() : EvidenceEntry() {
}

HLPController::PredictedEvidenceEntry::PredictedEvidenceEntry(_Fact *evidence) : EvidenceEntry(evidence, evidence->get_pred()->get_target()) {
}
}
