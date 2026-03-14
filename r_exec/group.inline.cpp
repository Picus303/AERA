

namespace r_exec {

inline Group::Group(r_code::Mem *m) : LObject(m), CriticalSection() {

  reset_ctrl_values();
  reset_stats();
  reset_decay_values();
}

inline Group::Group(r_code::SysObject *source) : LObject(source), CriticalSection() {

  reset_ctrl_values();
  reset_stats();
  reset_decay_values();
}

inline Group::~Group() {

  invalidate();
}

inline bool Group::invalidate() {

  if (LObject::invalidate())
    return true;

  // unregister from all groups it views.
  std::unordered_map<uint32, P<View> >::const_iterator gv;
  for (gv = group_views_.begin(); gv != group_views_.end(); ++gv) {

    ((Group *)gv->second->object_)->enter();
    ((Group *)gv->second->object_)->viewing_groups_.erase(this);
    ((Group *)gv->second->object_)->leave();
  }
  /* We keep the group intact: the only thing is now the group will not be updated anymore.
          // remove all views that are hosted by this group.
          FOR_ALL_VIEWS_BEGIN(this,v)

              v->second->object->acq_views();
              v->second->object->views_.erase(v->second); // delete view from object's views.
              v->second->object->rel_views();

          FOR_ALL_VIEWS_END

          notification_views_.clear();
          ipgm_views_.clear();
          anti_ipgm_views_.clear();
          input_less_ipgm_views_.clear();
          other_views_.clear();
          group_views_.clear();
  */
  return false;
}

inline uint32 Group::get_upr() const {

  return (uint32)code(GRP_UPR).asFloat();
}

inline float32 Group::get_sln_thr() const {

  return code(GRP_SLN_THR).asFloat();
}

inline float32 Group::get_act_thr() const {

  return code(GRP_ACT_THR).asFloat();
}

inline float32 Group::get_vis_thr() const {

  return code(GRP_VIS_THR).asFloat();
}

inline float32 Group::get_c_sln_thr() const {

  return code(GRP_C_SLN_THR).asFloat();
}

inline float32 Group::get_c_act_thr() const {

  return code(GRP_C_ACT_THR).asFloat();
}

inline float32 Group::get_c_sln() const {

  return code(GRP_C_SLN).asFloat();
}

inline float32 Group::get_c_act() const {

  return code(GRP_C_ACT).asFloat();
}

inline void Group::mod_sln_thr(float32 value) {

  ++sln_thr_changes_;
  acc_sln_thr_ += value;
}

inline void Group::set_sln_thr(float32 value) {

  ++sln_thr_changes_;
  acc_sln_thr_ += value - get_sln_thr();
}

inline void Group::mod_act_thr(float32 value) {

  ++act_thr_changes_;
  acc_act_thr_ += value;
}

inline void Group::set_act_thr(float32 value) {

  ++act_thr_changes_;
  acc_act_thr_ += value - get_act_thr();
}

inline void Group::mod_vis_thr(float32 value) {

  ++vis_thr_changes_;
  acc_vis_thr_ += value;
}

inline void Group::set_vis_thr(float32 value) {

  ++vis_thr_changes_;
  acc_vis_thr_ += value - get_vis_thr();
}

inline void Group::mod_c_sln(float32 value) {

  ++c_sln_changes_;
  acc_c_sln_ += value;
}

inline void Group::set_c_sln(float32 value) {

  ++c_sln_changes_;
  acc_c_sln_ += value - get_c_sln();
}

inline void Group::mod_c_act(float32 value) {

  ++c_act_changes_;
  acc_c_act_ += value;
}

inline void Group::set_c_act(float32 value) {

  ++c_act_changes_;
  acc_c_act_ += value - get_c_act();
}

inline void Group::mod_c_sln_thr(float32 value) {

  ++c_sln_thr_changes_;
  acc_c_sln_thr_ += value;
}

inline void Group::set_c_sln_thr(float32 value) {

  ++c_sln_thr_changes_;
  acc_c_sln_thr_ += value - get_c_sln_thr();
}

inline void Group::mod_c_act_thr(float32 value) {

  ++c_act_thr_changes_;
  acc_c_act_thr_ += value;
}

inline void Group::set_c_act_thr(float32 value) {

  ++c_act_thr_changes_;
  acc_c_act_thr_ += value - get_c_act_thr();
}

inline float32 Group::get_sln_chg_thr() {

  return code(GRP_SLN_CHG_THR).asFloat();
}

inline float32 Group::get_sln_chg_prd() {

  return code(GRP_SLN_CHG_PRD).asFloat();
}

inline float32 Group::get_act_chg_thr() {

  return code(GRP_ACT_CHG_THR).asFloat();
}

inline float32 Group::get_act_chg_prd() {

  return code(GRP_ACT_CHG_PRD).asFloat();
}

inline float32 Group::get_avg_sln() {

  return code(GRP_AVG_SLN).asFloat();
}

inline float32 Group::get_high_sln() {

  return code(GRP_HIGH_SLN).asFloat();
}

inline float32 Group::get_low_sln() {

  return code(GRP_LOW_SLN).asFloat();
}

inline float32 Group::get_avg_act() {

  return code(GRP_AVG_ACT).asFloat();
}

inline float32 Group::get_high_act() {

  return code(GRP_HIGH_ACT).asFloat();
}

inline float32 Group::get_low_act() {

  return code(GRP_LOW_ACT).asFloat();
}

inline float32 Group::get_high_sln_thr() {

  return code(GRP_HIGH_SLN_THR).asFloat();
}

inline float32 Group::get_low_sln_thr() {

  return code(GRP_LOW_SLN_THR).asFloat();
}

inline float32 Group::get_sln_ntf_prd() {

  return code(GRP_SLN_NTF_PRD).asFloat();
}

inline float32 Group::get_high_act_thr() {

  return code(GRP_HIGH_ACT_THR).asFloat();
}

inline float32 Group::get_low_act_thr() {

  return code(GRP_LOW_ACT_THR).asFloat();
}

inline float32 Group::get_act_ntf_prd() {

  return code(GRP_ACT_NTF_PRD).asFloat();
}

inline float32 Group::get_low_res_thr() {

  return code(GRP_LOW_RES_THR).asFloat();
}

inline float32 Group::get_ntf_new() {

  return code(GRP_NTF_NEW).asFloat();
}

inline uint16 Group::get_ntf_grp_count() {

  return code(code(GRP_NTF_GRPS).asIndex()).getAtomCount();
}

inline Group *Group::get_ntf_grp(uint16 i) {

  if (code(code(GRP_NTF_GRPS).asIndex() + i).readsAsNil())
    return this;

  uint16 index = code(code(GRP_NTF_GRPS).asIndex() + i).asIndex();
  return (Group *)get_reference(index);
}

inline void Group::_mod_0_positive(uint16 member_index, float32 value) {

  float32 v = code(member_index).asFloat() + value;
  if (v < 0)
    v = 0;
  code(member_index) = Atom::Float(v);
}

inline void Group::_mod_0_plus1(uint16 member_index, float32 value) {

  float32 v = code(member_index).asFloat() + value;
  if (v < 0)
    v = 0;
  else if (v > 1)
    v = 1;
  code(member_index) = Atom::Float(v);
}

inline void Group::_mod_minus1_plus1(uint16 member_index, float32 value) {

  float32 v = code(member_index).asFloat() + value;
  if (v < -1)
    v = -1;
  else if (v > 1)
    v = 1;
  code(member_index) = Atom::Float(v);
}

inline void Group::_set_0_positive(uint16 member_index, float32 value) {

  if (value < 0)
    code(member_index) = Atom::Float(0);
  else
    code(member_index) = Atom::Float(value);
}

inline void Group::_set_0_plus1(uint16 member_index, float32 value) {

  if (value < 0)
    code(member_index) = Atom::Float(0);
  else if (value > 1)
    code(member_index) = Atom::Float(1);
  else
    code(member_index) = Atom::Float(value);
}

inline void Group::_set_minus1_plus1(uint16 member_index, float32 value) {

  if (value < -1)
    code(member_index) = Atom::Float(-1);
  else if (value > 1)
    code(member_index) = Atom::Float(1);
  else
    code(member_index) = Atom::Float(value);
}

inline void Group::_set_0_1(uint16 member_index, float32 value) {

  if (value == 0 || value == 1)
    code(member_index) = Atom::Float(value);
}

inline void Group::mod(uint16 member_index, float32 value) {

  switch (member_index) {
  case GRP_UPR:
  case GRP_DCY_PRD:
  case GRP_SLN_CHG_PRD:
  case GRP_ACT_CHG_PRD:
  case GRP_SLN_NTF_PRD:
  case GRP_ACT_NTF_PRD:
    _mod_0_positive(member_index, value);
    return;
  case GRP_SLN_THR:
    mod_sln_thr(value);
    return;
  case GRP_ACT_THR:
    mod_act_thr(value);
    return;
  case GRP_VIS_THR:
    mod_vis_thr(value);
    return;
  case GRP_C_SLN:
    mod_c_sln(value);
    return;
  case GRP_C_SLN_THR:
    mod_c_sln_thr(value);
    return;
  case GRP_C_ACT:
    mod_c_act(value);
    return;
  case GRP_C_ACT_THR:
    mod_c_act_thr(value);
    return;
  case GRP_DCY_PER:
    _mod_minus1_plus1(member_index, value);
    return;
  case GRP_SLN_CHG_THR:
  case GRP_ACT_CHG_THR:
  case GRP_HIGH_SLN_THR:
  case GRP_LOW_SLN_THR:
  case GRP_HIGH_ACT_THR:
  case GRP_LOW_ACT_THR:
  case GRP_LOW_RES_THR:
    _mod_0_plus1(member_index, value);
    return;
  }
}

inline void Group::set(uint16 member_index, float32 value) {

  switch (member_index) {
  case GRP_UPR:
  case GRP_DCY_PRD:
  case GRP_SLN_CHG_PRD:
  case GRP_ACT_CHG_PRD:
  case GRP_SLN_NTF_PRD:
  case GRP_ACT_NTF_PRD:
    _set_0_positive(member_index, value);
    return;
  case GRP_SLN_THR:
    set_sln_thr(value);
    return;
  case GRP_ACT_THR:
    set_act_thr(value);
    return;
  case GRP_VIS_THR:
    set_vis_thr(value);
    return;
  case GRP_C_SLN:
    set_c_sln(value);
    return;
  case GRP_C_SLN_THR:
    set_c_sln_thr(value);
    return;
  case GRP_C_ACT:
    set_c_act(value);
    return;
  case GRP_C_ACT_THR:
    set_c_act_thr(value);
    return;
  case GRP_DCY_PER:
    _set_minus1_plus1(member_index, value);
    return;
  case GRP_SLN_CHG_THR:
  case GRP_ACT_CHG_THR:
  case GRP_HIGH_SLN_THR:
  case GRP_LOW_SLN_THR:
  case GRP_HIGH_ACT_THR:
  case GRP_LOW_ACT_THR:
  case GRP_LOW_RES_THR:
    _set_0_plus1(member_index, value);
    return;
  case GRP_NTF_NEW:
  case GRP_DCY_TGT:
  case GRP_DCY_AUTO:
    _set_0_1(member_index, value);
    return;
  }
}
}
