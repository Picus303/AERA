

#ifndef hlp_overlay_h
#define hlp_overlay_h

#include "overlay.h"
#include "binding_map.h"


namespace r_exec {

class HLPContext;

// HLP: high-level patterns.
class HLPOverlay :
  public Overlay {
  friend class HLPContext;
protected:
  friend class CSTController; // For abduce_simulated.
  P<HLPBindingMap> bindings_;

  bool evaluate_guards(uint16 guard_set_iptr_index);
  bool evaluate_fwd_guards();
  /**
   * Create an HLPContext and evaluate code_ at index.
   * \param index The index in code_ to evaluate.
   * \return True if successfully evaluated, false for problem evaluated including
   * unbound variables. Note that if the code at index is a boolean expression,
   * then this can return true for successful evaluation even though the expression value
   * is boolean false.
   */
  bool evaluate(uint16 index);

  bool evaluate_fwd_timings();

  bool scan_bwd_guards() const;
  bool scan_location(uint16 index, uint16 parent_guard_index) const;
  bool scan_variable(uint16 index, uint16 parent_guard_index) const;

  void store_evidence(_Fact *evidence, bool prediction, bool is_simulation); // stores both actual and non-simulated predicted evidences.

  HLPOverlay(Controller *c, HLPBindingMap *bindings);
public:
  static bool EvaluateBWDGuards(Controller *c, HLPBindingMap *bindings); // updates the bindings.

  /**
   * Find the backward guards which assign the forward timings and evaluate them.
   * \param c The model controller with the code for the backward guards.
   * \param bindings Update the binding map forward timings.
   * \return True if evaluted, false if no backward guards assign the forward timings or if can't evaluate.
   */
  static bool EvaluateFWDTimings(Controller *c, HLPBindingMap *bindings);

  static bool ScanBWDGuards(Controller *c, HLPBindingMap *bindings); // does not update the bindings.

  HLPOverlay(Controller *c, const HLPBindingMap *bindings, bool load_code);
  virtual ~HLPOverlay();

  HLPBindingMap *get_bindings() const { return bindings_; }

  Atom *get_value_code(uint16 id) const;
  uint16 get_value_code_size(uint16 id) const;

  r_code::Code *get_unpacked_object() const;

  bool evaluate_bwd_guards();
};
}


#endif
