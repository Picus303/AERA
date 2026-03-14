

#include "time_job.h"
#include "pgm_controller.h"
#include "runtime/mem.h"

using namespace std;
using namespace std::chrono;
using namespace r_code;

namespace r_exec {

uint32 TimeJob::job_count_ = 0;

TimeJob::TimeJob(Timestamp target_time) : _Object(), target_time_(target_time) {
  // Increment without thread lock. It's only for tracing.
  job_id_ = ++job_count_;
}

bool TimeJob::is_alive() const {

  return true;
}

void TimeJob::report(microseconds lag) const {

  std::cout << "> late generic: " << lag.count() << " us behind." << std::endl;
}

////////////////////////////////////////////////////////////

UpdateJob::UpdateJob(Group *g, Timestamp ijt) : TimeJob(ijt) {

  group_ = g;
}

bool UpdateJob::update(Timestamp &next_target) {

#ifdef WITH_DETAIL_OID
  OUTPUT_LINE((TraceLevel)0, Utils::RelativeTime(Now()) << " UpdateJob::TimeJob " << get_job_id() <<
    ": group_" << group_->get_oid() << "->update()");
#endif
  group_->update(target_time_);
  return true;
}

void UpdateJob::report(int64 lag) const {

  std::cout << "> late update: " << lag << " us behind." << std::endl;
}

////////////////////////////////////////////////////////////

SignalingJob::SignalingJob(View *v, Timestamp ijt) : TimeJob(ijt) {

  view_ = v;
}

bool SignalingJob::is_alive() const {

  return view_->controller_->is_alive();
}

////////////////////////////////////////////////////////////

AntiPGMSignalingJob::AntiPGMSignalingJob(View *v, Timestamp ijt) : SignalingJob(v, ijt) {
}

bool AntiPGMSignalingJob::update(Timestamp &next_target) {

#ifdef WITH_DETAIL_OID
  OUTPUT_LINE((TraceLevel)0, Utils::RelativeTime(Now()) << " AntiPGMSignalingJob::TimeJob " << get_job_id() <<
    ": controller(" << view_->controller_->get_detail_oid() << ")->signal_anti_pgm()");
#endif
  if (is_alive())
    ((AntiPGMController *)view_->controller_)->signal_anti_pgm();
  return true;
}

void AntiPGMSignalingJob::report(int64 lag) const {

  std::cout << "> late |pgm signaling: " << lag << " us behind." << std::endl;
}

////////////////////////////////////////////////////////////

InputLessPGMSignalingJob::InputLessPGMSignalingJob(View *v, Timestamp ijt) : SignalingJob(v, ijt) {
}

bool InputLessPGMSignalingJob::update(Timestamp &next_target) {

#ifdef WITH_DETAIL_OID
  OUTPUT_LINE((TraceLevel)0, Utils::RelativeTime(Now()) << " InputLessPGMSignalingJob::TimeJob " << get_job_id() <<
    ": controller(" << view_->controller_->get_detail_oid() << ")->signal_input_less_pgm()");
#endif
  if (is_alive())
    ((InputLessPGMController *)view_->controller_)->signal_input_less_pgm();
  return true;
}

void InputLessPGMSignalingJob::report(int64 lag) const {

  std::cout << "> late input-less pgm signaling: " << lag << " us behind." << std::endl;
}

////////////////////////////////////////////////////////////

InjectionJob::InjectionJob(View *v, Timestamp target_time, bool is_from_io_device) : TimeJob(target_time) {

  view_ = v;
  is_from_io_device_ = is_from_io_device;
}

bool InjectionJob::update(Timestamp &next_target) {

#ifdef WITH_DETAIL_OID
  OUTPUT_LINE((TraceLevel)0, Utils::RelativeTime(Now()) << " InjectionJob::TimeJob " << get_job_id() <<
    ": inject(View(fact(" << view_->object_->get_detail_oid() << ")))");
#endif
  _Mem::Get()->inject(view_);
  if (is_from_io_device_)
    // The view injection time may be different than now, so log it too.
    OUTPUT_LINE(IO_DEVICE_INJ_EJT, Utils::RelativeTime(Now()) << " I/O device inject " <<
      view_->object_->get_oid() << ", ijt " << Utils::RelativeTime(view_->get_ijt()));
  return true;
}

void InjectionJob::report(int64 lag) const {

  std::cout << "> late injection: " << lag << " us behind." << std::endl;
}

////////////////////////////////////////////////////////////

EInjectionJob::EInjectionJob(View *v, Timestamp ijt) : TimeJob(ijt) {

  view_ = v;
}

bool EInjectionJob::update(Timestamp &next_target) {

#ifdef WITH_DETAIL_OID
  OUTPUT_LINE((TraceLevel)0, Utils::RelativeTime(Now()) << " EInjectionJob::TimeJob " << get_job_id() <<
    ": inject_existing_object(View(fact(" << view_->object_->get_detail_oid() << ")))");
#endif
  _Mem::Get()->inject_existing_object(view_, view_->object_, view_->get_host());
  return true;
}

void EInjectionJob::report(int64 lag) const {

  std::cout << "> late injection: " << lag << " us behind." << std::endl;
}

////////////////////////////////////////////////////////////

SaliencyPropagationJob::SaliencyPropagationJob(Code *o, float32 sln_change, float32 source_sln_thr, Timestamp ijt) : TimeJob(ijt), sln_change_(sln_change), source_sln_thr_(source_sln_thr) {

  object_ = o;
}

bool SaliencyPropagationJob::update(Timestamp &next_target) {

#ifdef WITH_DETAIL_OID
  OUTPUT_LINE((TraceLevel)0, Utils::RelativeTime(Now()) << " SaliencyPropagationJob::TimeJob " << get_job_id() <<
    ": propagate_sln(fact(" << object_->get_detail_oid() << "))");
#endif
  if (!object_->is_invalidated())
    _Mem::Get()->propagate_sln(object_, sln_change_, source_sln_thr_);
  return true;
}

void SaliencyPropagationJob::report(int64 lag) const {

  std::cout << "> late sln propagation: " << lag << " us behind." << std::endl;
}

////////////////////////////////////////////////////////////

ShutdownTimeCore::ShutdownTimeCore() : TimeJob(Timestamp(seconds(0))) {
}

bool ShutdownTimeCore::update(Timestamp &next_target) {

  return false;
}

////////////////////////////////////////////////////////////

PerfSamplingJob::PerfSamplingJob(Timestamp start, microseconds period) : TimeJob(start), period_(period) {
}

bool PerfSamplingJob::update(Timestamp &next_target) {

#ifdef WITH_DETAIL_OID
  OUTPUT_LINE((TraceLevel)0, Utils::RelativeTime(Now()) << " PerfSamplingJob::TimeJob " << get_job_id() <<
    ": inject_perf_stats()");
#endif
  _Mem::Get()->inject_perf_stats();
  target_time_ += period_;
  next_target = target_time_;
  return true;
}

bool PerfSamplingJob::is_alive() const {

  return true;
}
}
