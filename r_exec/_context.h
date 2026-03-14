

#ifndef _context_h
#define _context_h

#include "../r_code/atoms/atom.h"
#include "overlay.h"

namespace r_exec {

// Base class for evaluation contexts.
// Subclasses: IPGMContext and HLPContext.
// _Context * wrapped in Context, the latter used by operators.
class dll_export _Context {
protected:
  Overlay *const overlay_; // the overlay where the evaluation is performed; NULL when the context is dereferenced outside the original pgm or outside the value array.
  Atom *code_; // the object's code, or the code in value array, or the view's code when the context is dereferenced from Atom::VIEW.
  uint16 index_; // in the code;

  typedef enum { // indicates whether the context refers to:
    STEM = 0, // - the pgm/hlp being reducing inputs;
    REFERENCE = 1, // - a reference to another object;
    VIEW = 2, // - a view;
    MKS = 3, // - the mks of an object;
    VWS = 4, // - the vws of an object;
    VALUE_ARRAY = 5, // - code in the overlay's value array.
    BINDING_MAP = 6, // - values of a imdl/icst.
    UNDEFINED = 7
  }Data;
  Data data_;

  _Context(Atom *code, uint16 index, Overlay *overlay, Data data) : code_(code), index_(index), overlay_(overlay), data_(data) {}
public:
  virtual _Context *clone() = 0;

  virtual bool equal(const _Context *c) const = 0;

  virtual Atom &get_atom(uint16 i) const = 0;

  virtual uint16 get_object_code_size() const = 0;

  virtual uint16 get_children_count() const = 0;

  /**
   * Call get_child and return a new allocated copy of the child. The caller is responsible to delete it.
   */
  virtual _Context *get_child_new(uint16 index) const = 0;

  /**
   * Dereference this and return a new allocated copy. The caller is responsible to delete it.
   */
  virtual _Context *dereference_new() const = 0;

  void commit() const { overlay_->commit(); }
  void rollback() const { overlay_->rollback(); }
  void patch_code(uint16 location, Atom value) const { overlay_->patch_code(location, value); }
  void unpatch_code(uint16 patch_index) const { overlay_->unpatch_code(patch_index); }
  uint16 get_last_patch_index() const { return overlay_->get_last_patch_index(); }

  void setAtomicResult(Atom a) const;
  void setTimestampResult(Timestamp t) const;
  void setDurationResult(std::chrono::microseconds d) const;

  /**
   * Patch the code with a VALUE_PTR to the values array.
   * \param a The Atom that is the head of the compound value.
   * \return The index in the value array of the head.
   */
  uint16 setCompoundResultHead(Atom a) const;

  void addCompoundResultPart(Atom a) const;

  void trace(std::ostream& out) const;
};
}


#endif
