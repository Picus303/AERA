#include "test_mem.h"

using namespace std;
using namespace std::chrono;
using namespace r_code;
using namespace r_exec;

namespace {

constexpr float kMaxForce = 20.0f;

void warn_missing_property(const char* property_name) {
  cout << "WARNING: Can't find the " << property_name << " property" << endl;
}

bool require_property(r_code::Code* property, const char* property_name) {
  if (property)
    return true;

  warn_missing_property(property_name);
  return false;
}

bool bind_tracked_object(r_code::Code*& tracked_object, r_code::Code* candidate) {
  if (!tracked_object) {
    tracked_object = candidate;
    return true;
  }

  return tracked_object == candidate;
}

bool read_identifier_argument(r_code::Code* command, uint16 args_set_index, string& identifier) {
  if (!(command->code_size() >= 2
    && command->code(args_set_index + 1).getDescriptor() == Atom::I_PTR
    && command->code(command->code(args_set_index + 1).asIndex()).getDescriptor() == Atom::STRING)) {
    return false;
  }

  identifier = Utils::GetString(&command->code(command->code(args_set_index + 1).asIndex()));
  return true;
}

bool read_object_argument(r_code::Code* command,
  uint16 object_arg_index,
  const char* warning_message,
  r_code::Code*& object) {
  if (!(command->code_size() >= object_arg_index + 1
    && command->code(object_arg_index).getDescriptor() == Atom::R_PTR
    && command->references_size() > command->code(object_arg_index).asIndex())) {
    cout << "WARNING: " << warning_message << endl;
    return false;
  }

  object = command->get_reference(command->code(object_arg_index).asIndex());
  return true;
}

}  // namespace

template<class O, class S> TestMem<O, S>::TestMem()
  : MemExec<O, S>(),
    time_tick_thread_(nullptr),
    last_inject_time_(Timestamp(seconds(0))),
    last_command_time_(Timestamp(seconds(0))),
    primary_group_(nullptr) {
}

template<class O, class S> TestMem<O, S>::~TestMem() {
  if (time_tick_thread_)
    delete time_tick_thread_;
}

template<class O, class S> bool TestMem<O, S>::load(
  const vector<Code*>* objects,
  uint32 stdin_oid,
  uint32 stdout_oid,
  uint32 self_oid) {
  if (!MemExec<O, S>::load(objects, stdin_oid, stdout_oid, self_oid))
    return false;

  opcodes_.ready = r_exec::GetOpcode("ready");
  opcodes_.set_velocity_y = r_exec::GetOpcode("set_velocity_y");
  opcodes_.set_force_y = r_exec::GetOpcode("set_force_y");
  opcodes_.move_y_plus = r_exec::GetOpcode("move_y_plus");
  opcodes_.move_y_minus = r_exec::GetOpcode("move_y_minus");

  return cache_environment_objects(objects);
}

template<class O, class S> bool TestMem<O, S>::cache_environment_objects(const vector<Code*>* objects) {
  properties_.position = S::find_object(objects, "position");
  properties_.position_y = S::find_object(objects, "position_y");
  properties_.velocity_y = S::find_object(objects, "velocity_y");
  properties_.force_y = S::find_object(objects, "force_y");
  properties_.theta_y = S::find_object(objects, "theta_y");
  properties_.omega_y = S::find_object(objects, "omega_y");
  primary_group_ = S::find_object(objects, "primary");

  for (int i = 0; i <= 9; ++i)
    discrete_.entities[i] = S::find_object(objects, ("y" + to_string(i)).c_str());

  return true;
}

template<class O, class S> Code* TestMem<O, S>::eject(Code* command) {
  uint16 function = (command->code(CMD_FUNCTION).atom_ >> 8) & 0x000000FF;
  uint16 args_set_index = command->code(CMD_ARGS).asIndex();

  if (function == opcodes_.ready)
    return handle_ready_command(command, args_set_index);
  if (function == opcodes_.set_velocity_y)
    return handle_velocity_command(command, args_set_index);
  if (function == opcodes_.set_force_y)
    return handle_force_command(command, args_set_index);
  if (function == opcodes_.move_y_plus || function == opcodes_.move_y_minus)
    return handle_discrete_move_command(command, function, args_set_index);

  return NULL;
}

