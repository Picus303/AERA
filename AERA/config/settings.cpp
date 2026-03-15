#include "settings.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "xml_parser.h"

namespace {

bool is_yes(const char* value) {
  return value && std::strcmp(value, "yes") == 0;
}

bool read_required_attribute(const core::XMLNode& node,
  const char* attribute_name,
  const char* section_name,
  const char*& value) {
  value = node.getAttribute(attribute_name);
  if (value && value[0] != '\0')
    return true;

  std::cerr << "> Error: missing attribute \"" << attribute_name
    << "\" in section " << section_name << std::endl;
  return false;
}

bool read_uint32(const core::XMLNode& node,
  const char* attribute_name,
  const char* section_name,
  core::uint32& value) {
  const char* raw_value = nullptr;
  if (!read_required_attribute(node, attribute_name, section_name, raw_value))
    return false;

  value = static_cast<core::uint32>(std::atoi(raw_value));
  return true;
}

bool read_uint64(const core::XMLNode& node,
  const char* attribute_name,
  const char* section_name,
  core::uint64& value) {
  const char* raw_value = nullptr;
  if (!read_required_attribute(node, attribute_name, section_name, raw_value))
    return false;

  value = static_cast<core::uint64>(std::strtoull(raw_value, nullptr, 10));
  return true;
}

bool read_float32(const core::XMLNode& node,
  const char* attribute_name,
  const char* section_name,
  core::float32& value) {
  const char* raw_value = nullptr;
  if (!read_required_attribute(node, attribute_name, section_name, raw_value))
    return false;

  value = static_cast<core::float32>(std::atof(raw_value));
  return true;
}

bool read_trace_levels(const core::XMLNode& node,
  const char* attribute_name,
  const char* section_name,
  core::uint32& value) {
  const char* raw_value = nullptr;
  if (!read_required_attribute(node, attribute_name, section_name, raw_value))
    return false;

  std::sscanf(raw_value, "%x", &value);
  return true;
}

bool read_string(const core::XMLNode& node,
  const char* attribute_name,
  const char* section_name,
  std::string& value) {
  const char* raw_value = nullptr;
  if (!read_required_attribute(node, attribute_name, section_name, raw_value))
    return false;

  value = raw_value;
  return true;
}

bool read_bool(const core::XMLNode& node,
  const char* attribute_name,
  const char* section_name,
  bool& value) {
  const char* raw_value = nullptr;
  if (!read_required_attribute(node, attribute_name, section_name, raw_value))
    return false;

  value = is_yes(raw_value);
  return true;
}

bool load_io_device(const core::XMLNode& main_node, aera::RuntimeSettings& settings) {
  core::XMLNode io_device = main_node.getChildNode("IODevice");
  if (!io_device) {
    std::cerr << "> Error: IODevice section is unreadable" << std::endl;
    return false;
  }

  return read_string(io_device, "io_device", "IODevice", settings.io_device);
}

bool load_paths(const core::XMLNode& main_node, aera::RuntimeSettings& settings) {
  core::XMLNode load = main_node.getChildNode("Load");
  if (!load) {
    std::cerr << "> Error: Load section is unreadable" << std::endl;
    return false;
  }

  return read_string(load, "usr_class_path", "Load", settings.load.usr_class_path)
    && read_string(load, "source_file_name", "Load", settings.load.source_file_name);
}

bool load_execution(const core::XMLNode& main_node, aera::RuntimeSettings& settings) {
  core::XMLNode init = main_node.getChildNode("Init");
  if (!init) {
    std::cerr << "> Error: Init section is unreadable" << std::endl;
    return false;
  }

  return read_uint32(init, "base_period", "Init", settings.execution.base_period)
    && read_uint32(init, "reduction_core_count", "Init", settings.execution.reduction_core_count)
    && read_uint32(init, "time_core_count", "Init", settings.execution.time_core_count);
}

bool load_system(const core::XMLNode& main_node, aera::RuntimeSettings& settings) {
  core::XMLNode system = main_node.getChildNode("System");
  if (!system) {
    std::cerr << "> Error: System section is unreadable" << std::endl;
    return false;
  }

  return read_float32(system, "mdl_inertia_sr_thr", "System", settings.system.mdl_inertia_sr_threshold)
    && read_uint32(system, "mdl_inertia_cnt_thr", "System", settings.system.mdl_inertia_count_threshold)
    && read_float32(system, "tpx_dsr_thr", "System", settings.system.tpx_dsr_threshold)
    && read_uint32(system, "min_sim_time_horizon", "System", settings.system.min_sim_time_horizon)
    && read_uint32(system, "max_sim_time_horizon", "System", settings.system.max_sim_time_horizon)
    && read_float32(system, "sim_time_horizon_factor", "System", settings.system.sim_time_horizon_factor)
    && read_uint32(system, "tpx_time_horizon", "System", settings.system.tpx_time_horizon)
    && read_uint32(system, "perf_sampling_period", "System", settings.system.perf_sampling_period)
    && read_float32(system, "float_tolerance", "System", settings.system.float_tolerance)
    && read_uint32(system, "time_tolerance", "System", settings.system.time_tolerance)
    && read_uint64(system, "primary_thz", "System", settings.system.primary_thz)
    && read_uint64(system, "secondary_thz", "System", settings.system.secondary_thz);
}

bool load_debug(const core::XMLNode& main_node, aera::RuntimeSettings& settings) {
  core::XMLNode debug = main_node.getChildNode("Debug");
  if (!debug) {
    std::cerr << "> Error: Debug section is unreadable" << std::endl;
    return false;
  }

  if (!read_bool(debug, "debug", "Debug", settings.debug.enabled)
    || !read_uint32(debug, "debug_windows", "Debug", settings.debug.window_count)
    || !read_string(debug, "runtime_output_file_path", "Debug", settings.debug.runtime_output_file_path)
    || !read_trace_levels(debug, "trace_levels", "Debug", settings.debug.trace_levels)) {
    return false;
  }

  core::XMLNode resilience = debug.getChildNode("Resilience");
  if (!resilience) {
    std::cerr << "> Error: Debug/Resilience section is unreadable" << std::endl;
    return false;
  }

  if (!read_uint32(resilience, "ntf_mk_resilience", "Debug/Resilience", settings.debug.resilience.ntf_mk)
    || !read_uint32(resilience, "goal_pred_success_resilience", "Debug/Resilience", settings.debug.resilience.goal_pred_success)) {
    return false;
  }

  core::XMLNode objects = debug.getChildNode("Objects");
  if (!objects) {
    std::cerr << "> Error: Debug/Objects section is unreadable" << std::endl;
    return false;
  }

  if (!read_bool(objects, "get_objects", "Debug/Objects", settings.debug.objects.get)
    || !read_bool(objects, "keep_invalidated_objects", "Debug/Objects", settings.debug.objects.keep_invalidated)
    || !read_bool(objects, "decompile_objects", "Debug/Objects", settings.debug.objects.decompile)
    || !read_bool(objects, "decompile_to_file", "Debug/Objects", settings.debug.objects.decompile_to_file)
    || !read_string(objects, "decompilation_file_path", "Debug/Objects", settings.debug.objects.decompilation_file_path)
    || !read_bool(objects, "ignore_named_objects", "Debug/Objects", settings.debug.objects.ignore_named)
    || !read_bool(objects, "write_objects", "Debug/Objects", settings.debug.objects.write)
    || !read_bool(objects, "test_objects", "Debug/Objects", settings.debug.objects.test_round_trip)) {
    return false;
  }

  if (settings.debug.objects.write
    && !read_string(objects, "objects_path", "Debug/Objects", settings.debug.objects.output_path)) {
    return false;
  }

  return true;
}

bool load_run(const core::XMLNode& main_node, aera::RuntimeSettings& settings) {
  core::XMLNode run = main_node.getChildNode("Run");
  if (!run) {
    std::cerr << "> Error: Run section is unreadable" << std::endl;
    return false;
  }

  if (!read_uint32(run, "run_time", "Run", settings.run.duration_ms)
    || !read_uint32(run, "probe_level", "Run", settings.run.probe_level)) {
    return false;
  }

  core::XMLNode models = run.getChildNode("Models");
  if (!models) {
    std::cerr << "> Error: Run/Models section is unreadable" << std::endl;
    return false;
  }

  if (!read_bool(models, "get_models", "Run/Models", settings.run.models.get)
    || !read_bool(models, "decompile_models", "Run/Models", settings.run.models.decompile)
    || !read_bool(models, "ignore_named_models", "Run/Models", settings.run.models.ignore_named)
    || !read_bool(models, "write_models", "Run/Models", settings.run.models.write)
    || !read_bool(models, "test_models", "Run/Models", settings.run.models.test_round_trip)) {
    return false;
  }

  if (settings.run.models.write
    && !read_string(models, "models_path", "Run/Models", settings.run.models.output_path)) {
    return false;
  }

  return true;
}

}  // namespace

namespace aera {

bool RuntimeSettings::load_from_file(const char* file_name) {
  core::XMLNode main_node = core::XMLNode::openFileHelper(file_name, "AERAConfiguration");
  if (!main_node) {
    std::cerr << "> Error: AERAConfiguration is unreadable" << std::endl;
    return false;
  }

  return load_io_device(main_node, *this)
    && load_paths(main_node, *this)
    && load_execution(main_node, *this)
    && load_system(main_node, *this)
    && load_debug(main_node, *this)
    && load_run(main_node, *this);
}

}  // namespace aera
