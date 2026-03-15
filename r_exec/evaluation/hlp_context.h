

#ifndef hlp_context_h
#define hlp_context_h

#include "../r_code/object.h"

#include "_context.h"
#include "hlp_overlay.h"


namespace r_exec {

class dll_export HLPContext :
  public _Context {
public:
  HLPContext();
  HLPContext(Atom *code, uint16 index, HLPOverlay *const overlay, Data data = STEM);

  HLPContext dereference() const;

  HLPContext &operator =(const HLPContext &c) {

    code_ = c.code_;
    index_ = c.index_;
    return *this;
  }

  Atom &operator [](uint16 i) const { return code_[index_ + i]; }

  bool operator ==(const HLPContext &c) const;
  bool operator !=(const HLPContext &c) const;

  HLPContext get_child(uint16 index) const {

    return HLPContext(code_, index_ + index, (HLPOverlay *)overlay_);
  }

  /**
   * Call get_child(index) and then return the result of dereference().
   */
  HLPContext get_child_deref(uint16 index) const {
    return get_child(index).dereference();
  }

  bool evaluate() const {
    if (data_ == BINDING_MAP || data_ == VALUE_ARRAY)
      return true;

    HLPContext c = dereference();
    return c.evaluate_no_dereference();
  }

  bool evaluate_no_dereference() const;

  // _Context implementation.
  _Context *clone() override { return new HLPContext(*this); }

  bool equal(const _Context *c) const override { return *this == *(HLPContext *)c; }

  Atom &get_atom(uint16 i) const override { return this->operator [](i); }

  uint16 get_object_code_size() const override;

  uint16 get_children_count() const override {

    return code_[index_].getAtomCount();
  }

  /**
   * Call get_child and return a new allocated copy of the child. The caller is responsible to delete it.
   */
  _Context *get_child_new(uint16 index) const override {

    HLPContext *_c = new HLPContext(get_child(index));
    return _c;
  }

  /**
   * Dereference this and return a new allocated copy. The caller is responsible to delete it.
   */
  _Context *dereference_new() const override {

    HLPContext *_c = new HLPContext(dereference());
    return _c;
  }
};
}


#endif
