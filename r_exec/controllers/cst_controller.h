

#ifndef cst_controller_h
#define cst_controller_h

#include "hlp_overlay.h"
#include "hlp_controller.h"
#include "construction/factory.h"


namespace r_exec {

// All inputs are expected to be synchronized (within the time tolerance).
// The time value of each fact is therefore the same (within the time tolerance).
// Therefore, said time value is not listed in the argument list of the icst.
// It is held by the fact holding said icst.
// Inputs synchronized on state are treated as if they were produced continuously, i.e. Now().
// The confidence value for an icst is the lowest value taken from the matched inputs.
// No cmds or imdls in a cst.
//
// Forward chaining:
// output a prediction of icst instead of icst if at least one input is a prediction.
// output as many predictions of icst as we got simulated predictions for different goals.
class CSTOverlay :
  public HLPOverlay {
protected:
  Timestamp match_deadline_; // before deadline after the last match.
  float32 lowest_cfd_; // among the inputs (forward chaining).

  // The set of accumulated DefeasibleValidity from inputs, which is copied to each produced Pred.
  std::set<P<DefeasibleValidity> > defeasible_validities_;

  std::unordered_set<P<_Fact>, r_code::PHash<_Fact> > predictions_; // f0->pred->f1->obj.
  std::unordered_set<P<Sim>, r_code::PHash<Sim> > simulations_;
  // This tracks whether promoting has already been done for this Sim. It needs to be a list of Sim
  // because this overlay may be for the initial non-simulated icst which can be promoted for multiple sims.
  std::vector<Sim*> promoted_in_sim_;

  r_code::list<P<_Fact> > axiom_patterns_;
  r_code::list<P<_Fact> > non_axiom_patterns_;
  std::vector<P<_Fact> > axiom_inputs_;
  std::vector<P<_Fact> > non_axiom_inputs_;

  /**
   * Use bindings_ to make the f_icst, putting it in a (fact (pred ...)) if needed.
   * \param input The input which triggered the production.
   * \return The injected object, or NULL if not injected.
   */
  _Fact* inject_production(View* input);
  void update(HLPBindingMap *map, _Fact *input, bool is_axiom);

  /**
   * Make a copy of this CSTOverlay, then call update() to update the inputs_ and bindings_.
   * \param map The HLPBindingMap to copy to bindings_. This also updates match_deadline_ from
   * map->get_fwd_before() if needed.
   * \param input The _Fact to add to inputs_.
   * \param is_axiom True if input (and bound_pattern) is matched from axiom_patterns_.
   * \param bound_pattern (optional) The pattern to remove from axiom_patterns_ or non_axiom_patterns_
   * (according to is_axiom). If omitted or NULL, then don't use it.
   * \return The copy of this CSTOverlay before making changes.
   */
  CSTOverlay *get_offspring(HLPBindingMap *map, _Fact *input, bool is_axiom, _Fact *bound_pattern = NULL);

  /**
   * Similar to Pred::get_simulation, find the first Sim in simulations_ whose root_ is root.
   * \param root The root to match with the Sim root_.
   * \return The first matching Sim, or NULL if no match.
   */
  Sim* get_simulation(Controller *root) const;

  CSTOverlay(const CSTOverlay *original);
public:
  CSTOverlay(Controller *c, HLPBindingMap *bindings);
  ~CSTOverlay();

  bool reduce(View *input, CSTOverlay *&offspring);

  void load_patterns();

  bool can_match(Timestamp now) const;

  bool is_simulated() { return simulations_.size() > 0; }

  /**
   * Find the first _Fact in axiom_patterns_ or non_axiom_patterns_ which matches the input, and update
   * the binding map.
   * \param input The input _Fact to match against a pattern.
   * \param map The binding map for calling match_fwd_strict and has the bindings
   * if this returns a pattern.
   * \param predictionSimulation If not NULL, then ensure that its simulation root matches the root
   * of the simulation in this object's simulations_.
   * Also if this overlay does not yet have non-axiom saved inputs, then use the timings from input to
   * update the timing variables of this CSTOverlay.
   * \param is_axiom True if the matching pattern came from axiom_patterns_ .
   * \return The matching pattern from axiom_patterns_ or non_axiom_patterns_, or NULL if not found.
   */
  _Fact* bind_pattern(_Fact *input, HLPBindingMap* map, Sim* predictionSimulation, bool& is_axiom);
};

// Backward chaining:
// if there are requirements, do nothing: these requirements will get the goal and abduce.
// else
// bind all patterns and look in the cache for positive evidences; for all bound patterns not matched in the cache, output a sub-goal (simulated or not, depending on the super-goal).
class CSTController :
  public HLPController {
private:
  friend class CSTOverlay;
  Group *secondary_host_;

  /**
   * For each of the overlays, make a copy of bm and merge it with the overlay's binding map, then call abduce() with the
   * merged binding map and f_super_goal. In this way we make a branch of the simulation with each possible way of
   * instantiating the cst.
   * \param bm The binding map created by reduce.
   * \param f_super_goal The super goal to pass to abduce().
   */
  void abduce_simulated(HLPBindingMap *bm, Fact *f_super_goal);
  void abduce(HLPBindingMap *bm, Fact *f_super_goal); // f_super_goal is f0->g->f1->icst or f0->g->|f1->icst.
  void inject_goal(HLPBindingMap *bm,
    Fact *f_super_goal, // f0->g->f1->icst or f0->g->|f1->icst.
    _Fact *sub_goal_target, // f1.
    Sim *sim,
    Timestamp now,
    float32 confidence,
    r_code::Code *group) const;

  void kill_views() override;
  void check_last_match_time(bool match) override; // kill if no match after primary_thz;
public:
  CSTController(r_code::_View *view);
  ~CSTController();

  void take_input(r_exec::View *input) override;
  void reduce(r_exec::View *input);

  Fact *get_f_ihlp(HLPBindingMap *bindings, bool wr_enabled) const override;
  Fact *get_f_icst(HLPBindingMap *bindings, std::vector<P<_Fact> > *axiom_inputs, std::vector<P<_Fact> > *non_axiom_inputs) const;

  void inject_icst(Fact *production, float32 confidence, std::chrono::microseconds time_to_live) const; // here, resilience=time to live, in us.
  bool inject_prediction(Fact *prediction, float32 confidence, std::chrono::microseconds time_to_live) const; // here, resilience=time to live, in us; returns true if the prediction has actually been injected.

  void set_secondary_host(Group *host);
  Group *get_secondary_host() const;
};
}


#endif
