

#ifndef overlay_h
#define overlay_h

#include "../submodules/CoreLibrary/CoreLibrary/base.h"
#include "../submodules/CoreLibrary/CoreLibrary/utils.h"
#include "../r_code/object.h"
#include "controller.h"
#include "reduction_job.h"
#include "dll.h"

namespace r_exec {

class _Context;
class IPGMContext;
class HLPContext;
class Controller;
class View;

class r_exec_dll Overlay :
  public _Object {
  friend class _Context;
  friend class IPGMContext;
  friend class HLPContext;
protected:
  volatile uint32 invalidated_;

  Controller *controller_;

  r_code::resized_vector<r_code::Atom> values_; // value array: stores the results of computations.
  // Copy of the pgm/hlp code. Will be patched during matching and evaluation:
  // any area indexed by a vl_ptr will be overwritten with:
  //   the evaluation result if it fits in a single atom,
  //   a ptr to the value array if the result is larger than a single atom,
  //   a ptr to an input if the result is a pattern input.
  r_code::Atom *code_;
  uint16 code_size_;
  std::vector<uint16> patch_indices_; // indices where patches are applied; used for rollbacks.
  uint16 value_commit_index_; // index of the last computed value_+1; used for rollbacks.

  void load_code();
  void patch_code(uint16 index, r_code::Atom value);
  uint16 get_last_patch_index();
  void unpatch_code(uint16 patch_index);

  void rollback(); // reset the overlay to the last commited state: unpatch code and values.
  void commit(); // empty the patch_indices_ and set value_commit_index_ to values.size().

  r_code::Code *get_core_object() const; // pgm, mdl, cst.

  Overlay();
  Overlay(Controller *c, bool load_code = true);
public:
  Overlay(size_t values_size);
  virtual ~Overlay();

  virtual void reset(); // reset to original state.
  virtual Overlay *reduce(r_exec::View *input); // returns an offspring in case of a match.

  void invalidate() { invalidated_ = 1; }
  virtual bool is_invalidated() { return invalidated_ == 1; }

  r_code::Code* get_object() const;
  r_exec::View* get_view() const;

  r_code::Code *build_object(r_code::Atom head) const;
  const r_code::Atom* values() const { return &values_[0]; }
};

/**
 * A DefeasibleValidity is an object this is attached to a defeasible prediction and copied
 * to each later prediction in forward chaining. If a new fact defeats the grounds of
 * the original prediction, then call invalidate() to invalidate the defeasible prediction
 * and all predictions which followed from it. (The is_invalidate() method of Pred checks its
 * set of DefeasibleValidity and invalidates the Pred if a DefeasibleValidity is invalidated.)
 */
class r_exec_dll DefeasibleValidity :
  public _Object {
public:
  /**
   * Create a DefeasibleValidity that is not invalidated.
   */
  DefeasibleValidity() : invalidated_(0) {}

  /**
   * Check if this is invalidated
   * \return True if this is invalidated.
   */
  bool is_invalidated() { return invalidated_ != 0; }

  /**
   * Set this to invalidated so that is_invalidated() returns true.
   */
  void invalidate() { invalidated_ = 1; }

private:
  volatile uint32 invalidated_; // 32 bit alignment.
};

template<class T> class CriticalSectionList {
public:
  CriticalSection CS_;
  r_code::list<T> list_;
};

}


#endif
