

#include "pgm_controller.h"
#include "mem.h"

using namespace r_code;

namespace r_exec {

_PGMController::_PGMController(_View *ipgm_view) : OController(ipgm_view) {

  run_once_ = !ipgm_view->object_->code(IPGM_RUN).asBoolean();
}

_PGMController::~_PGMController() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InputLessPGMController::InputLessPGMController(_View *ipgm_view) : _PGMController(ipgm_view) {

  overlays_.push_back(new InputLessPGMOverlay(this));
}

InputLessPGMController::~InputLessPGMController() {
}

void InputLessPGMController::signal_input_less_pgm() { // next job will be pushed by the rMem upon processing the current signaling job, i.e. right after exiting this function.

  reductionCS_.enter();
  if (overlays_.size()) {

    InputLessPGMOverlay *overlay = (InputLessPGMOverlay *)overlays_.front();
    overlay->inject_productions();
    overlay->reset();

    if (!run_once_) {

      if (is_alive()) {

        Group *host = get_view()->get_host();
        host->enter();
        if (host->get_c_act() > host->get_c_act_thr() && // c-active group.
          host->get_c_sln() > host->get_c_sln_thr()) { // c-salient group.

          host->leave();

          TimeJob *next_job = new InputLessPGMSignalingJob((r_exec::View*)view_, Now() + time_scope_);
          _Mem::Get()->push_time_job(next_job);
        } else
          host->leave();
      }
    }
  }
  reductionCS_.leave();

  if (run_once_)
    invalidate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PGMController::PGMController(_View *ipgm_view) : _PGMController(ipgm_view) {

  overlays_.push_back(new PGMOverlay(this));
}

PGMController::~PGMController() {
}

void PGMController::notify_reduction() {

  if (run_once_)
    invalidate();
}

void PGMController::take_input(r_exec::View *input) {

  Controller::__take_input<PGMController>(input);
}

void PGMController::reduce(r_exec::View *input) {

  r_code::list<P<Overlay> >::const_iterator o;
  uint32 oid = input->object_->get_oid();
  if (time_scope_.count() > 0) {

    reductionCS_.enter();
    auto now = Now(); // call must be located after the CS.enter() since (*o)->reduce() may update (*o)->birth_time.
    for (o = overlays_.begin(); o != overlays_.end();) {

      if ((*o)->is_invalidated())
        o = overlays_.erase(o);
      else {

        auto  birth_time = ((PGMOverlay *)*o)->get_birth_time();
        if (birth_time.time_since_epoch().count() > 0 && now - birth_time > time_scope_) {
          o = overlays_.erase(o);
        } else {
          Overlay *offspring = (*o)->reduce(input);
          if (offspring) {
            overlays_.push_front(offspring);
          }
          if (!is_alive())
            break;
          ++o;
        }
      }
    }
    reductionCS_.leave();
  } else {

    reductionCS_.enter();
    for (o = overlays_.begin(); o != overlays_.end();) {

      if ((*o)->is_invalidated())
        o = overlays_.erase(o);
      else {

        Overlay *offspring = (*o)->reduce(input);
        if (offspring)
          overlays_.push_front(offspring);
        if (!is_alive())
          break;
        ++o;
      }

    }
    reductionCS_.leave();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AntiPGMController::AntiPGMController(_View *ipgm_view) : _PGMController(ipgm_view), successful_match_(false) {

  overlays_.push_back(new AntiPGMOverlay(this));
}

AntiPGMController::~AntiPGMController() {
}

void AntiPGMController::take_input(r_exec::View *input) {

  Controller::__take_input<AntiPGMController>(input);
}

void AntiPGMController::reduce(r_exec::View *input) {

  reductionCS_.enter();
  r_code::list<P<Overlay> >::const_iterator o;
  for (o = overlays_.begin(); o != overlays_.end();) {

    if ((*o)->is_invalidated())
      o = overlays_.erase(o);
    else {

      Overlay *offspring = (*o)->reduce(input);
      if (successful_match_) { // the controller has been restarted: reset the overlay and kill all others

        Overlay *overlay = *o;
        overlay->reset();
        overlays_.clear();
        overlays_.push_back(overlay);
        successful_match_ = false;
        break;
      }
      ++o;
      if (offspring)
        overlays_.push_front(offspring);
    }
  }
  reductionCS_.leave();
}

void AntiPGMController::signal_anti_pgm() {

  reductionCS_.enter();
  if (successful_match_) // a signaling job has been spawn in restart(): we are here in an old job during which a positive match occurred: do nothing.
    successful_match_ = false;
  else { // no positive match during this job: inject productions and restart.

    Overlay *overlay = overlays_.front();
    ((AntiPGMOverlay *)overlay)->inject_productions();
    overlay->reset(); // reset the first overlay and kill all others.
    if (!run_once_ && is_alive()) {

      overlays_.clear();
      overlays_.push_back(overlay);
    }
  }
  reductionCS_.leave();

  if (run_once_)
    invalidate();
}

void AntiPGMController::restart() { // one anti overlay matched all its inputs, timings and guards.

  push_new_signaling_job();
  successful_match_ = true;
}

void AntiPGMController::push_new_signaling_job() {

  Group *host = get_view()->get_host();
  host->enter();
  if (get_view()->get_act() > host->get_act_thr() && // active ipgm.
    host->get_c_act() > host->get_c_act_thr() && // c-active group.
    host->get_c_sln() > host->get_c_sln_thr()) { // c-salient group.

    host->leave();
    TimeJob *next_job = new AntiPGMSignalingJob((r_exec::View*)view_, Now() + Utils::GetDuration<Code>(get_object(), IPGM_TSC));
    _Mem::Get()->push_time_job(next_job);
  } else
    host->leave();
}
}
