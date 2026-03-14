

#include "init.h"
#include "construction/factory.h"
#include "evaluation/binding_map.h"
#include "evaluation/operator.h"
#include "cpp_programs.h"
#include "../../r_code/replicode_defs.h"
#include "../../r_code/utils.h"
#include <math.h>


namespace r_exec {

template<class O, class S> MemExec<O, S>::MemExec() : S() {
}

template<class O, class S> MemExec<O, S>::~MemExec() {

  if (state_ == RUNNING)
    S::stop();
  deleted_ = true;
  objects_.clear();
}

////////////////////////////////////////////////////////////////

template<class O, class S> r_code::Code *MemExec<O, S>::build_object(r_code::SysObject *source) const {

  Atom head = source->code_[0];
  switch (head.getDescriptor()) {
  case Atom::GROUP:
    return new Group(source);
  default: {
    uint16 opcode = head.asOpcode();
    if (opcode == Opcodes::Fact)
      return new Fact(source);
    else if (opcode == Opcodes::AntiFact)
      return new AntiFact(source);
    else if (opcode == Opcodes::Goal)
      return new Goal(source);
    else if (opcode == Opcodes::Pred)
      return new Pred(source);
    else if (opcode == Opcodes::ICst)
      return new ICST(source);
    else if (opcode == Opcodes::MkRdx)
      return new MkRdx(source);
    else if (opcode == Opcodes::MkActChg ||
      opcode == Opcodes::MkHighAct ||
      opcode == Opcodes::MkHighSln ||
      opcode == Opcodes::MkLowAct ||
      opcode == Opcodes::MkLowRes ||
      opcode == Opcodes::MkLowSln ||
      opcode == Opcodes::MkNew ||
      opcode == Opcodes::MkSlnChg ||
      opcode == Opcodes::Success ||
      opcode == Opcodes::Perf)
      return new r_code::LocalObject(source);
    else
      return new O(source);
  }
  }
}

template<class O, class S> r_code::Code *MemExec<O, S>::_build_object(Atom head) const {

  r_code::Code *object = new O();
  object->code(0) = head;
  return object;
}

template<class O, class S> r_code::Code *MemExec<O, S>::build_object(Atom head) const {

  r_code::Code *object;
  switch (head.getDescriptor()) {
  case Atom::GROUP:
    object = new Group();
    break;
  default: {
    uint16 opcode = head.asOpcode();
    if (opcode == Opcodes::Fact)
      object = new Fact();
    else if (opcode == Opcodes::AntiFact)
      object = new AntiFact();
    else if (opcode == Opcodes::Pred)
      object = new Pred();
    else if (opcode == Opcodes::Goal)
      object = new Goal();
    else if (opcode == Opcodes::ICst)
      object = new ICST();
    else if (opcode == Opcodes::MkRdx)
      object = new MkRdx();
    else if (opcode == Opcodes::MkActChg ||
      opcode == Opcodes::MkHighAct ||
      opcode == Opcodes::MkHighSln ||
      opcode == Opcodes::MkLowAct ||
      opcode == Opcodes::MkLowRes ||
      opcode == Opcodes::MkLowSln ||
      opcode == Opcodes::MkNew ||
      opcode == Opcodes::MkSlnChg ||
      opcode == Opcodes::Success ||
      opcode == Opcodes::Perf)
      object = new r_code::LocalObject();
    else if (O::RequiresPacking())
      object = new r_code::LocalObject(); // temporary sand box for assembling code; will be packed into an O at injection time.
    else
      object = new O();
    break;
  }
  }

  object->code(0) = head;
  return object;
}

////////////////////////////////////////////////////////////////

template<class O, class S> r_code::Code *MemExec<O, S>::check_existence(r_code::Code *object) {

  if (object->code(0).getDescriptor() == Atom::GROUP) // groups are always new.
    return object;

  O *_object;
  if (O::RequiresPacking()) // false if LObject, true for network-aware objects.
    _object = O::Pack(object, this); // non compact form will be deleted (P<> in view) if not an instance of O; compact forms are left unchanged.
  else
    _object = (O *)object;

  return _object;
}

template<class O, class S> void MemExec<O, S>::inject(O *object, View *view) {

  view->set_object(object);
  S::inject_new_object(view);
}
}
