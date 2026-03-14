

#ifndef hlp_controller_h
#define hlp_controller_h

#include "overlay.h"
#include "binding_map.h"
#include "g_monitor.h"
#include "group.h"
#include "init.h"


namespace r_exec {

typedef enum {
  WEAK_REQUIREMENT_DISABLED = 0,
  STRONG_REQUIREMENT_NO_WEAK_REQUIREMENT = 1,
  STRONG_REQUIREMENT_DISABLED_WEAK_REQUIREMENT = 2,
  WEAK_REQUIREMENT_ENABLED = 3,
  NO_REQUIREMENT = 4
}ChainingStatus;

class HLPController :
  public OController {
private:
  uint32 strong_requirement_count_; // number of active strong requirements in the same group; updated dynamically.
  uint32 weak_requirement_count_; // number of active weak requirements in the same group; updated dynamically.
  uint32 requirement_count_; // sum of the two above.
protected:
  class EvidenceEntry { // evidences.
  private:
    void load_data(_Fact *evidence);
  public:
    P<_Fact> evidence_;
    Timestamp after_;
    Timestamp before_;
    float32 confidence_;

    EvidenceEntry();
    EvidenceEntry(_Fact *evidence);
    EvidenceEntry(_Fact *evidence, _Fact *payload);

    bool is_too_old(Timestamp now) const {
      return evidence_->is_invalidated() ||
        // If an instantaneous fact, require if to be strictly before now to be considered old,
        // otherwise consider a time interval to be old if now is at the end of the interval.
        (after_ == before_ ? (before_ < now) : (before_ <= now));
    }
  };

  class PredictedEvidenceEntry : // predicted evidences.
    public EvidenceEntry {
  public:
    PredictedEvidenceEntry();
    PredictedEvidenceEntry(_Fact *evidence);
  };

  CriticalSectionList<EvidenceEntry> evidences_;
  CriticalSectionList<PredictedEvidenceEntry> predicted_evidences_;

  template<class E> void _store_evidence(CriticalSectionList<E> *cache, _Fact *evidence) {

    E e(evidence);
    cache->CS_.enter();
    auto now = Now();
    typename r_code::list<E>::const_iterator _e;
    for (_e = cache->list_.begin(); _e != cache->list_.end();) {

      if ((*_e).evidence_ == e.evidence_) {
        // Already stored.
        cache->CS_.leave();
        return;
      }
      if ((*_e).is_too_old(now)) // garbage collection.
        _e = cache->list_.erase(_e);
      else
        ++_e;
    }
    cache->list_.push_front(e);
    cache->CS_.leave();
  }

  P<HLPBindingMap> bindings_;

  /**
   * Evaluate the backward guards and update the binding map.
   * \param bm The binding map to update.
   * \param narrow_fwd_timings If true and the binding map fwd_after and fwd_before variables are bound, then
   * save them before evaluating the guards and then set the fwd_after and fwd_before variables to the intersection
   * of the previous values and the guard values. If omitted or false, then let the guards overwrite the
   * fwd_after and fwd_before variables.
   * \return True if the guards are successfully evaluated.
   */
  bool evaluate_bwd_guards(HLPBindingMap *bm, bool narrow_fwd_timings = false);

  MatchResult check_evidences(_Fact *target, _Fact *&evidence); // evidence with the match (positive or negative), get_absentee(target) otherwise.
  MatchResult check_predicted_evidences(_Fact *target, _Fact *&evidence); // evidence with the match (positive or negative), NULL otherwise.

  /**
   * For each output group, make a NotificationView and inject into the group.
   * \param marker The origin for the NotificationView.
   * \param marker The marker for the NotificationView.
   */
  void inject_notification_into_out_groups(r_code::Code* origin, r_code::Code* marker) const;

  bool has_tpl_args_;
  uint32 ref_count_; // used to detect _Object::refCount_ dropping down to 1 for hlp with tpl args.
  bool is_orphan(); // true when there are tpl args and no requirements: the controller cannot execute anymore.

  std::vector<P<HLPController> > controllers_; // all controllers for models/states instantiated in the patterns; case of models: [0]==lhs, [1]==rhs.
  Timestamp last_match_time_; // last time a match occurred (fwd), regardless of its outcome.
  bool become_invalidated(); // true if one controller is invalidated or if all controllers pointing to this are invalidated.
  virtual void kill_views() {}
  virtual void check_last_match_time(bool match) = 0;

  HLPController(r_code::_View *view);
public:
  virtual ~HLPController();

  void invalidate() override;

  r_code::Code *get_core_object() const override { return get_object(); } // cst or mdl.
  r_code::Code *get_unpacked_object() const { // the unpacked version of the core object.

    r_code::Code *core_object = get_core_object();
    return core_object->get_reference(core_object->references_size() - MDL_HIDDEN_REFS);
  }

  void add_requirement(bool strong);
  void remove_requirement(bool strong);

  uint32 get_requirement_count(uint32 &weak_requirement_count, uint32 &strong_requirement_count);
  uint32 get_requirement_count();

  void store_evidence(_Fact *evidence) { _store_evidence<EvidenceEntry>(&evidences_, evidence); }
  void store_predicted_evidence(_Fact *evidence) { _store_evidence <PredictedEvidenceEntry>(&predicted_evidences_, evidence); }

  virtual Fact *get_f_ihlp(HLPBindingMap *bindings, bool wr_enabled) const = 0;

  uint16 get_out_group_count() const;
  r_code::Code *get_out_group(uint16 i) const; // i starts at 1.
  Group *get_host() const { return (Group *)get_view()->get_host(); }
  bool has_tpl_args() const { return has_tpl_args_; }

  bool inject_prediction(Fact *prediction, float32 confidence) const; // for simulated predictions.
};
}


#endif
