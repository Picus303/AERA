

#ifndef r_exec_object_h
#define r_exec_object_h

#include "../submodules/CoreLibrary/CoreLibrary/utils.h"
#include "../r_code/object.h"
#include "view.h"
#include "opcodes.h"

#include <list>


namespace r_exec {

static inline bool IsNotification(r_code::Code *object) {

  switch (object->code(0).getDescriptor()) {
  case r_code::Atom::MARKER:
    return object->code(0).asOpcode() == Opcodes::MkActChg ||
      object->code(0).asOpcode() == Opcodes::MkHighAct ||
      object->code(0).asOpcode() == Opcodes::MkHighSln ||
      object->code(0).asOpcode() == Opcodes::MkLowAct ||
      object->code(0).asOpcode() == Opcodes::MkLowRes ||
      object->code(0).asOpcode() == Opcodes::MkLowSln ||
      object->code(0).asOpcode() == Opcodes::MkNew ||
      object->code(0).asOpcode() == Opcodes::MkRdx ||
      object->code(0).asOpcode() == Opcodes::MkSlnChg;
  default:
    return false;
  }
}

// Shared resources:
// views: accessed by Mem::injectNow (via various sub calls) and Mem::update.
// psln_thr: accessed by reduction cores (via overlay mod/set).
// marker_set: accessed by Mem::injectNow ans Mem::_initiate_sln_propagation.
template<class C, class U> class Object :
  public C {
private:
  size_t hash_value_;

  volatile uint32 invalidated_; // must be aligned on 32 bits.

  CriticalSection psln_thrCS_;
  CriticalSection viewsCS_;
  CriticalSection markersCS_;
protected:
  Object();
  Object(r_code::Mem *mem);
public:
  virtual ~Object(); // un-registers from the rMem's object_register.

  r_code::_View *build_view(r_code::SysView *source) override {

    return r_code::Code::build_view<r_exec::View>(source);
  }

  bool is_invalidated() override;
  bool invalidate() override; // return false when was not invalidated, true otherwise.

  void compute_hash_value();

  float32 get_psln_thr() override;

  void acq_views() override { viewsCS_.enter(); }
  void rel_views() override { viewsCS_.leave(); }
  void acq_markers() override { markersCS_.enter(); }
  void rel_markers() override { markersCS_.leave(); }

  // Target psln_thr only.
  void set(uint16 member_index, float32 value) override;
  void mod(uint16 member_index, float32 value) override;

  r_code::_View *get_view(r_code::Code *group, bool lock) override; // returns the found view if any, NULL otherwise.

  class Hash {
  public:
    size_t operator ()(U *o) const {

      if (o->hash_value_ == 0)
        o->compute_hash_value();
      return o->hash_value_;
    }
  };

  class Equal {
  public:
    bool operator ()(const U *lhs, const U *rhs) const { // lhs and rhs have the same hash value_, i.e. same opcode, same code size and same reference size.

      if (lhs->code(0).asOpcode() == Opcodes::Ent || rhs->code(0).asOpcode() == Opcodes::Ent)
        return lhs == rhs;

      uint16 i;
      for (i = 0; i < lhs->references_size(); ++i)
        if (lhs->get_reference(i) != rhs->get_reference(i))
          return false;
      for (i = 0; i < lhs->code_size(); ++i) {

        if (lhs->code(i) != rhs->code(i))
          return false;
      }
      return true;
    }
  };
};

// Local object.
// Used for r-code that does not travel across networks (groups and notifications) or when the rMem is not distributed.
// Markers are killed when at least one of their references dies (held by their views).
// Marker deletion is performed by registering pending delete operations in the groups they are projected onto.
class r_exec_dll LObject :
  public Object<r_code::LocalObject, LObject> {
public:
  static bool RequiresPacking() { return false; }
  static LObject *Pack(r_code::Code *object, r_code::Mem* /* mem */) { return (LObject*)object; } // object is always a LObject (local operation).
  LObject(r_code::Mem *mem = NULL) : Object<r_code::LocalObject, LObject>(mem) {}
  LObject(r_code::SysObject *source) : Object<r_code::LocalObject, LObject>() {

    load(source);
  }
  virtual ~LObject() {}
};
}


#endif