template<class O, class S> Code* TestMem<O, S>::handle_ready_command(Code* command, uint16 args_set_index) {
  string identifier;
  if (!read_identifier_argument(command, args_set_index, identifier))
    return NULL;

  if (identifier == "ball") {
    if (!require_property(properties_.velocity_y, "velocity_y")
      || !require_property(properties_.position_y, "position_y")) {
      return NULL;
    }

    Code* object = nullptr;
    if (!read_object_argument(command, args_set_index + 2, "Cannot get the object for ready \"ball\"", object))
      return NULL;

    if (!bind_tracked_object(continuous_.tracked_object, object))
      return NULL;

    startTimeTickThread();
    return command;
  }

  if (identifier == "cart-pole") {
    if (!require_property(properties_.velocity_y, "velocity_y")
      || !require_property(properties_.position_y, "position_y")
      || !require_property(properties_.force_y, "force_y")
      || !require_property(properties_.omega_y, "omega_y")
      || !require_property(properties_.theta_y, "theta_y")) {
      return NULL;
    }

    Code* object = nullptr;
    if (!read_object_argument(command, args_set_index + 2, "Cannot get the object for ready \"cart-pole\"", object))
      return NULL;

    if (!bind_tracked_object(cart_pole_.tracked_object, object))
      return NULL;

    startTimeTickThread();
    cart_output_.open("cart.out");
    return command;
  }

  cout << "WARNING: Ignoring unrecognized ready command identifier: " << identifier << endl;
  return NULL;
}

template<class O, class S> Code* TestMem<O, S>::handle_velocity_command(Code* command, uint16 args_set_index) {
  if (!require_property(properties_.velocity_y, "velocity_y")
    || !require_property(properties_.position_y, "position_y")) {
    return NULL;
  }

  Code* object = nullptr;
  if (!read_object_argument(command, args_set_index + 1, "Cannot get the object for set_velocity_y", object))
    return NULL;

  if (!bind_tracked_object(continuous_.tracked_object, object))
    return NULL;

  last_command_time_ = r_exec::Now();
  continuous_.velocity = command->code(args_set_index + 2).asFloat();
  startTimeTickThread();
  return command;
}

template<class O, class S> Code* TestMem<O, S>::handle_force_command(Code* command, uint16 args_set_index) {
  if (!require_property(properties_.velocity_y, "velocity_y")
    || !require_property(properties_.position_y, "position_y")
    || !require_property(properties_.omega_y, "omega_y")
    || !require_property(properties_.theta_y, "theta_y")
    || !require_property(properties_.force_y, "force_y")) {
    return NULL;
  }

  Code* object = nullptr;
  if (!read_object_argument(command, args_set_index + 1, "Cannot get the object for set_force_y", object))
    return NULL;

  if (!bind_tracked_object(cart_pole_.tracked_object, object))
    return NULL;

  last_command_time_ = r_exec::Now();
  startTimeTickThread();

  float desired_force = command->code(args_set_index + 2).asFloat();
  if (desired_force > kMaxForce)
    desired_force = kMaxForce;
  else if (desired_force < -kMaxForce)
    desired_force = -kMaxForce;

  cart_pole_.force = desired_force;

  if (desired_force == command->code(args_set_index + 2).asFloat())
    return command;

  Code* clipped_force_command = new r_exec::LObject(this);
  clipped_force_command->code(0) = Atom::Object(r_exec::GetOpcode("cmd"), 3);
  clipped_force_command->code(1) = Atom::DeviceFunction(opcodes_.set_force_y);
  clipped_force_command->code(2) = Atom::IPointer(4);
  clipped_force_command->code(3) = Atom::Float(1);
  clipped_force_command->code(4) = Atom::Set(2);
  clipped_force_command->code(5) = Atom::RPointer(0);
  clipped_force_command->code(6) = Atom::Float(cart_pole_.force);
  clipped_force_command->set_reference(0, cart_pole_.tracked_object);
  return clipped_force_command;
}

