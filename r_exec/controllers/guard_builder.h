

#ifndef guard_builder_h
#define guard_builder_h

#include "construction/factory.h"


namespace r_exec {

class GuardBuilder :
  public _Object {
public:
  GuardBuilder();
  virtual ~GuardBuilder();

  virtual void build(r_code::Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const;
};

// fwd: t2=t0+period, t3=t1+period.
// bwd: t0=t2-period, t1=t3-period.
class TimingGuardBuilder :
  public GuardBuilder {
protected:
  std::chrono::microseconds period_;

  void write_guard(r_code::Code *mdl, uint16 l, uint16 r, uint16 opcode, std::chrono::microseconds offset, uint16 &write_index, uint16 &extent_index) const;
  void _build(r_code::Code *mdl, uint16 t0, uint16 t1, uint16 &write_index) const;
public:
  TimingGuardBuilder(std::chrono::microseconds period);
  virtual ~TimingGuardBuilder();

  void build(r_code::Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const override;
};

// fwd: q1=q0+speed*period.
// bwd: speed=(q1-q0)/period, speed.after=q1.after-offset, speed.before=q1.before-offset.
class SGuardBuilder :
  public TimingGuardBuilder {
private:
  std::chrono::microseconds offset_; // period-(speed.after-t0).

  void _build(r_code::Code *mdl, uint16 q0, uint16 t0, uint16 t1, uint16 &write_index) const;
public:
  SGuardBuilder(std::chrono::microseconds period, std::chrono::microseconds offset);
  ~SGuardBuilder();

  void build(r_code::Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const override;
};

// bwd: cmd.after=q1.after-offset, cmd.before=cmd.after+cmd_duration.
class NoArgCmdGuardBuilder :
  public TimingGuardBuilder {
protected:
  std::chrono::microseconds offset_;
  std::chrono::microseconds cmd_duration_;

  void _build(r_code::Code *mdl, uint16 q0, uint16 t0, uint16 t1, uint16 &write_index) const;
public:
  /**
   * \param (optional) add_imdl_template_timings If true, assume that the lhs is an imdl and add
   * backward guards similar to those added for t0 and t1, but assign the imdl template timings.
   * If ommitted, use false.
   */
  NoArgCmdGuardBuilder(std::chrono::microseconds period, std::chrono::microseconds offset, std::chrono::microseconds cmd_duration);
  ~NoArgCmdGuardBuilder();

  void build(r_code::Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const override;
};

// bwd: cmd.after=q1.after-period, cmd.before=q1.before-period.
class CmdGuardBuilder :
  public TimingGuardBuilder {
protected:
  std::chrono::microseconds offset_;
  uint16 cmd_arg_index_;

  void _build(r_code::Code *mdl, uint16 fwd_opcode, uint16 bwd_opcode, uint16 q0, uint16 t0, uint16 t1, uint16 &write_index) const;
  void _build(r_code::Code *mdl, uint16 fwd_opcode, uint16 bwd_opcode, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const;

  CmdGuardBuilder(std::chrono::microseconds period, std::chrono::microseconds offset, uint16 cmd_arg_index);
public:
  virtual ~CmdGuardBuilder();
};

// fwd: q1=q0*cmd_arg.
// bwd: cmd_arg=q1/q0.
class MCGuardBuilder :
  public CmdGuardBuilder {
public:
  MCGuardBuilder(std::chrono::microseconds period, std::chrono::microseconds offset, float32 cmd_arg_index);
  ~MCGuardBuilder();

  void build(r_code::Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const override;
};

// fwd: q1=q0+cmd_arg.
// bwd: cmd_arg=q1-q0.
class ACGuardBuilder :
  public CmdGuardBuilder {
private:

public:
  ACGuardBuilder(std::chrono::microseconds period, std::chrono::microseconds offset, uint16 cmd_arg_index);
  ~ACGuardBuilder();

  void build(r_code::Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const override;
};

// bwd: cause.after=t2-offset, cause.before=t3-offset.
class ConstGuardBuilder :
  public TimingGuardBuilder {
protected:
  float32 constant_;
  std::chrono::microseconds offset_;

  void _build(r_code::Code *mdl, uint16 fwd_opcode, uint16 bwd_opcode, uint16 q0, uint16 t0, uint16 t1, uint16 &write_index) const;
  void _build(r_code::Code *mdl, uint16 fwd_opcode, uint16 bwd_opcode, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const;

  ConstGuardBuilder(std::chrono::microseconds period, float32 constant, std::chrono::microseconds offset);
public:
  ~ConstGuardBuilder();
};

// fwd: q1=q0*constant.
// bwd: q0=q1/constant.
class MGuardBuilder :
  public ConstGuardBuilder {
public:
  MGuardBuilder(std::chrono::microseconds period, float32 constant, std::chrono::microseconds offset);
  ~MGuardBuilder();

  void build(r_code::Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const override;
};

// fwd: q1=q0+constant.
// bwd: q0=q1-constant.
class AGuardBuilder :
  public ConstGuardBuilder {
public:
  AGuardBuilder(std::chrono::microseconds period, float32 constant, std::chrono::microseconds offset);
  ~AGuardBuilder();

  void build(r_code::Code *mdl, _Fact *premise_pattern, _Fact *cause_pattern, uint16 &write_index) const override;
};

/**
 * Use the timings of NoArgCmdGuardBuilder, but also add a backward guard to set the cmd arg to
 * the constant value from the given original cause.
 */
class ConstBwdArgCmdGuardBuilder :
  public TimingGuardBuilder {
protected:
  std::chrono::microseconds offset_;
  uint16 cmd_arg_index_;
  P<_Fact> cause_;
public:
  ConstBwdArgCmdGuardBuilder(std::chrono::microseconds period, std::chrono::microseconds offset, uint16 cmd_arg_index, _Fact* cause);
  virtual ~ConstBwdArgCmdGuardBuilder();

  void build(r_code::Code* mdl, _Fact* premise_pattern, _Fact* cause_pattern, uint16& write_index) const override;
};

}


#endif
