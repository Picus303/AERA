

#ifndef g_monitor_h
#define g_monitor_h

#include "monitor.h"


namespace r_exec {

class PMDLController;
class PrimaryMDLController;

class _GMonitor :
  public Monitor {
protected:
  Timestamp deadline_; // of the goal.
  Timestamp sim_thz_timestamp_;
  _Fact *goal_target_; // convenience; f1->object.
  P<Fact> f_imdl_;
  SimMode sim_mode_;

  uint32 volatile simulating_; // 32 bits alignment.

  // The first in the pair is the (fact (pred (fact (success...)))) or
  // (fact (pred (|fact (success...)))) where the success's object is the (fact (goal ...)).
  typedef std::list<std::pair<P<_Fact>, P<Sim> > > SolutionList;
  
  /* From the (fact (pred (fact (success...)))) of a solution, get the goal from
   * the success object's (fact (goal ...)).
   */
  static Goal* get_solution_goal(_Fact* f_pred_f_success) {
    Pred* pred = f_pred_f_success->get_pred();
    if (!pred)
      return NULL;
    Success* success = pred->get_target()->get_success();
    if (!success)
      return NULL;
    return success->get_object()->get_goal();
  }

  class SimOutcomes {
  public:
    SolutionList mandatory_solutions;
    SolutionList optional_solutions;
  };

  // Simulated predictions of any goal success resulting from the simulation of the monitored goal.
  SimOutcomes sim_successes_;
  SimOutcomes sim_failures_;

  /**
   * Store the outcome of any goal affected by the simulation of the monitored goal. If
   * (fact (pred (fact ...))) then store in sim_successes_. Otherwise if
   * (fact (pred (|fact ...))) then store in sim_failures_.
   * \param f_pred_f_success The (fact (pred (fact (success...)))) or
   * (fact (pred (|fact (success...)))) where the success's object is the affected (fact (goal ...)).
   * \param sim The Sim object (for the relevant controller) from the prediction.
   */
  void store_simulated_outcome(_Fact *f_pred_f_success, Sim *sim);
  void invalidate_sim_outcomes();

  _GMonitor(PMDLController *controller,
    BindingMap *bindings,
    Timestamp deadline,
    Timestamp sim_thz_timestamp,
    Fact *goal,
    Fact *f_imdl); // goal is f0->g->f1->object.
public:
  /**
   * If prediction->is_simulation(), then this is for a simulation.
   */
  virtual bool signal(Pred* /* prediction */) { return false; }
};

/**
 * Monitors goals (other than requirements).
 * Use for SIM_ROOT.
 * Is aware of any predicted evidence for the goal target: if at construction time such an evidence is known, the goal is not injected.
 * Reporting a success or failure to the controller invalidates the goal; reporting a predicted success also does.
 * Reporting a predicted failure injects the goal if it has not been already, invalidates it otherwise (a new goal will be injected).
 * The monitor still runs after reporting a predicted success.
 * The monitor does not run anymore if the goal is invalidated (case of a predicted success, followed by a predicted failure).
 * Wait for the time horizon; in the meantime:
 * actual inputs:
 * If an input is an evidence for the target, report a success.
 * If an input is a counter-evidence of the target, report a failure.
 * If an input is a predicted evidence for the target, report a predicted success.
 * If an input is a predicted counter-evidence for the target, report a predicted failure.
 * If there is a predicted evidence for the target that becomes invalidated, report a predicted failure.
 * Simulated predictions: catch only those that are a simulation for the monitored goal.
 * If an input is an evidence of the goal target, simulate a prediction of the goal success.
 * If an input is an evidence of the goal target, simulate a prediction of the goal failure.
 * Store any simulated prediction of success/failure for any goal.
 * At the time horizon:
 * Simulation mode:
 *   Commit to the appropriate solutions for the goal.
 *   Mode become actual.
 *   Time horizon becomes the goal deadline.
 * Actual mode:
 *   If the goal is not invalidated, report a failure.
 */
class GMonitor :
  public _GMonitor {
protected:
  _Fact *volatile predicted_evidence_; // f0->pred->f1->object; 32 bits alignment.
  bool injected_goal_;

  void commit();
public:
  GMonitor(PMDLController *controller,
    BindingMap *bindings,
    Timestamp deadline,
    Timestamp sim_thz_timestamp,
    Fact *goal,
    Fact *f_imdl,
    _Fact *predicted_evidence); // goal is f0->g->f1->object.

  bool reduce(_Fact *input) override; // returning true will remove the monitor form the controller.
  virtual void update(Timestamp &next_target);
};

/**
 * Monitors actual requirements.
 * Use for SIM_ROOT.
 * target==f_imdl; this means we need to fullfill some requirements:
 * Wait until the deadline of the goal, in the meantime:
 * Each time the monitor is signalled (i.e. a new pred->f_imdl has been produced), check if chaining is allowed:
 * If no, do nothing.
 * If yes: assert success and abort: the model will bind its rhs with the bm retrieved from the pred->f_imdl; this will
 * Kill the monitor and a new one will be built for the bound rhs sub-goal.
 * At the deadline, assert failure.
 */
class RMonitor :
  public GMonitor {
public:
  RMonitor(PrimaryMDLController *controller,
    BindingMap *bindings,
    Timestamp deadline,
    Timestamp sim_thz_timestamp,
    Fact *goal,
    Fact *f_imdl);

  bool reduce(_Fact *input) override;
  void update(Timestamp &next_target) override;
  bool signal(Pred* prediction) override;
};

// Monitors simulated goals.
class SGMonitor :
  public _GMonitor {
protected:
  void commit();
public:
  SGMonitor(PrimaryMDLController *controller,
    BindingMap *bindings,
    Timestamp sim_thz_timestamp,
    Fact *goal,
    Fact *f_imdl); // goal is f0->g->f1->object.

  bool reduce(_Fact *input) override;
  void update(Timestamp &next_target);
};

// Monitors simulated requirements.
// Use for SIM_OPTIONAL and SIM_MANDATORY.
class SRMonitor :
  public SGMonitor {
public:
  SRMonitor(PrimaryMDLController *controller,
    BindingMap *bindings,
    Timestamp sim_thz_timestamp,
    Fact *goal,
    Fact *f_imdl);

  bool reduce(_Fact *input) override;
  void update(Timestamp &next_target);

  bool signal(Pred* prediction) override;
};

// Case A: target==actual goal and target!=f_imdl: simulations have been produced for all sub-goals.
// Wait until the STHZ, in the meantime:
// if input==goal target, assert success and abort: invalidate goal (this will invalidate the related simulations).
// if input==|goal target, assert failure and abort: the super-goal will probably fail and so on, until some drives fail, which will trigger their re-injection.
// if input==pred goal target, do nothing.
// if input==pred |goal target, do nothing.
// if input==pred success/failure of any other goal and sim->super-goal==goal, store the simulation for decision at STHZ.
// if input==pred goal target and sim->super-goal==goal, store the simulation for decision at STHZ.
// if input==pred |goal target and sim->super-goal==goal, store the simulation for decision at STHZ.
// if input==pred goal target and sim->super-goal!=goal, predict success for the goal.
// if input==pred |goal target and sim->super-goal!=goal, predict failure for the goal.
// At STHZ, choose the best simulations if any, and commit to their sub-goals; kill the predictions for the discarded simulations.
//
// Case B: target==f_imdl; this means we need to fullfill some requirements: simulations have been produced for all the sub-goals of f_imdl.
// Wait until the STHZ, in the meantime:
// if input==pred success/failure of any goal and sim->super-goal==goal, store the simulation for decision at STHZ.
// At STHZ, choose the best simulations if any, and commit to their sub-goals; kill the predictions for the discarded simulations.
//
// Case C: target==simulated goal and target!=f_imdl: simulations have been produced for all sub-goals.
// Wait until the STHZ, in the meantime:
// if input==goal target, predict success for the goal and abort: invalidate goal.
// if input==|goal target, predict failure for the goal and abort: invalidate goal.
// if input==pred success/failure of any other goal and sim->super-goal==goal, store the simulation for decision at STHZ.
// if input==pred goal target and sim->super-goal==goal, store the simulation for decision at STHZ.
// if input==pred |goal target and sim->super-goal==goal, store the simulation for decision at STHZ.
// if input==pred goal target and sim->super-goal!=goal, predict success for the goal.
// if input==pred |goal target and sim->super-goal!=goal, predict failure for the goal.
//
// Case D: target==simulated f_imdl; this means we need to fullfill some requirements: simulations have been produced for all the sub-goals of f_imdl.
// Wait until the STHZ, in the meantime:
// if input==pred success/failure of any goal and sim->super-goal==goal, store the simulation for decision at STHZ.
// At STHZ, choose the best simulations if any, and commit to their sub-goals; kill the predictions for the discarded simulations.
}


#endif