template<class O, class S> Code* TestMem<O, S>::handle_discrete_move_command(Code* command, uint16 function, uint16 args_set_index) {
  if (!require_property(properties_.position, "position"))
    return NULL;

  for (int i = 0; i <= 9; ++i) {
    if (discrete_.entities[i])
      continue;

    cout << "WARNING: Can't find the entities y0, y1, etc." << endl;
    return NULL;
  }

  Code* object = nullptr;
  if (!read_object_argument(command, args_set_index + 1, "Cannot get the object for move_y command", object))
    return NULL;

  if (!bind_tracked_object(discrete_.tracked_object, object))
    return NULL;

  if (!discrete_.current_position) {
    discrete_.current_position = discrete_.entities[0];
    startTimeTickThread();
  }

  if (discrete_.pending_position)
    return NULL;

  last_command_time_ = r_exec::Now();
  const int max_y_position = 9;
  if (function == opcodes_.move_y_plus) {
    for (int i = 0; i <= max_y_position - 1; ++i) {
      if (discrete_.current_position == discrete_.entities[i])
        discrete_.pending_position = discrete_.entities[i + 1];
    }
  }
  else {
    for (int i = 1; i <= max_y_position; ++i) {
      if (discrete_.current_position == discrete_.entities[i])
        discrete_.pending_position = discrete_.entities[i - 1];
    }
  }

  return command;
}

template<class O, class S> void TestMem<O, S>::on_time_tick() {
  auto now = r_exec::Now();
  if (now <= last_inject_time_ + S::get_sampling_period() * 8 / 10)
    return;

  update_continuous_motion(now);
  update_cart_pole(now);
  update_discrete_motion(now);
}

template<class O, class S> void TestMem<O, S>::update_continuous_motion(Timestamp now) {
  if (!continuous_.tracked_object)
    return;

  if (last_inject_time_.time_since_epoch().count() != 0)
    continuous_.position += continuous_.velocity * duration_cast<microseconds>(now - last_inject_time_).count();

  last_inject_time_ = now;

  S::inject_marker_value_from_io_device(
    continuous_.tracked_object, properties_.velocity_y, Atom::Float(continuous_.velocity),
    now, now + S::get_sampling_period(), r_exec::View::SYNC_HOLD);
  S::inject_marker_value_from_io_device(
    continuous_.tracked_object, properties_.position_y, Atom::Float(continuous_.position),
    now, now + S::get_sampling_period());
}

template<class O, class S> void TestMem<O, S>::update_cart_pole(Timestamp now) {
  if (!cart_pole_.tracked_object)
    return;

  if (last_inject_time_.time_since_epoch().count() != 0) {
    float current_position = cart_pole_.next_position;
    float current_velocity = cart_pole_.next_velocity;
    float current_theta = cart_pole_.next_theta;
    float current_omega = cart_pole_.next_omega;

    cart_pole_.next_velocity = (0.1f * cart_pole_.force) + current_velocity;
    cart_pole_.next_position = (0.5e-2f * cart_pole_.force) + (0.1f * current_velocity) + current_position;
    cart_pole_.next_omega = current_theta + 0.1f * cart_pole_.force + current_omega;
    cart_pole_.next_theta = 0.0371943f * cart_pole_.force + 1.05042f * current_theta + 0.101675f * current_omega;
  }

  last_inject_time_ = now;

  S::inject_marker_value_from_io_device(
    cart_pole_.tracked_object, properties_.force_y, Atom::Float(cart_pole_.force),
    now, now + S::get_sampling_period());
  S::inject_marker_value_from_io_device(
    cart_pole_.tracked_object, properties_.velocity_y, Atom::Float(cart_pole_.next_velocity),
    now, now + S::get_sampling_period());
  S::inject_marker_value_from_io_device(
    cart_pole_.tracked_object, properties_.position_y, Atom::Float(cart_pole_.next_position),
    now, now + S::get_sampling_period());
  S::inject_marker_value_from_io_device(
    cart_pole_.tracked_object, properties_.theta_y, Atom::Float(cart_pole_.next_theta),
    now, now + S::get_sampling_period());
  S::inject_marker_value_from_io_device(
    cart_pole_.tracked_object, properties_.omega_y, Atom::Float(cart_pole_.next_omega),
    now, now + S::get_sampling_period());

  if (cart_output_.is_open()) {
    cart_output_ << cart_pole_.next_theta << "," << cart_pole_.next_omega << ","
      << cart_pole_.next_position << "," << cart_pole_.next_velocity << ","
      << cart_pole_.force << endl;
  }
}

