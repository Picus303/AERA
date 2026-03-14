

#include "hlp_overlay.h"
#include "hlp_controller.h"
#include "hlp_context.h"
#include "runtime/mem.h"

using namespace r_code;

namespace r_exec {

bool HLPOverlay::EvaluateBWDGuards(Controller *c, HLPBindingMap *bindings) {

  HLPOverlay o(c, bindings);
  return o.evaluate_bwd_guards();
}

bool HLPOverlay::EvaluateFWDTimings(Controller *c, HLPBindingMap *bindings) {

  HLPOverlay o(c, bindings);
  return o.evaluate_fwd_timings();
}

bool HLPOverlay::ScanBWDGuards(Controller *c, HLPBindingMap *bindings) {

  const HLPOverlay o(c, bindings);
  return o.scan_bwd_guards();
}

HLPOverlay::HLPOverlay(Controller *c, HLPBindingMap *bindings) : Overlay(c, true), bindings_(bindings) {
}

HLPOverlay::HLPOverlay(Controller *c, const HLPBindingMap *bindings, bool load_code) : Overlay(c, load_code) {

  bindings_ = new HLPBindingMap((HLPBindingMap *)bindings);
}

HLPOverlay::~HLPOverlay() {
}

Atom *HLPOverlay::get_value_code(uint16 id) const {

  return bindings_->get_value_code(id);
}

uint16 HLPOverlay::get_value_code_size(uint16 id) const {

  return bindings_->get_value_code_size(id);
}

inline bool HLPOverlay::evaluate_guards(uint16 guard_set_iptr_index) {

  uint16 guard_set_index = code_[guard_set_iptr_index].asIndex();
  uint16 guard_count = code_[guard_set_index].getAtomCount();
  for (uint16 i = 1; i <= guard_count; ++i) {

    // Get the HLPContext like in HLPOverlay::evaluate.
    HLPContext c(code_, guard_set_index + i, this);
    if (!c.evaluate())
      return false;
    if (c.dereference()[0].isBooleanFalse())
      // This is a boolean guard (not an assignment) and it is false.
      return false;
  }
  return true;
}

bool HLPOverlay::evaluate_fwd_guards() {

  return evaluate_guards(HLP_FWD_GUARDS);
}

bool HLPOverlay::evaluate_bwd_guards() {

  return evaluate_guards(HLP_BWD_GUARDS);
}

bool HLPOverlay::evaluate(uint16 index) {

  HLPContext c(code_, index, this);
  return c.evaluate();
}

bool HLPOverlay::evaluate_fwd_timings() {

  int16 fwd_after_guard_index = -1;
  int16 fwd_before_guard_index = -1;

  uint16 bm_fwd_after_index = bindings_->get_fwd_after_index();
  uint16 bm_fwd_before_index = bindings_->get_fwd_before_index();

  uint16 guard_set_index = code_[HLP_BWD_GUARDS].asIndex();
  uint16 guard_count = code_[guard_set_index].getAtomCount();
  for (uint16 i = 1; i <= guard_count; ++i) { // find the relevant guards.

    uint16 index = guard_set_index + i;
    Atom a = code_[index];
    if (a.getDescriptor() == Atom::ASSIGN_PTR) {

      uint16 _i = a.asAssignmentIndex();
      if (_i == bm_fwd_after_index)
        fwd_after_guard_index = i;
      if (_i == bm_fwd_before_index)
        fwd_before_guard_index = i;
    }
  }

  // These are assignment guards, so we don't need result_index to check a boolean guard.
  if (!bindings_->has_fwd_before()) {
    // We need to evaluate forward before.
    if (fwd_before_guard_index == -1)
      // None of the backward guards assigns the variable for forward before.
      return false;
    if (!evaluate(guard_set_index + fwd_before_guard_index))
#if 1 // Debug: temporary solution to handle dependecies among guards. The full solution would recurse through the guards.
    {
      // This may depend on forward after, so try evaluating it first.
      if (fwd_after_guard_index == -1)
        // None of the backward guards assigns the variable for forward after.
        return false;
      if (!evaluate(guard_set_index + fwd_after_guard_index))
        return false;
      // Now try again to evaluate forward before.
      if (!evaluate(guard_set_index + fwd_before_guard_index))
        return false;
    }
#else
      return false;
#endif
  }

  if (!bindings_->has_fwd_after()) {
    // We need to evaluate forward after.
    if (fwd_after_guard_index == -1)
      // None of the backward guards assigns the variable for forward after.
      return false;
    if (!evaluate(guard_set_index + fwd_after_guard_index))
      return false;
  }

  return true;
}

bool HLPOverlay::scan_bwd_guards() const {

  uint16 guard_set_index = code_[HLP_BWD_GUARDS].asIndex();
  uint16 guard_count = code_[guard_set_index].getAtomCount();
  for (uint16 i = 1; i <= guard_count; ++i) {

    uint16 index = guard_set_index + i;
    Atom a = code_[index];
    switch (a.getDescriptor()) {
    case Atom::I_PTR:
      if (!scan_location(a.asIndex(), index))
        return false;
      break;
    case Atom::ASSIGN_PTR:
      // If scan_location fails, then succeed if the assignment variable is already bound.
      if (!scan_location(a.asIndex(), index) && !bindings_->scan_variable(a.asAssignmentIndex()))
        return false;
      break;
    }
  }
  return true;
}

bool HLPOverlay::scan_location(uint16 index, uint16 parent_guard_index) const {

  Atom a = code_[index];
  switch (a.getDescriptor()) {
  case Atom::I_PTR:
    return scan_location(a.asIndex(), parent_guard_index);
  case Atom::ASSIGN_PTR:
    return scan_location(a.asIndex(), parent_guard_index);
  case Atom::VL_PTR:
    if (bindings_->scan_variable(a.asIndex()))
      return true;
    else
      return scan_variable(a.asIndex(), parent_guard_index);
  case Atom::OPERATOR: {
    uint16 atom_count = a.getAtomCount();
    for (uint16 j = 1; j <= atom_count; ++j) {

      if (!scan_location(index + j, parent_guard_index))
        return false;
    }
    return true;
  }
  default:
    return true;
  }
}

bool HLPOverlay::scan_variable(uint16 index, uint16 parent_guard_index) const { // check if the variable can be bound.

  uint16 guard_set_index = code_[HLP_BWD_GUARDS].asIndex();
  uint16 guard_count = code_[guard_set_index].getAtomCount();
  for (uint16 i = 1; i <= guard_count; ++i) {

    uint16 guard_index = guard_set_index + i;
    Atom a = code_[guard_index];
    switch (a.getDescriptor()) {
    case Atom::ASSIGN_PTR:
      if (a.asAssignmentIndex() == index) {
        if (guard_index == parent_guard_index)
          // Prevent loops.
          return false;
        return scan_location(a.asIndex(), guard_index);
      }
      break;
    }
  }

  return false;
}

Code *HLPOverlay::get_unpacked_object() const {

  return ((HLPController *)controller_)->get_unpacked_object();
}

void HLPOverlay::store_evidence(_Fact *evidence, bool prediction, bool is_simulation) {

  if (prediction) {

    if (!is_simulation)
      ((HLPController *)controller_)->store_predicted_evidence(evidence);
  } else
    ((HLPController *)controller_)->store_evidence(evidence);
}
}
