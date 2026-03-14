

#include "guard_builder.h"

using namespace std::chrono;
using namespace r_code;

namespace r_exec {

GuardBuilder::GuardBuilder() : _Object() {
}

GuardBuilder::~GuardBuilder() {
}

void GuardBuilder::build(Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  mdl->code(MDL_FWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(0);

  mdl->code(MDL_BWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TimingGuardBuilder::TimingGuardBuilder(microseconds period) : GuardBuilder(), period_(period) {
}

TimingGuardBuilder::~TimingGuardBuilder() {
}

void TimingGuardBuilder::write_guard(Code *mdl, uint16 l, uint16 r, uint16 opcode, microseconds offset, uint16 &write_index, uint16 &extent_index) const {

  mdl->code(++write_index) = Atom::AssignmentPointer(l, ++extent_index);
  mdl->code(extent_index) = Atom::Operator(opcode, 2); // l:(opcode r offset)
  mdl->code(++extent_index) = Atom::VLPointer(r);
  uint16 index = extent_index + 2;
  mdl->code(++extent_index) = Atom::IPointer(index);
  Utils::SetDurationStruct(mdl, ++extent_index, offset);
  extent_index += 2;
}

void TimingGuardBuilder::_build(Code *mdl, uint16 t0, uint16 t1, uint16 &write_index) const {

  Code *rhs_val = mdl->get_reference(1);
  uint16 t2 = rhs_val->code(FACT_AFTER).asIndex();
  uint16 t3 = rhs_val->code(FACT_BEFORE).asIndex();

  mdl->code(MDL_FWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(2);

  uint16 extent_index = write_index + 2;

  write_guard(mdl, t2, t0, Opcodes::Add, period_, write_index, extent_index);
  write_guard(mdl, t3, t1, Opcodes::Add, period_, write_index, extent_index);

  write_index = extent_index;
  mdl->code(MDL_BWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(2);

  extent_index = write_index + 2;

  write_guard(mdl, t0, t2, Opcodes::Sub, period_, write_index, extent_index);
  write_guard(mdl, t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  write_index = extent_index;
}

void TimingGuardBuilder::build(Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  uint16 t0;
  uint16 t1;
  uint16 tpl_arg_set_index = mdl->code(MDL_TPL_ARGS).asIndex();
  if (mdl->code(tpl_arg_set_index).getAtomCount() == 0) {

    Code *lhs_val = mdl->get_reference(0);
    t0 = lhs_val->code(FACT_AFTER).asIndex();
    t1 = lhs_val->code(FACT_BEFORE).asIndex();
  } else { // use the tpl args.

    t0 = 1;
    t1 = 2;
  }

  _build(mdl, t0, t1, write_index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SGuardBuilder::SGuardBuilder(microseconds period, microseconds offset) : TimingGuardBuilder(period), offset_(offset) {
}

SGuardBuilder::~SGuardBuilder() {
}

void SGuardBuilder::_build(Code *mdl, uint16 q0, uint16 t0, uint16 t1, uint16 &write_index) const {

  Code *rhs = mdl->get_reference(1);
  uint16 t2 = rhs->code(FACT_AFTER).asIndex();
  uint16 t3 = rhs->code(FACT_BEFORE).asIndex();
  uint16 q1 = rhs->get_reference(0)->code(MK_VAL_VALUE).asIndex();

  Code *lhs = mdl->get_reference(0);
  uint16 speed_t0 = lhs->code(FACT_AFTER).asIndex();
  uint16 speed_t1 = lhs->code(FACT_BEFORE).asIndex();
  uint16 speed_value = lhs->get_reference(0)->code(MK_VAL_VALUE).asIndex();

  mdl->code(MDL_FWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(3);

  uint16 extent_index = write_index + 3;

  write_guard(mdl, t2, t0, Opcodes::Add, period_, write_index, extent_index);
  write_guard(mdl, t3, t1, Opcodes::Add, period_, write_index, extent_index);

  mdl->code(++write_index) = Atom::AssignmentPointer(q1, ++extent_index);
  mdl->code(extent_index) = Atom::Operator(Opcodes::Add, 2); // q1:(+ q0 (* s period))
  mdl->code(++extent_index) = Atom::VLPointer(q0);
  uint16 index = extent_index + 2;
  mdl->code(++extent_index) = Atom::IPointer(index);
  mdl->code(++extent_index) = Atom::Operator(Opcodes::Mul, 2);
  mdl->code(++extent_index) = Atom::VLPointer(speed_value);
  index = extent_index + 2;
  mdl->code(++extent_index) = Atom::IPointer(index);
  Utils::SetDurationStruct(mdl, ++extent_index, period_);
  extent_index += 2;

  write_index = extent_index;
  mdl->code(MDL_BWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(5);

  extent_index = write_index + 5;

  write_guard(mdl, t0, t2, Opcodes::Sub, period_, write_index, extent_index);
  write_guard(mdl, t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  write_guard(mdl, speed_t0, t2, Opcodes::Sub, offset_, write_index, extent_index);
  write_guard(mdl, speed_t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  mdl->code(++write_index) = Atom::AssignmentPointer(speed_value, ++extent_index);
  mdl->code(extent_index) = Atom::Operator(Opcodes::Div, 2); // s:(/ (- q1 q0) period)
  index = extent_index + 3;
  mdl->code(++extent_index) = Atom::IPointer(index);
  index = extent_index + 5;
  mdl->code(++extent_index) = Atom::IPointer(index);
  mdl->code(++extent_index) = Atom::Operator(Opcodes::Sub, 2);
  mdl->code(++extent_index) = Atom::VLPointer(q1);
  mdl->code(++extent_index) = Atom::VLPointer(q0);
  Utils::SetDurationStruct(mdl, ++extent_index, period_);
  extent_index += 2;

  write_index = extent_index;
}

void SGuardBuilder::build(Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  uint16 q0;
  uint16 t0;
  uint16 t1;
  uint16 tpl_arg_set_index = mdl->code(MDL_TPL_ARGS).asIndex();
  if (mdl->code(tpl_arg_set_index).getAtomCount() == 0) {

    q0 = premise_pattern->get_reference(0)->code(MK_VAL_VALUE).asIndex();
    t0 = premise_pattern->code(FACT_AFTER).asIndex();
    t1 = premise_pattern->code(FACT_BEFORE).asIndex();
  } else { // use the tpl args.

    q0 = 0;
    t0 = 1;
    t1 = 2;
  }

  _build(mdl, q0, t0, t1, write_index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NoArgCmdGuardBuilder::NoArgCmdGuardBuilder(microseconds period, microseconds offset, microseconds cmd_duration)
: TimingGuardBuilder(period), offset_(offset), cmd_duration_(cmd_duration) {
}

NoArgCmdGuardBuilder::~NoArgCmdGuardBuilder() {
}

void NoArgCmdGuardBuilder::_build(Code *mdl, uint16 q0, uint16 t0, uint16 t1, uint16 &write_index) const {

  Code *rhs = mdl->get_reference(1);
  uint16 t2 = rhs->code(FACT_AFTER).asIndex();
  uint16 t3 = rhs->code(FACT_BEFORE).asIndex();

  Code *lhs = mdl->get_reference(0);
  uint16 cmd_t0 = lhs->code(FACT_AFTER).asIndex();
  uint16 cmd_t1 = lhs->code(FACT_BEFORE).asIndex();

  mdl->code(MDL_FWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(2);

  uint16 extent_index = write_index + 2;

  write_guard(mdl, t2, t0, Opcodes::Add, period_, write_index, extent_index);
  write_guard(mdl, t3, t1, Opcodes::Add, period_, write_index, extent_index);

  write_index = extent_index;
  mdl->code(MDL_BWD_GUARDS) = Atom::IPointer(++write_index);

  mdl->code(write_index) = Atom::Set(4);
  extent_index = write_index + 4;

  write_guard(mdl, t0, t2, Opcodes::Sub, period_, write_index, extent_index);
  write_guard(mdl, t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  write_guard(mdl, cmd_t0, t2, Opcodes::Sub, offset_, write_index, extent_index);
  write_guard(mdl, cmd_t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  write_index = extent_index;
}

void NoArgCmdGuardBuilder::build(Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  uint16 q0;
  uint16 t0;
  uint16 t1;
  uint16 tpl_arg_set_index = mdl->code(MDL_TPL_ARGS).asIndex();
  if (mdl->code(tpl_arg_set_index).getAtomCount() == 0) {

    q0 = premise_pattern->get_reference(0)->code(MK_VAL_VALUE).asIndex();
    t0 = premise_pattern->code(FACT_AFTER).asIndex();
    t1 = premise_pattern->code(FACT_BEFORE).asIndex();
  } else { // use the tpl args.

    q0 = 0;
    t0 = 1;
    t1 = 2;
  }

  _build(mdl, q0, t0, t1, write_index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CmdGuardBuilder::CmdGuardBuilder(microseconds period, microseconds offset, uint16 cmd_arg_index) : TimingGuardBuilder(period), offset_(offset), cmd_arg_index_(cmd_arg_index) {
}

CmdGuardBuilder::~CmdGuardBuilder() {
}

void CmdGuardBuilder::_build(Code *mdl, uint16 fwd_opcode, uint16 bwd_opcode, uint16 q0, uint16 t0, uint16 t1, uint16 &write_index) const {

  Code *rhs = mdl->get_reference(1);
  uint16 t2 = rhs->code(FACT_AFTER).asIndex();
  uint16 t3 = rhs->code(FACT_BEFORE).asIndex();
  uint16 q1 = rhs->get_reference(0)->code(MK_VAL_VALUE).asIndex();

  Code *lhs = mdl->get_reference(0);
  uint16 cmd_t0 = lhs->code(FACT_AFTER).asIndex();
  uint16 cmd_t1 = lhs->code(FACT_BEFORE).asIndex();
  uint16 cmd_arg = lhs->get_reference(0)->code(cmd_arg_index_).asIndex();

  mdl->code(MDL_FWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(3);

  uint16 extent_index = write_index + 3;

  write_guard(mdl, t2, t0, Opcodes::Add, period_, write_index, extent_index);
  write_guard(mdl, t3, t1, Opcodes::Add, period_, write_index, extent_index);

  mdl->code(++write_index) = Atom::AssignmentPointer(q1, ++extent_index);
  mdl->code(extent_index) = Atom::Operator(fwd_opcode, 2); // q1:(fwd_opcode q0 cmd_arg)
  mdl->code(++extent_index) = Atom::VLPointer(q0);
  mdl->code(++extent_index) = Atom::VLPointer(cmd_arg);
  extent_index += 1;

  write_index = extent_index;
  mdl->code(MDL_BWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(5);

  extent_index = write_index + 5;

  write_guard(mdl, t0, t2, Opcodes::Sub, period_, write_index, extent_index);
  write_guard(mdl, t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  write_guard(mdl, cmd_t0, t2, Opcodes::Sub, offset_, write_index, extent_index);
  write_guard(mdl, cmd_t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  mdl->code(++write_index) = Atom::AssignmentPointer(cmd_arg, ++extent_index);
  mdl->code(extent_index) = Atom::Operator(bwd_opcode, 2); // cmd_arg:(bwd_opcode q1 q0)
  mdl->code(++extent_index) = Atom::VLPointer(q1);
  mdl->code(++extent_index) = Atom::VLPointer(q0);
  extent_index += 1;

  write_index = extent_index;
}

void CmdGuardBuilder::_build(Code *mdl, uint16 fwd_opcode, uint16 bwd_opcode, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  uint16 q0;
  uint16 t0;
  uint16 t1;
  uint16 tpl_arg_set_index = mdl->code(MDL_TPL_ARGS).asIndex();
  if (mdl->code(tpl_arg_set_index).getAtomCount() == 0) {

    q0 = premise_pattern->get_reference(0)->code(MK_VAL_VALUE).asIndex();
    t0 = premise_pattern->code(FACT_AFTER).asIndex();
    t1 = premise_pattern->code(FACT_BEFORE).asIndex();
  } else { // use the tpl args.

    q0 = 0;
    t0 = 1;
    t1 = 2;
  }

  _build(mdl, fwd_opcode, bwd_opcode, q0, t0, t1, write_index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MCGuardBuilder::MCGuardBuilder(microseconds period, microseconds offset, float32 cmd_arg_index) : CmdGuardBuilder(period, offset, cmd_arg_index) {
}

MCGuardBuilder::~MCGuardBuilder() {
}

void MCGuardBuilder::build(Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  _build(mdl, Opcodes::Mul, Opcodes::Div, premise_pattern, cause_pattern, write_index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACGuardBuilder::ACGuardBuilder(microseconds period, microseconds offset, uint16 cmd_arg_index) : CmdGuardBuilder(period, offset, cmd_arg_index) {
}

ACGuardBuilder::~ACGuardBuilder() {
}

void ACGuardBuilder::build(Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  _build(mdl, Opcodes::Add, Opcodes::Sub, premise_pattern, cause_pattern, write_index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ConstGuardBuilder::ConstGuardBuilder(microseconds period, float32 constant, microseconds offset) : TimingGuardBuilder(period), constant_(constant), offset_(offset) {
}

ConstGuardBuilder::~ConstGuardBuilder() {
}

void ConstGuardBuilder::_build(Code *mdl, uint16 fwd_opcode, uint16 bwd_opcode, uint16 q0, uint16 t0, uint16 t1, uint16 &write_index) const {

  Code *rhs = mdl->get_reference(1);
  uint16 t2 = rhs->code(FACT_AFTER).asIndex();
  uint16 t3 = rhs->code(FACT_BEFORE).asIndex();
  uint16 q1 = rhs->get_reference(0)->code(MK_VAL_VALUE).asIndex();

  Code *lhs = mdl->get_reference(1);
  uint16 t4 = lhs->code(FACT_AFTER).asIndex();
  uint16 t5 = lhs->code(FACT_BEFORE).asIndex();

  mdl->code(MDL_FWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(3);

  uint16 extent_index = write_index + 3;

  write_guard(mdl, t2, t0, Opcodes::Add, period_, write_index, extent_index);
  write_guard(mdl, t3, t1, Opcodes::Add, period_, write_index, extent_index);

  mdl->code(++write_index) = Atom::AssignmentPointer(q1, ++extent_index);
  mdl->code(extent_index) = Atom::Operator(fwd_opcode, 2); // q1:(fwd_opcode q0 constant)
  mdl->code(++extent_index) = Atom::VLPointer(q0);
  mdl->code(++extent_index) = Atom::Float(constant_);
  extent_index += 1;

  write_index = extent_index;
  mdl->code(MDL_BWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(5);

  extent_index = write_index + 5;

  write_guard(mdl, t0, t2, Opcodes::Sub, period_, write_index, extent_index);
  write_guard(mdl, t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  write_guard(mdl, t4, t2, Opcodes::Sub, offset_, write_index, extent_index);
  write_guard(mdl, t5, t3, Opcodes::Sub, period_, write_index, extent_index);

  mdl->code(++write_index) = Atom::AssignmentPointer(q0, ++extent_index);
  mdl->code(extent_index) = Atom::Operator(bwd_opcode, 2); // q0:(bwd_opcode q1 constant)
  mdl->code(++extent_index) = Atom::VLPointer(q1);
  mdl->code(++extent_index) = Atom::Float(constant_);
  extent_index += 1;

  write_index = extent_index;
}

void ConstGuardBuilder::_build(Code *mdl, uint16 fwd_opcode, uint16 bwd_opcode, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  uint16 q0;
  uint16 t0;
  uint16 t1;
  uint16 tpl_arg_set_index = mdl->code(MDL_TPL_ARGS).asIndex();
  if (mdl->code(tpl_arg_set_index).getAtomCount() == 0) {

    q0 = premise_pattern->get_reference(0)->code(MK_VAL_VALUE).asIndex();
    t0 = premise_pattern->code(FACT_AFTER).asIndex();
    t1 = premise_pattern->code(FACT_BEFORE).asIndex();
  } else { // use the tpl args.

    q0 = 0;
    t0 = 1;
    t1 = 2;
  }

  _build(mdl, fwd_opcode, bwd_opcode, q0, t0, t1, write_index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MGuardBuilder::MGuardBuilder(microseconds period, float32 constant, microseconds offset) : ConstGuardBuilder(period, constant, offset) {
}

MGuardBuilder::~MGuardBuilder() {
}

void MGuardBuilder::build(Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  _build(mdl, Opcodes::Mul, Opcodes::Div, premise_pattern, cause_pattern, write_index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AGuardBuilder::AGuardBuilder(microseconds period, float32 constant, microseconds offset) : ConstGuardBuilder(period, constant, offset) {
}

AGuardBuilder::~AGuardBuilder() {
}

void AGuardBuilder::build(Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const {

  _build(mdl, Opcodes::Add, Opcodes::Sub, premise_pattern, cause_pattern, write_index);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ConstBwdArgCmdGuardBuilder::ConstBwdArgCmdGuardBuilder(microseconds period, microseconds offset, uint16 cmd_arg_index, _Fact* cause) : TimingGuardBuilder(period), offset_(offset), cmd_arg_index_(cmd_arg_index), cause_(cause) {
}

ConstBwdArgCmdGuardBuilder::~ConstBwdArgCmdGuardBuilder() {
}

void ConstBwdArgCmdGuardBuilder::build(Code* mdl, _Fact* premise_pattern, _Fact* cause_pattern, uint16& write_index) const {
  // use the tpl args.
  uint16 t0 = 1;
  uint16 t1 = 2;

  Code* rhs = mdl->get_reference(1);
  uint16 t2 = rhs->code(FACT_AFTER).asIndex();
  uint16 t3 = rhs->code(FACT_BEFORE).asIndex();

  Code* lhs = mdl->get_reference(0);
  uint16 cmd_t0 = lhs->code(FACT_AFTER).asIndex();
  uint16 cmd_t1 = lhs->code(FACT_BEFORE).asIndex();
  uint16 cmd_arg = lhs->get_reference(0)->code(cmd_arg_index_).asIndex();

  mdl->code(MDL_FWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(2);

  uint16 extent_index = write_index + 2;

  write_guard(mdl, t2, t0, Opcodes::Add, period_, write_index, extent_index);
  write_guard(mdl, t3, t1, Opcodes::Add, period_, write_index, extent_index);

  write_index = extent_index;
  mdl->code(MDL_BWD_GUARDS) = Atom::IPointer(++write_index);
  mdl->code(write_index) = Atom::Set(5);

  extent_index = write_index + 5;

  write_guard(mdl, t0, t2, Opcodes::Sub, period_, write_index, extent_index);
  write_guard(mdl, t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  write_guard(mdl, cmd_t0, t2, Opcodes::Sub, offset_, write_index, extent_index);
  write_guard(mdl, cmd_t1, t3, Opcodes::Sub, period_, write_index, extent_index);

  // Assign the specific value from the cause.
  Atom cmd_arg_value = cause_->get_reference(0)->code(cmd_arg_index_);
  mdl->code(++write_index) = Atom::AssignmentPointer(cmd_arg, ++extent_index);
  if (cmd_arg_value.getDescriptor() == Atom::I_PTR &&
    cause_->get_reference(0)->code(cmd_arg_value.asIndex()).getDescriptor() == Atom::OBJECT)
    // Copy the object.
    StructureValue::copy_structure(
      mdl, extent_index, &cause_->get_reference(0)->code(0), cmd_arg_value.asIndex());
  else if (cmd_arg_value.isFloat()) {
    // We can't put the float value in the AssignmentPointer, so point to the identity operator.
    mdl->code(extent_index) = Atom::Operator(Opcodes::Id, 1);
    mdl->code(++extent_index) = cmd_arg_value;
    extent_index += 1;
  }

  write_index = extent_index;
}

}
