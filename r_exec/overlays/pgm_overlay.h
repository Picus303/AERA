

#ifndef pgm_overlay_h
#define pgm_overlay_h

#include "overlay.h"


namespace r_exec {

class PGMController;
class InputLessPGMController;
class IPGMContext;

// Overlays for input-less programs.
// Base class for other programs (with inputs, anti-programs).
class r_exec_dll InputLessPGMOverlay :
  public Overlay {
  friend class PGMController;
  friend class InputLessPGMController;
  friend class IPGMContext;
protected:
  std::vector<P<r_code::Code> > productions_; // receives the results of ins, inj and eje; views are retrieved (fvw) or built (reduction) in the value array.

  /**
   * Create an IPGMContext and evaluate code_ at index.
   * \param index The index in code_ to evaluate.
   * \return True if successfully evaluated, false for problem evaluated including
   * unbound variables. Note that if the code at index is a boolean expression,
   * then this can return true for successful evaluation even though the expression value
   * is boolean false.
   */
  bool evaluate(uint16 index);

  virtual r_code::Code *get_mk_rdx(uint16 &extent_index) const;

  void patch_tpl_args(); // no views in tpl args; patches the ptn skeleton's first atom with IPGM_PTR with an index in the ipgm arg set; patches wildcards with similar IPGM_PTRs.
  void patch_tpl_code(uint16 pgm_code_index, uint16 ipgm_code_index); // to recurse.
  virtual void patch_input_code(uint16 pgm_code_index, uint16 input_index, uint16 input_code_index, int16 parent_index = -1); // defined in PGMOverlay.

  InputLessPGMOverlay();
  InputLessPGMOverlay(Controller *c);
public:
  virtual ~InputLessPGMOverlay();

  void reset() override; // reset to original state (pristine copy of the pgm code and empty value set).

  bool inject_productions(); // return true upon successful evaluation; no existence check in simulation mode.
};

// Overlay with inputs.
// Several ReductionCores can attempt to reduce the same overlay simultaneously (each with a different input).
class r_exec_dll PGMOverlay :
  public InputLessPGMOverlay {
  friend class PGMController;
  friend class IPGMContext;
private:
  bool is_volatile_;
  Timestamp birth_time_; // used for ipgms: overlays older than ipgm->tsc are killed; birth_time set to the time of the first match, 0 if no match occurred.
protected:
  r_code::list<uint16> input_pattern_indices_; // stores the input patterns still waiting for a match: will be plucked upon each successful match.
  std::vector<P<r_code::_View> > input_views_; // copies of the inputs; vector updated at each successful match.

  typedef enum {
    SUCCESS = 0,
    FAILURE = 1,
    IMPOSSIBLE = 3 // when the input's class does not even match the object class in the pattern's skeleton.
  }MatchResult;

  MatchResult match(r_exec::View *input, uint16 &input_index); // delegates to _match; input_index is set to the index of the pattern that matched the input.
  bool check_guards(); // return true upon successful evaluation.

  MatchResult _match(r_exec::View *input, uint16 pattern_index); // delegates to __match.

  /**
   * \return SUCCESS upon a successful match, IMPOSSIBLE if the input is not of the right class, 
   * FAILURE otherwise.
   */
  MatchResult __match(r_exec::View *input, uint16 pattern_index);

  r_code::Code *dereference_in_ptr(Atom a);
  void patch_input_code(uint16 pgm_code_index, uint16 input_index, uint16 input_code_index, int16 parent_index = -1) override;

  r_code::Code *get_mk_rdx(uint16 &extent_index) const override;

  void init();

  PGMOverlay(Controller *c);
  PGMOverlay(PGMOverlay *original, uint16 last_input_index, uint16 value_commit_index); // copy from the original and rollback.
public:
  virtual ~PGMOverlay();

  void reset() override {
    InputLessPGMOverlay::reset();
    patch_indices_.clear();
    input_views_.clear();
    input_pattern_indices_.clear();
    init();
  }

  Overlay *reduce(r_exec::View *input) override; // called upon the processing of a reduction job.

  r_code::Code *getInputObject(uint16 i) const;
  r_code::_View *getInputView(uint16 i) const;

  Timestamp get_birth_time() const { return birth_time_; }

  bool is_invalidated() override;
};

// Several ReductionCores can attempt to reduce the same overlay simultaneously (each with a different input).
// In addition, ReductionCores and signalling jobs can attempt to inject productions concurrently.
// Usues the same mk.rdx as for InputLessPGMOverlays.
class r_exec_dll AntiPGMOverlay :
  public PGMOverlay {
  friend class AntiPGMController;
private:
  AntiPGMOverlay(Controller *c) : PGMOverlay(c) {}
  AntiPGMOverlay(AntiPGMOverlay *original, uint16 last_input_index, uint16 value_limit)
  : PGMOverlay(original, last_input_index, value_limit) {}
public:
  ~AntiPGMOverlay();

  Overlay *reduce(r_exec::View *input) override; // called upon the processing of a reduction job.
};
}


#include "pgm_overlay.inline.cpp"


#endif
