

#include "hlp_context.h"
#include "operator.h"
#include "opcodes.h"


namespace r_exec {

HLPContext::HLPContext() : _Context(NULL, 0, NULL, UNDEFINED) {
}

HLPContext::HLPContext(Atom *code, uint16 index, HLPOverlay *const overlay, Data data) : _Context(code, index, overlay, data) {
}

bool HLPContext::operator ==(const HLPContext &c) const {

  HLPContext lhs = dereference();
  HLPContext rhs = c.dereference();

  if (lhs[0] != rhs[0]) // both contexts point to an atom which is not a pointer.
    return false;

  if (lhs[0].isStructural()) { // both are structural.

    uint16 atom_count = lhs.get_children_count();
    for (uint16 i = 1; i <= atom_count; ++i)
      if (lhs.get_child_deref(i) != rhs.get_child_deref(i))
        return false;
    return true;
  }
  return true;
}

bool HLPContext::operator !=(const HLPContext &c) const {

  return !(*this == c);
}

HLPContext HLPContext::dereference() const {

  switch ((*this)[0].getDescriptor()) {
  case Atom::I_PTR:
    return HLPContext(code_, (*this)[0].asIndex(), (HLPOverlay *)overlay_, data_).dereference();
  case Atom::VL_PTR: {
    Atom *value_code = ((HLPOverlay *)overlay_)->get_value_code((*this)[0].asIndex());
    if (value_code)
      return HLPContext(value_code, 0, (HLPOverlay *)overlay_, BINDING_MAP).dereference();
    else // unbound variable.
      return HLPContext(); // data=undefined: evaluation will return false.
  }case Atom::VALUE_PTR:
    return HLPContext(&overlay_->values_[0], (*this)[0].asIndex(), (HLPOverlay *)overlay_, VALUE_ARRAY).dereference();
  default:
    return *this;
  }
}

bool HLPContext::evaluate_no_dereference() const {

  switch (data_) {
  case VALUE_ARRAY:
  case BINDING_MAP:
    return true;
  case UNDEFINED:
    return false;
  }

  switch (code_[index_].getDescriptor()) {
  case Atom::ASSIGN_PTR: {

    HLPContext c(code_, code_[index_].asIndex(), (HLPOverlay *)overlay_);
    if (c.evaluate_no_dereference()) {

      ((HLPOverlay *)overlay_)->bindings_->bind_variable(code_, code_[index_].asAssignmentIndex(), code_[index_].asIndex(), &overlay_->values_[0]);
      return true;
    } else if (((HLPOverlay*)overlay_)->bindings_->scan_variable(code_[index_].asAssignmentIndex())) {
      // The assignment expression could not be evaluated, but the assignment variable is already bound.
      return true;
    } else
      return false;
  }case Atom::OPERATOR: {

    uint16 atom_count = get_children_count();
    for (uint16 i = 1; i <= atom_count; ++i) {

      if (!get_child_deref(i).evaluate_no_dereference())
        return false;
    }

    Operator op = Operator::Get((*this)[0].asOpcode());
    HLPContext *c = new HLPContext(*this);
    Context _c(c);
    return op(_c);
  }case Atom::OBJECT:
  case Atom::MARKER:
  case Atom::INSTANTIATED_PROGRAM:
  case Atom::INSTANTIATED_CPP_PROGRAM:
  case Atom::INSTANTIATED_INPUT_LESS_PROGRAM:
  case Atom::INSTANTIATED_ANTI_PROGRAM:
  case Atom::COMPOSITE_STATE:
  case Atom::MODEL:
  case Atom::GROUP:
  case Atom::SET:
  case Atom::S_SET: {

    uint16 atom_count = get_children_count();
    for (uint16 i = 1; i <= atom_count; ++i) {

      if (!get_child_deref(i).evaluate_no_dereference())
        return false;
    }
    return true;
  }default:
    return true;
  }
}

uint16 HLPContext::get_object_code_size() const {

  switch (data_) {
  case STEM:
    return ((HLPOverlay *)overlay_)->get_unpacked_object()->code_size();
  case BINDING_MAP:
    return ((HLPOverlay *)overlay_)->get_value_code_size((*this)[0].asIndex());
  case VALUE_ARRAY:
    return overlay_->values_.size();
  default:
    return 0;
  }
}
}
