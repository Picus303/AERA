

#ifndef reduction_job_h
#define reduction_job_h

#include "../r_code/utils.h"
#include "object.h"
#include "mem_output.h"
#include "view.h"


namespace r_exec {

class r_exec_dll _ReductionJob :
  public _Object {
protected:
  _ReductionJob();
  void register_latency(Timestamp now);
public:
  Timestamp ijt_; // time of injection of the job in the pipe.
  virtual bool update(Timestamp now) = 0; // return false to shutdown the reduction core.
  virtual void debug() {}
  uint32 get_job_id() const { return job_id_; }
private:
  static uint32 job_count_;
  int job_id_;
};

template<class _P> class ReductionJob :
  public _ReductionJob {
public:
  P<View> input_;
  P<_P> processor_;
  ReductionJob(View *input, _P *processor) : _ReductionJob(), input_(input), processor_(processor) {}
  bool update(Timestamp now) override {

    register_latency(now);
#ifdef WITH_DETAIL_OID
    OUTPUT_LINE((TraceLevel)0, r_code::Utils::RelativeTime(now) << " ReductionJob " << get_job_id() <<
      ": controller(" << processor_->get_detail_oid() << ")->reduce(View(fact_" << 
      input_->object_->get_oid() << "))");
#endif
    processor_->reduce(input_);
    return true;
  }
  void debug() override {

    processor_->debug(input_);
  }
};

template<class _P, class T, class C> class BatchReductionJob :
  public _ReductionJob {
public:
  P<_P> processor_; // the controller that will process the job.
  P<T> trigger_; // the event that triggered the job.
  P<C> controller_; // the controller that produced the job.
  BatchReductionJob(_P *processor, T *trigger, C *controller) : _ReductionJob(), processor_(processor), trigger_(trigger), controller_(controller) {}
  bool update(Timestamp now) override {

    register_latency(now);
#ifdef WITH_DETAIL_OID
    OUTPUT_LINE((TraceLevel)0, r_code::Utils::RelativeTime(now) << " BatchReductionJob " << get_job_id() <<
      ": controller(" << controller_->get_detail_oid() << "), trigger fact(" << 
      trigger_->get_detail_oid() << ")");
#endif
    processor_->reduce_batch(trigger_, controller_);
    return true;
  }
};

class r_exec_dll ShutdownReductionCore :
  public _ReductionJob {
public:
  bool update(Timestamp now) override;
};

class r_exec_dll AsyncInjectionJob :
  public _ReductionJob {
public:
  P<View> input_;
  AsyncInjectionJob(View *input) : _ReductionJob(), input_(input) {}
  bool update(Timestamp now) override;
};
}


#endif
