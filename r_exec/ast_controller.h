

#ifndef ast_controller_h
#define ast_controller_h

#include "overlay.h"
#include "factory.h"
#include "pattern_extractor.h"


namespace r_exec {

// Atomic state controller. Attached to a null-pgm and monitoring to a (repeated) input fact (SYNC_PERIODIC or SYNC_HOLD).
// Has a resilience of 2 times the upr of the group its target comes from.
// Upon catching a counter-evidence, signal the TPX and kill the object (i.e. invalidate and kill views); this will kill the controller and TPX.
// Catching a predicted evidence means that there is a model that predicts the next value of the object: kill the CTPX.
// AST live in primary groups and take their inputs therefrom: these are filtered by the A/F WRT goals/predictions.
// There is no control over AST: instead, computation is minimal (just pattern-matching) and CTPX are killed asap whenever a model predicts a value change.
// There cannot be any control based on the semantics of the inputs as these are atomic and henceforth no icst is available at injection time.
template<class U> class ASTController :
  public OController {
protected:
  P<CTPX> tpx_;
  P<_Fact> target_; // the repeated fact to be monitored.
  Timestamp thz_timestamp_; // time horizon: if an input is caught with ijt<thz (meaning it's too old), discard it.

  void kill();

  ASTController(AutoFocusController *auto_focus, View *target);
public:
  virtual ~ASTController();

  r_code::Code *get_core_object() const override { return get_object(); }

  void take_input(r_exec::View *input) override;
  void reduce(View *input);
};

// For SYNC_PERIODIC targets.
class PASTController :
  public ASTController<PASTController> {
public:
  PASTController(AutoFocusController *auto_focus, View *target);
  ~PASTController();

  void reduce(View *input) { this->ASTController<PASTController>::reduce(input); }
  void reduce(View *v, _Fact *input);
};

// For SYNC_HOLD targets.
class HASTController :
  public ASTController<HASTController> {
private:
  P<_Fact> source_; // to be invalidated if a counter-evidence is found.
public:
  HASTController(AutoFocusController *auto_focus, View *target, _Fact *source);
  ~HASTController();

  void reduce(View *input) { this->ASTController<HASTController>::reduce(input); }
  void reduce(View *v, _Fact *input);
};
}


#include "ast_controller.tpl.cpp"


#endif