template<class O, class S> void TestMem<O, S>::update_discrete_motion(Timestamp now) {
  if (!discrete_.tracked_object)
    return;

  if (discrete_.pending_position
    && now >= last_command_time_ + S::get_sampling_period() * 4 / 10) {
    discrete_.current_position = discrete_.pending_position;
    discrete_.pending_position = NULL;
  }

  last_inject_time_ = now;
  S::inject_marker_value_from_io_device(
    discrete_.tracked_object, properties_.position, discrete_.current_position,
    now, now + S::get_sampling_period());

  const microseconds babble_stop_time(3700000);
  const int max_babble_position = 9;
  if (now - Utils::GetTimeReference() >= babble_stop_time)
    return;

  if (discrete_.current_position == discrete_.entities[0])
    discrete_.babble_state = 1;
  else if (discrete_.current_position == discrete_.entities[max_babble_position])
    discrete_.babble_state = max_babble_position + 1;
  else {
    ++discrete_.babble_state;
    if (discrete_.babble_state >= max_babble_position * 2)
      discrete_.babble_state = 0;
  }

  uint16 next_command = (discrete_.babble_state >= 0 && discrete_.babble_state <= max_babble_position - 1)
    ? opcodes_.move_y_plus
    : opcodes_.move_y_minus;

  Code* cmd = new r_exec::LObject(this);
  cmd->code(0) = Atom::Object(r_exec::GetOpcode("cmd"), 3);
  cmd->code(1) = Atom::DeviceFunction(next_command);
  cmd->code(2) = Atom::IPointer(4);
  cmd->code(3) = Atom::Float(1);
  cmd->code(4) = Atom::Set(1);
  cmd->code(5) = Atom::RPointer(0);
  cmd->set_reference(0, discrete_.tracked_object);

  auto after = now + S::get_sampling_period() + 2 * Utils::GetTimeTolerance();
  r_exec::Fact* fact_command = new r_exec::Fact(cmd, after, now + 2 * S::get_sampling_period(), 1, 1);
  r_exec::Goal* goal = new r_exec::Goal(fact_command, S::get_self(), NULL, 1);
  S::inject_fact_from_io_device(goal, after, now + S::get_sampling_period(), primary_group_);
}

template<class O, class S> void TestMem<O, S>::startTimeTickThread() {
  if (S::reduction_core_count_ == 0 && S::time_core_count_ == 0)
    return;
  if (time_tick_thread_)
    return;

  time_tick_thread_ = Thread::New<_Thread>(timeTickRun, this);
}

template<class O, class S> thread_ret thread_function_call
TestMem<O, S>::timeTickRun(void* args) {
  TestMem<O, S>* self = (TestMem*)args;

  auto sampling_period = _Mem::Get()->get_sampling_period();
  auto tick_time = r_exec::Now();
  while (self->state_ == S::RUNNING) {
    self->on_time_tick();

    tick_time += sampling_period;
    Thread::Sleep(tick_time - r_exec::Now());
  }

  thread_ret_val(0);
}

template class TestMem<r_exec::LObject, r_exec::MemStatic>;
template class TestMem<r_exec::LObject, r_exec::MemVolatile>;
