

#ifndef pgm_controller_h
#define pgm_controller_h

#include "controller.h"
#include "pgm_overlay.h"


namespace r_exec {

class r_exec_dll _PGMController :
  public OController {
protected:
  bool run_once_;

  _PGMController(r_code::_View *ipgm_view);
  virtual ~_PGMController();
public:
  r_code::Code *get_core_object() const override { return get_object()->get_reference(0); }
};

// TimeCores holding InputLessPGMSignalingJob trigger the injection of the productions.
// No overlays.
class r_exec_dll InputLessPGMController :
  public _PGMController {
public:
  InputLessPGMController(r_code::_View *ipgm_view);
  ~InputLessPGMController();

  void signal_input_less_pgm();
};

// Controller for programs with inputs.
class r_exec_dll PGMController :
  public _PGMController {
public:
  PGMController(r_code::_View *ipgm_view);
  virtual ~PGMController();

  void take_input(r_exec::View *input) override;
  void reduce(r_exec::View *input);

  void notify_reduction();
};

// Signaled by TimeCores (holding AntiPGMSignalingJob).
// Possible recursive locks: signal_anti_pgm()->overlay->inject_productions()->mem->inject()->injectNow()->inject_reduction_jobs()->overlay->take_input().
class r_exec_dll AntiPGMController :
  public _PGMController {
private:
  bool successful_match_;

  void push_new_signaling_job();
public:
  AntiPGMController(r_code::_View *ipgm_view);
  ~AntiPGMController();

  void take_input(r_exec::View *input) override;
  void reduce(r_exec::View *input);
  void signal_anti_pgm();

  void restart();
};
}

#endif
