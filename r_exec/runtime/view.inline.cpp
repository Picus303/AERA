

#include "../../r_code/utils.h"
#include "metadata/opcodes.h"


namespace r_exec {

using r_code::Atom;

inline View::View() : r_code::_View(), controller_(NULL) {

  code_[VIEW_OID].atom_ = GetOID();
  reset_ctrl_values();
}

inline View::View(r_code::SysView *source, r_code::Code *object) : r_code::_View(source, object), controller_(NULL) {

  code_[VIEW_OID].atom_ = GetOID();
  reset();
}

inline View::View(const View *view, bool new_OID) : r_code::_View(), controller_(NULL) {

  object_ = view->object_;
  memcpy(code_, view->code_, VIEW_CODE_MAX_SIZE * sizeof(Atom) + 2 * sizeof(r_code::Code *)); // reference_set is contiguous to code; memcpy in one go.
  if (new_OID)
    code_[VIEW_OID].atom_ = GetOID();
  controller_ = NULL; // deprecated: controller=view->controller;
  reset();
}

inline View::View(SyncMode sync,
  Timestamp ijt,
  float32 sln,
  int32 res,
  r_code::Code *destination,
  r_code::Code *origin,
  r_code::Code *object) : r_code::_View(), controller_(NULL) {

  code(VIEW_OPCODE) = Atom::SSet(Opcodes::View, VIEW_ARITY);
  init(sync, ijt, sln, res, destination, origin, object);
}

inline View::View(SyncMode sync,
  Timestamp ijt,
  float32 sln,
  int32 res,
  r_code::Code *destination,
  r_code::Code *origin,
  r_code::Code *object,
  float32 act) : r_code::_View(), controller_(NULL) {

  code(VIEW_OPCODE) = Atom::SSet(Opcodes::PgmView, PGM_VIEW_ARITY);
  init(sync, ijt, sln, res, destination, origin, object);
  code(VIEW_ACT) = Atom::Float(act);
}

inline void View::init(SyncMode sync,
  Timestamp ijt,
  float32 sln,
  int32 res,
  r_code::Code *destination,
  r_code::Code *origin,
  r_code::Code *object) {

  code_[VIEW_OID].atom_ = GetOID();
  reset_ctrl_values();

  code(VIEW_SYNC) = Atom::Float((float32)sync);
  code(VIEW_IJT) = Atom::IPointer(code(VIEW_OPCODE).getAtomCount() + 1);
  r_code::Utils::SetTimestamp<View>(this, VIEW_IJT, ijt);
  code(VIEW_SLN) = Atom::Float(sln);
  code(VIEW_RES) = res < 0 ? Atom::PlusInfinity() : Atom::Float((float32)res);
  code(VIEW_HOST) = Atom::RPointer(0);
  code(VIEW_ORG) = origin ? Atom::RPointer(1) : Atom::Nil();

  references_[0] = destination;
  references_[1] = origin;

  set_object(object);
}

inline void View::reset() {

  reset_ctrl_values();
  reset_init_sln();
  reset_init_act();
}

inline uint32 View::get_oid() const {

  return code_[VIEW_OID].atom_;
}

inline bool View::is_notification() const {

  return false;
}

inline Group *View::get_host() {

  uint32 host_reference = code(VIEW_HOST).asIndex();
  return (Group *)references_[host_reference];
}

inline View::SyncMode View::get_sync() {

  return (SyncMode)(uint32)code(VIEW_SYNC).asFloat();
}

inline float32 View::get_res() {

  return code(VIEW_RES).asFloat();
}

inline float32 View::get_sln() {

  return code(VIEW_SLN).asFloat();
}

inline float32 View::get_act() {

  return code(VIEW_ACT).asFloat();
}

inline float32 View::get_vis() {

  return code(GRP_VIEW_VIS).asFloat();
}

inline bool View::get_cov() {

  if (object_->code(0).getDescriptor() == Atom::GROUP)
    return code(GRP_VIEW_COV).asBoolean();
  return false;
}

inline void View::mod_res(float32 value) {

  if (code(VIEW_RES) == Atom::PlusInfinity())
    return;
  acc_res_ += value;
  ++res_changes_;
}

inline void View::set_res(float32 value) {

  if (code(VIEW_RES) == Atom::PlusInfinity())
    return;
  acc_res_ += value - get_res();
  ++res_changes_;
}

inline void View::mod_sln(float32 value) {

  acc_sln_ += value;
  ++sln_changes_;
}

inline void View::set_sln(float32 value) {

  acc_sln_ += value - get_sln();
  ++sln_changes_;
}

inline void View::mod_act(float32 value) {

  acc_act_ += value;
  ++act_changes_;
}

inline void View::set_act(float32 value) {

  acc_act_ += value - get_act();
  ++act_changes_;
}

inline void View::mod_vis(float32 value) {

  acc_vis_ += value;
  ++vis_changes_;
}

inline void View::set_vis(float32 value) {

  acc_vis_ += value - get_vis();
  ++vis_changes_;
}

inline float32 View::update_sln_delta() {

  float32 delta = get_sln() - initial_sln_;
  initial_sln_ = get_sln();
  return delta;
}

inline float32 View::update_act_delta() {

  float32 act = get_act();
  float32 delta = act - initial_act_;
  initial_act_ = act;
  return delta;
}

inline void View::force_res(float32 value) {

  code(VIEW_RES) = Atom::Float(value);
}

inline void View::mod(uint16 member_index, float32 value) {

  switch (member_index) {
  case VIEW_SLN:
    mod_sln(value);
    break;
  case VIEW_RES:
    mod_res(value);
    break;
  case VIEW_ACT:
    mod_act(value);
    break;
  case GRP_VIEW_VIS:
    mod_vis(value);
    break;
  }
}

inline void View::set(uint16 member_index, float32 value) {

  switch (member_index) {
  case VIEW_SLN:
    set_sln(value);
    break;
  case VIEW_RES:
    set_res(value);
    break;
  case VIEW_ACT:
    set_act(value);
    break;
  case GRP_VIEW_VIS:
    set_vis(value);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool NotificationView::is_notification() const {

  return true;
}
}
