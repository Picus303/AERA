

#ifndef auto_focus_h
#define auto_focus_h

#include "../r_code/time_buffer.h"

#include "overlay.h"
#include "group.h"
#include "pattern_extractor.h"
#include "mem.h"


namespace r_exec {

class r_exec_dll AutoFocusController :
  public Controller {
private:
  // icpp_pgm parameters.
  bool pass_through_;
  bool ctpx_on_;
  bool gtpx_on_;
  bool ptpx_on_;
  bool trace_injections_;
  bool decompile_models_;
  std::vector<Group *> output_groups_; // 1st is the primary, 2nd the secondary, followed by other groups if any.

  class Rating {
  public:
    uint32 evidences_;
    uint32 positive_evidences_;
    float32 success_rate_;
    float32 delta_success_rate_;

    static bool DeltaSuccessRate(float32 delta_success_rate) {

      return delta_success_rate > 0 && delta_success_rate < _Mem::Get()->get_tpx_dsr_thr();
    }

    Rating() : evidences_(0), positive_evidences_(0), success_rate_(0), delta_success_rate_(1) {}

    void add_evidence(bool success) {

      ++evidences_;
      if (success)
        ++positive_evidences_;
      delta_success_rate_ = success_rate_;
      success_rate_ = positive_evidences_ / evidences_;
      delta_success_rate_ = success_rate_ - delta_success_rate_;
    }
  };

  typedef std::unordered_map<P<_Fact>, P<TPX>, r_code::PHash<_Fact> > TPXMap;

  TPXMap goals_; // f->g->f->target.
  TPXMap predictions_; // f->p->f->target.

  typedef std::unordered_map<P<_Fact>, Rating, r_code::PHash<_Fact> > RatingMap;

  // entries are patterns, i.e. abstract targets.
  RatingMap goal_ratings_;
  RatingMap prediction_ratings_;

  static const uint32 CacheInitialSize = 128;
  static const uint32 CrossBufferInitialSize = 1024;

  r_code::time_buffer<CInput, CInput::IsInvalidated> cache_; // contains all inputs we don't know yet if they are relevant or not; thz==sampling period.
  r_code::time_buffer<Input, Input::IsInvalidated> cross_buffer_; // contains all relevant inputs.

  void notify(_Fact *target, View *input, TPXMap &map);
  void dispatch_pred_success(Success* success, TPXMap &map);
  void dispatch(View *input, _Fact *abstract_input, BindingMap *bm, bool &injected, TPXMap &map);
  void dispatch_no_inject(View *input, _Fact *abstract_input, BindingMap *bm, TPXMap &map);
  template<class T> TPX *build_tpx(_Fact *target, _Fact *pattern, BindingMap *bm, RatingMap &map, Fact *f_imdl, bool wr_enabled) {

    if (!gtpx_on_ && !ptpx_on_)
      return new TPX(this, target, pattern, bm);

    if (wr_enabled)
      return new TPX(this, target, pattern, bm);

    RatingMap::const_iterator r = map.find(pattern);
    if (r != map.end()) {

      if (Rating::DeltaSuccessRate(r->second.delta_success_rate_)) // target for which we don't see much improvement over time.
        return new TPX(this, target, pattern, bm);
      else
        return new T(this, target, pattern, bm, f_imdl);
    } else
      return new T(this, target, pattern, bm, f_imdl);
  }
public:
  AutoFocusController(r_code::_View *view);
  ~AutoFocusController();

  r_code::Code *get_core_object() const override;

  void take_input(r_exec::View *input) override;
  void reduce(r_exec::View *input);

  View *inject_input(View *input); // inject a filtered input into the output groups starting from 0; return the view injected in the primary group.
  void inject_input(View *input, uint32 start); // inject an unfiltered input into the output groups starting from start.
  void inject_input(View *input, _Fact *abstract_input, BindingMap *bm); // inject a filtered input into the output groups.
  void inject_hlps(const std::vector<P<r_code::Code> > &hlps) const; // called by TPX; hlp is a mdl or a cst.

  bool decompile_models() const { return decompile_models_; }
  bool gtpx_on() const { return gtpx_on_; }
  bool ptpx_on() const { return ptpx_on_; }
  Group *get_primary_group() const { return output_groups_[0]; }

  void copy_cross_buffer(r_code::list<Input> &destination);
  r_code::time_buffer<CInput, CInput::IsInvalidated> &get_cache() { return cache_; }
};
}


#endif
