

#ifndef view_h
#define view_h

#include "../r_code/object.h"
#include "dll.h"


namespace r_exec {

class Group;
class LObject;
class Controller;

// OID is hidden at code_[VIEW_OID].
// Shared resources:
// none: all mod/set operations are pushed on the group and executed at update time.
class r_exec_dll View :
  public r_code::_View {
private:
  static uint32 lastOID_;
  static uint32 GetOID();

  // Ctrl values.
  uint32 sln_changes_;
  float32 acc_sln_;
  uint32 act_changes_;
  float32 acc_act_;
  uint32 vis_changes_;
  float32 acc_vis_;
  uint32 res_changes_;
  float32 acc_res_;
  void reset_ctrl_values();

  // Monitoring
  float32 initial_sln_;
  float32 initial_act_;

  void init(r_code::_View::SyncMode sync,
    Timestamp ijt,
    float32 sln,
    int32 res,
    r_code::Code *host,
    r_code::Code *origin,
    r_code::Code *object);
protected:
  void reset_init_sln();
  void reset_init_act();
public:
  static uint16 ViewOpcode_;

  P<Controller> controller_; // built upon injection of the view (if the object is an ipgm/icpp_pgm/cst/mdl).

  static float32 MorphValue(float32 value, float32 source_thr, float32 destination_thr);
  static float32 MorphChange(float32 change, float32 source_thr, float32 destination_thr);

  uint32 periods_at_low_sln_;
  uint32 periods_at_high_sln_;
  uint32 periods_at_low_act_;
  uint32 periods_at_high_act_;

  View();
  View(r_code::SysView *source, r_code::Code *object);
  View(View *view, Group *group); // copy the view and assigns it to the group (used for cov); morph ctrl values.
  View(const View *view, bool new_OID = false); // simple copy.
  View(r_code::_View::SyncMode sync,
    Timestamp ijt,
    float32 sln,
    int32 res,
    r_code::Code *host,
    r_code::Code *origin,
    r_code::Code *object); // regular view; res set to -1 means forever.
  View(r_code::_View::SyncMode sync,
    Timestamp ijt,
    float32 sln,
    int32 res,
    r_code::Code *host,
    r_code::Code *origin,
    r_code::Code *object,
    float32 act); // pgm/mdl view; res set to -1 means forever.
  ~View();

  void reset();
  void set_object(r_code::Code *object);

  uint32 get_oid() const;

  virtual bool is_notification() const;

  Group *get_host();

  r_code::_View::SyncMode get_sync();
  float32 get_res();
  float32 get_sln();
  float32 get_act();
  bool get_cov();
  float32 get_vis();
  uint32 &ctrl0() { return code_[VIEW_CTRL_0].atom_; } // use only for non-group views.
  uint32 &ctrl1() { return code_[VIEW_CTRL_1].atom_; } // idem.

  void mod_res(float32 value);
  void set_res(float32 value);
  void mod_sln(float32 value);
  void set_sln(float32 value);
  void mod_act(float32 value);
  void set_act(float32 value);
  void mod_vis(float32 value);
  void set_vis(float32 value);

  float32 update_res();
  float32 update_sln(float32 low, float32 high);
  float32 update_act(float32 low, float32 high);
  float32 update_vis();

  float32 update_sln_delta();
  float32 update_act_delta();

  void force_res(float32 value); // unmediated.

  // Target res, sln, act, vis.
  void mod(uint16 member_index, float32 value);
  void set(uint16 member_index, float32 value);

  void delete_from_object();
  void delete_from_group();
};

class r_exec_dll NotificationView :
  public View {
public:
  NotificationView(r_code::Code *origin, r_code::Code *destination, r_code::Code *marker); // res=1, sln=1.

  bool is_notification() const override;
};
}


#include "view.inline.cpp"


#endif
