

#ifndef controller_h
#define controller_h

#include "../core/base.h"
#include "../core/utils.h"
#include "../r_code/object.h"
#include "runtime/mem_output.h"
#include "dll.h"
#include "reduction_job.h"

namespace r_exec {

class Overlay;
class View;

// Upon invocation of take_input() the overlays older than tsc are killed, assuming stc>0; otherwise, overlays live unitl the ipgm dies.
// Controllers are built at loading time and at the view's injection time.
// Derived classes must expose a function: void reduce(r_code::_View*input); (called by reduction jobs).
class r_exec_dll Controller :
  public _Object {
protected:
  volatile uint32 invalidated_; // 32 bit alignment.
  volatile uint32 activated_; // 32 bit alignment.

  std::chrono::microseconds time_scope_;

  r_code::_View* view_;

  CriticalSection reductionCS_;

  virtual void take_input(r_exec::View* /* input */) {}
  template<class C> void __take_input(r_exec::View* input) { // utility: to be called by sub-classes.

    ReductionJob<C>* j = new ReductionJob<C>(input, (C*)this);
#ifdef WITH_DETAIL_OID
    OUTPUT_LINE((TraceLevel)0, "  make ReductionJob " << j->get_job_id() <<
      "(" << j->get_detail_oid() << "): controller(" << get_detail_oid() << ")->reduce(View(fact_" <<
      input->object_->get_oid() << ")) for " << get_core_object()->get_oid());
#endif
    push_reduction_job(j);
  }

  static void push_reduction_job(_ReductionJob* j);

  Controller(r_code::_View* view);
public:
  virtual ~Controller();

  std::chrono::microseconds get_tsc() { return time_scope_; }

  virtual void invalidate() { invalidated_ = 1; }
  bool is_invalidated() { return invalidated_ == 1; };
  void activate(bool a) { activated_ = (a ? 1 : 0); }
  bool is_activated() const { return activated_ == 1; }
  bool is_alive() const { return invalidated_ == 0 && activated_ == 1; }

  virtual r_code::Code* get_core_object() const = 0;

  r_code::Code* get_object() const { return view_->object_; } // return the reduction object (e.g. ipgm, icpp_pgm, cst, mdl).
  r_exec::View* get_view() const { return (r_exec::View*)view_; } // return the reduction object's view.

  void _take_input(r_exec::View* input); // called by the rMem at update time and at injection time.

  virtual void gain_activation() { activate(true); }
  virtual void lose_activation() { activate(false); }

  void set_view(View* view);

  void debug(View* /* input */) {}
};

class r_exec_dll OController :
  public Controller {
protected:
  r_code::list<P<Overlay> > overlays_;

  OController(r_code::_View* view);
public:
  virtual ~OController();
};

}

#endif
