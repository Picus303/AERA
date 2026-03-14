#ifndef test_mem_h
#define test_mem_h

#include <fstream>

#include "mem.h"

template<class O, class S> class TestMem :
  public r_exec::MemExec<O, S> {
public:
  TestMem();
  ~TestMem();

  bool load(const std::vector<r_code::Code*>* objects, uint32 stdin_oid, uint32 stdout_oid, uint32 self_oid) override;
  r_code::Code* eject(r_code::Code* command) override;
  void on_diagnostic_time_tick() override { on_time_tick(); }

protected:
  class _Thread : public Thread {
  };

  struct CommandOpcodes {
    uint16 ready = 0xFFFF;
    uint16 set_velocity_y = 0xFFFF;
    uint16 set_force_y = 0xFFFF;
    uint16 move_y_plus = 0xFFFF;
    uint16 move_y_minus = 0xFFFF;
  };

  struct PropertyCache {
    r_code::Code* position = nullptr;
    r_code::Code* position_y = nullptr;
    r_code::Code* velocity_y = nullptr;
    r_code::Code* force_y = nullptr;
    r_code::Code* theta_y = nullptr;
    r_code::Code* omega_y = nullptr;
  };

  struct ContinuousMotionState {
    float velocity = 0.0001f;
    float position = 0.0f;
    r_code::Code* tracked_object = nullptr;
  };

  struct CartPoleState {
    float force = 0.1f;
    float next_velocity = 0.1f;
    float next_position = 1.0f;
    float next_theta = 1.0f;
    float next_omega = 0.1f;
    r_code::Code* tracked_object = nullptr;
  };

  struct DiscreteMotionState {
    r_code::Code* entities[10] = {};
    r_code::Code* tracked_object = nullptr;
    r_code::Code* current_position = nullptr;
    r_code::Code* pending_position = nullptr;
    int babble_state = 0;
  };

  bool cache_environment_objects(const std::vector<r_code::Code*>* objects);
  r_code::Code* handle_ready_command(r_code::Code* command, uint16 args_set_index);
  r_code::Code* handle_velocity_command(r_code::Code* command, uint16 args_set_index);
  r_code::Code* handle_force_command(r_code::Code* command, uint16 args_set_index);
  r_code::Code* handle_discrete_move_command(r_code::Code* command, uint16 function, uint16 args_set_index);

  void update_continuous_motion(Timestamp now);
  void update_cart_pole(Timestamp now);
  void update_discrete_motion(Timestamp now);

  void on_time_tick();
  void startTimeTickThread();
  static thread_ret thread_function_call timeTickRun(void* args);

  std::ofstream cart_output_;

  Thread* time_tick_thread_;
  Timestamp last_inject_time_;
  Timestamp last_command_time_;
  r_code::Code* primary_group_;

  CommandOpcodes opcodes_;
  PropertyCache properties_;
  ContinuousMotionState continuous_;
  CartPoleState cart_pole_;
  DiscreteMotionState discrete_;
};

#endif
