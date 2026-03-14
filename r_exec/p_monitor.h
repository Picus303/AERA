

#ifndef p_monitor_h
#define p_monitor_h

#include "monitor.h"


namespace r_exec {

class MDLController;

class PMonitor :
  public Monitor {
private:
  bool rate_failures_;
  _Fact *prediction_target_; // f1 as in f0->pred->f1->object.
public:
  PMonitor(MDLController *controller,
    BindingMap *bindings,
    Fact *prediction, // f0->pred->f1->object.
    r_code::Code* mk_rdx,
    bool rate_failures);
  ~PMonitor();

  bool reduce(_Fact *input) override;
  void update(Timestamp &next_target);
};
}


#endif
