

#ifndef model_base_h
#define model_base_h

#include "construction/factory.h"


namespace r_exec {

class _Mem;

// TPX guess models: this list is meant for TPXs to (a) avoid re-guessing known failed models and,
// (b) avoid producing the same models in case they run concurrently.
// The black list contains bad models (models that were killed). This list is trimmed down on a time basis (black_thz) by the garbage collector.
// Each bad model is tagged with the last time it was successfully compared to. GC is performed by comparing this time to the thz.
// The white list contains models that are still alive and is trimmed down when models time out.
// Models are packed before insertion in the white list.
class ModelBase {
  friend class _Mem;
private:
  static ModelBase *singleton_;

  CriticalSection mdlCS_;

  std::chrono::microseconds thz_;

  class MEntry {
  private:
    static bool Match(r_code::Code *lhs, r_code::Code *rhs);
    static uint32 _ComputeHashCode(_Fact *component); // use for lhs/rhs.
  public:
    static uint32 ComputeHashCode(r_code::Code *mdl, bool packed);

    MEntry();
    MEntry(r_code::Code *mdl, bool packed);

    P<r_code::Code> mdl_;
    bool packed_;
    Timestamp touch_time_; // last time the mdl was successfully compared to.
    uint32 hash_code_;

    bool match(const MEntry &e) const;

    class Hash {
    public:
      size_t operator ()(const MEntry& e) const { return e.hash_code_; }
    };

    class Equal {
    public:
      bool operator ()(const MEntry& lhs, const MEntry& rhs) const { return lhs.match(rhs); }
    };
  };

  typedef std::unordered_set<MEntry, typename MEntry::Hash, typename MEntry::Equal> MdlSet;

  MdlSet black_list_; // mdls are already packed when inserted (they come from the white list).
  MdlSet white_list_; // mdls are packed just before insertion.

  void set_thz(std::chrono::microseconds thz) { thz_ = thz; } // called by _Mem::start(); set to secondary_thz.
  void trim_objects(); // called by _Mem::GC().

  ModelBase();
public:
  static ModelBase *Get();

  void load(r_code::Code *mdl); // called by _Mem::load(); models with no views go to the black_list_.
  void get_models(r_code::list<P<r_code::Code> > &models); // white_list_ first, black_list_ next.

  r_code::Code *check_existence(r_code::Code *mdl); // caveat: mdl is unpacked; return (a) NULL if the model is in the black list, (b) a model in the white list if the mdl has been registered there or (c) the mdl itself if not in the model base, in which case the mdl is added to the white list.
  void check_existence(r_code::Code *m0, r_code::Code *m1, r_code::Code *&_m0, r_code::Code *&_m1); // m1 is a requirement on m0; _m0 and _m1 are the return values as defined above; m0 added only if m1 is not black listed.
  void register_mdl_failure(r_code::Code *mdl); // moves the mdl from the white to the black list; happens to bad models.
  void register_mdl_timeout(r_code::Code *mdl); // deletes the mdl from the white list; happen to models that have been unused for primary_thz.
};
}


#endif
