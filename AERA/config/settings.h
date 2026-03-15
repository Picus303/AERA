#ifndef settings_h
#define settings_h

#include <string>

#include "types.h"

namespace aera {

struct LoadSettings {
  std::string usr_class_path;
  std::string source_file_name;
};

struct ExecutionSettings {
  core::uint32 base_period = 0;
  core::uint32 reduction_core_count = 0;
  core::uint32 time_core_count = 0;
};

struct SystemSettings {
  core::float32 mdl_inertia_sr_threshold = 0;
  core::uint32 mdl_inertia_count_threshold = 0;
  core::float32 tpx_dsr_threshold = 0;
  core::uint32 min_sim_time_horizon = 0;
  core::uint32 max_sim_time_horizon = 0;
  core::float32 sim_time_horizon_factor = 0;
  core::uint32 tpx_time_horizon = 0;
  core::uint32 perf_sampling_period = 0;
  core::float32 float_tolerance = 0;
  core::uint32 time_tolerance = 0;
  core::uint64 primary_thz = 0;
  core::uint64 secondary_thz = 0;
};

struct ObjectDumpSettings {
  bool get = false;
  bool keep_invalidated = false;
  bool decompile = false;
  bool decompile_to_file = false;
  std::string decompilation_file_path;
  bool ignore_named = false;
  bool write = false;
  std::string output_path;
  bool test_round_trip = false;
};

struct DebugResilienceSettings {
  core::uint32 ntf_mk = 0;
  core::uint32 goal_pred_success = 0;
};

struct DebugSettings {
  bool enabled = false;
  core::uint32 window_count = 0;
  std::string runtime_output_file_path;
  core::uint32 trace_levels = 0;
  DebugResilienceSettings resilience;
  ObjectDumpSettings objects;
};

struct ModelDumpSettings {
  bool get = false;
  bool decompile = false;
  bool ignore_named = false;
  bool write = false;
  std::string output_path;
  bool test_round_trip = false;
};

struct RunSettings {
  core::uint32 duration_ms = 0;
  core::uint32 probe_level = 0;
  ModelDumpSettings models;
};

struct RuntimeSettings {
  std::string io_device;
  LoadSettings load;
  ExecutionSettings execution;
  SystemSettings system;
  DebugSettings debug;
  RunSettings run;

  bool load_from_file(const char* file_name);
};

}  // namespace aera

#endif
