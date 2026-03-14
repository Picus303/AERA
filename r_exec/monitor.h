

#ifndef monitor_h
#define monitor_h

#include "../submodules/CoreLibrary/CoreLibrary/utils.h"
#include "binding_map.h"
#include "factory.h"


namespace r_exec {

class MDLController;

class Monitor :
  public _Object {
protected:
  P<BindingMap> bindings_;
  P<Fact> target_; // f->g or f->p.
  P<r_code::Code> mk_rdx_; // The reduction which make the target.

  MDLController *controller_;

  Monitor(MDLController *controller,
    BindingMap *bindings,
    Fact *target,
    r_code::Code* mk_rdx);
public:
  bool is_alive() const;
  virtual bool reduce(_Fact *input) = 0;
};
}


#endif
