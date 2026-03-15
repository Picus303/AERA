

#ifndef time_job_h
#define time_job_h

#include "../r_code/utils.h"
#include "init.h"
#include "runtime/group.h"
#include "pgm_overlay.h"


namespace r_exec {

class r_exec_dll TimeJob :
  public _Object {
protected:
  TimeJob(Timestamp target_time);
public:
  Timestamp target_time_; // absolute deadline; 0 means ASAP.
  virtual bool update(Timestamp &next_target) = 0; // next_target: absolute deadline; 0 means no more waiting; return false to shutdown the time core.
  virtual bool is_alive() const;
  virtual void report(std::chrono::microseconds lag) const;
  uint32 get_job_id() const { return job_id_; }

  /**
   * Compare P<TimeJob> based only on target_time_.
   */
  class Compare {
  public:
    bool
      operator()
      (const P<TimeJob>& x, const P<TimeJob>& y) const
    {
      return x->target_time_ < y->target_time_;
    }
  };

private:
  static uint32 job_count_;
  int job_id_;
};

class r_exec_dll UpdateJob :
  public TimeJob {
public:
  P<Group> group_;
  UpdateJob(Group *g, Timestamp ijt);
  bool update(Timestamp &next_target) override;
  void report(int64 lag) const;
};

class r_exec_dll SignalingJob :
  public TimeJob {
protected:
  SignalingJob(View *v, Timestamp ijt);
public:
  P<View> view_;
  bool is_alive() const override;
};

class r_exec_dll AntiPGMSignalingJob :
  public SignalingJob {
public:
  AntiPGMSignalingJob(View *v, Timestamp ijt);
  bool update(Timestamp &next_target) override;
  void report(int64 lag) const;
};

class r_exec_dll InputLessPGMSignalingJob :
  public SignalingJob {
public:
  InputLessPGMSignalingJob(View *v, Timestamp ijt);
  bool update(Timestamp &next_target) override;
  void report(int64 lag) const;
};

/**
 * InjectionJob extends TimeJob to inject a View at a later time.
 */
class r_exec_dll InjectionJob :
  public TimeJob {
public:
  P<View> view_;
  /**
   * Create an InjectionJob to call  _Mem::Get()->inject(view_) at the target_time.
   * \param v The View for calling inject.
   * \param target_time The target time for the TimeJob.
   * \param is_from_io_device True if this is called from inject_from_io_device().
   * This is only needed so that this will log the I/O device inject.
   */
  InjectionJob(View *v, Timestamp target_time, bool is_from_io_device);
  bool update(Timestamp &next_target) override;
  void report(int64 lag) const;

  bool is_from_io_device_;
};

class r_exec_dll EInjectionJob :
  public TimeJob {
public:
  P<View> view_;
  EInjectionJob(View *v, Timestamp ijt);
  bool update(Timestamp &next_target) override;
  void report(int64 lag) const;
};

class r_exec_dll SaliencyPropagationJob :
  public TimeJob {
public:
  P<r_code::Code> object_;
  float32 sln_change_;
  float32 source_sln_thr_;
  SaliencyPropagationJob(r_code::Code *o, float32 sln_change, float32 source_sln_thr, Timestamp ijt);
  bool update(Timestamp &next_target) override;
  void report(int64 lag) const;
};

class r_exec_dll ShutdownTimeCore :
  public TimeJob {
public:
  ShutdownTimeCore();
  bool update(Timestamp &next_target) override;
};

template<class M> class MonitoringJob :
  public TimeJob {
public:
  P<M> monitor_;
  MonitoringJob(M *monitor, Timestamp deadline) : TimeJob(deadline), monitor_(monitor) {
#ifdef WITH_DETAIL_OID
    OUTPUT_LINE((TraceLevel)0, "  make MonitoringJob::TimeJob " << get_job_id() <<
      "(" << get_detail_oid() << ") for monitor(" << monitor_->get_detail_oid() << "), deadline " <<
      r_code::Utils::RelativeTime(deadline));
#endif
  }
  bool update(Timestamp &next_target) override {

#ifdef WITH_DETAIL_OID
    OUTPUT_LINE((TraceLevel)0, r_code::Utils::RelativeTime(r_exec::Now()) << " MonitoringJob::TimeJob " << get_job_id() <<
      ": monitor(" << monitor_->get_detail_oid() << ")->update()");
#endif
    monitor_->update(next_target);
    return true;
  }
  bool is_alive() const override {

    return monitor_->is_alive();
  }
  void report(std::chrono::microseconds lag) const override {

    std::cout << "> late monitoring: " << lag.count() << " us behind." << std::endl;
  }
};

class r_exec_dll PerfSamplingJob :
  public TimeJob {
public:
  std::chrono::microseconds period_;
  PerfSamplingJob(Timestamp start, std::chrono::microseconds period);
  bool is_alive() const override;
  bool update(Timestamp &next_target) override;
};
}


#endif
