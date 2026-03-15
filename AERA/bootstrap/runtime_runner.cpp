#include "runtime_runner.h"

#include <fstream>
#include <unordered_map>

#include "init.h"
#include "runtime_exports.h"
#include "settings.h"
#include "test_mem.h"

using namespace std;
using namespace std::chrono;
using namespace r_code;
using namespace r_comp;

namespace {

bool find_named_object_id(const unordered_map<uint32, string>& symbols,
  const string& name,
  uint32& object_id) {
  for (const auto& symbol : symbols) {
    if (symbol.second == name) {
      object_id = symbol.first;
      return true;
    }
  }

  return false;
}

bool resolve_runtime_object_ids(uint32& stdin_oid, uint32& stdout_oid, uint32& self_oid) {
  const auto& symbols = r_exec::Seed.object_names_.symbols_;
  return find_named_object_id(symbols, "stdin", stdin_oid)
    && find_named_object_id(symbols, "stdout", stdout_oid)
    && find_named_object_id(symbols, "self", self_oid);
}

void export_objects(r_exec::_Mem* mem,
  const aera::RuntimeSettings& settings,
  r_comp::Decompiler& decompiler,
  Timestamp starting_time) {
  if (!settings.debug.objects.get)
    return;

  r_comp::Image* image = mem->get_objects(settings.debug.objects.keep_invalidated);
  image->object_names_.symbols_ = r_exec::Seed.object_names_.symbols_;

  if (settings.debug.objects.write) {
    write_image_to_file(
      image,
      settings.debug.objects.output_path,
      settings.debug.objects.test_round_trip ? &decompiler : NULL,
      starting_time);
  }

  if (settings.debug.objects.decompile
    && (!settings.debug.objects.write || !settings.debug.objects.test_round_trip)) {
    if (settings.debug.objects.decompile_to_file) {
      ofstream outfile(settings.debug.objects.decompilation_file_path.c_str(), ios_base::trunc);
      streambuf* coutbuf = cout.rdbuf(outfile.rdbuf());
      decompile_image(decompiler, image, starting_time, settings.debug.objects.ignore_named);
      cout.rdbuf(coutbuf);
      outfile.close();
    }
    else {
      decompile_image(decompiler, image, starting_time, settings.debug.objects.ignore_named);
    }
  }

  delete image;
}

void export_models(r_exec::_Mem* mem,
  const aera::RuntimeSettings& settings,
  r_comp::Decompiler& decompiler,
  Timestamp starting_time,
  const char* decompiled_file_name) {
  if (!settings.run.models.get)
    return;

  r_comp::Image* image = mem->get_models();
  image->object_names_.symbols_ = r_exec::Seed.object_names_.symbols_;

  if (settings.run.models.write) {
    write_image_to_file(
      image,
      settings.run.models.output_path,
      settings.run.models.test_round_trip ? &decompiler : NULL,
      starting_time);
  }

  if (settings.run.models.decompile
    && (!settings.run.models.write || !settings.run.models.test_round_trip)) {
    if (decompiled_file_name && decompiled_file_name[0] != '\0') {
      ofstream outfile(decompiled_file_name, ios_base::trunc);
      streambuf* coutbuf = cout.rdbuf(outfile.rdbuf());
      decompile_image(decompiler, image, starting_time, settings.run.models.ignore_named);
      cout.rdbuf(coutbuf);
      outfile.close();
    }
    else {
      decompile_image(decompiler, image, starting_time, settings.run.models.ignore_named);
    }
  }

  delete image;
}

}  // namespace

core::int32 start_AERA(const char* file_name, const char* decompiled_file_name) {

  core::Time::Init(1000);

  aera::RuntimeSettings settings;
  if (!settings.load_from_file(file_name))
    return 1;

  ofstream runtime_output_stream;
  if (!settings.debug.runtime_output_file_path.empty()) {
    runtime_output_stream.open(settings.debug.runtime_output_file_path);
    if (!runtime_output_stream.is_open()) {
      cout << "Cannot open runtime_output_file_path \"" << settings.debug.runtime_output_file_path << "\"" << endl;
      return 2;
    }
  }

  cout << "> compiling ...\n";
  if (settings.execution.reduction_core_count == 0 && settings.execution.time_core_count == 0) {
    r_exec::_Mem::diagnostic_time_now_ = Time::Get();
    if (!r_exec::Init(
      r_exec::_Mem::get_diagnostic_time_now,
      settings.load.usr_class_path.c_str())) {
      return 2;
    }
  }
  else {
    if (!r_exec::Init(Time::Get, settings.load.usr_class_path.c_str()))
      return 2;
  }

  srand(duration_cast<microseconds>(r_exec::Now().time_since_epoch()).count());
  Random::Init();

  string error;
  if (!r_exec::Compile(settings.load.source_file_name.c_str(), error)) {
    cerr << " <- " << error << endl;
    return 3;
  }

  cout << "> ... done\n";

  r_exec::PipeOStream::Open(settings.debug.window_count);

  Decompiler decompiler;
  decompiler.init(&r_exec::Metadata);

  if (settings.io_device != "test_mem") {
    cerr << "> Error: unsupported IODevice \"" << settings.io_device
      << "\". The current baseline only supports test_mem.\n";
    return 4;
  }

  r_exec::_Mem* mem = settings.debug.objects.get
    ? static_cast<r_exec::_Mem*>(new TestMem<r_exec::LObject, r_exec::MemStatic>())
    : static_cast<r_exec::_Mem*>(new TestMem<r_exec::LObject, r_exec::MemVolatile>());

  if (runtime_output_stream.is_open())
    mem->set_default_runtime_output_stream(&runtime_output_stream);

  resized_vector<r_code::Code*> ram_objects;
  r_exec::Seed.get_objects(mem, ram_objects);

  mem->init(microseconds(settings.execution.base_period),
    settings.execution.reduction_core_count,
    settings.execution.time_core_count,
    settings.system.mdl_inertia_sr_threshold,
    settings.system.mdl_inertia_count_threshold,
    settings.system.tpx_dsr_threshold,
    microseconds(settings.system.min_sim_time_horizon),
    microseconds(settings.system.max_sim_time_horizon),
    settings.system.sim_time_horizon_factor,
    microseconds(settings.system.tpx_time_horizon),
    microseconds(settings.system.perf_sampling_period),
    settings.system.float_tolerance,
    microseconds(settings.system.time_tolerance),
    seconds(settings.system.primary_thz),
    seconds(settings.system.secondary_thz),
    settings.debug.enabled,
    settings.debug.resilience.ntf_mk,
    settings.debug.resilience.goal_pred_success,
    settings.run.probe_level,
    settings.debug.trace_levels,
    settings.debug.objects.keep_invalidated);

  uint32 stdin_oid = 0;
  uint32 stdout_oid = 0;
  uint32 self_oid = 0;
  if (!resolve_runtime_object_ids(stdin_oid, stdout_oid, self_oid)) {
    cerr << "> Error: failed to resolve stdin/stdout/self in the seed image.\n";
    return 4;
  }

  if (!mem->load(ram_objects.as_std(), stdin_oid, stdout_oid, self_oid))
    return 4;
  auto starting_time = mem->start();

  if (settings.execution.reduction_core_count == 0 && settings.execution.time_core_count == 0) {
    cout << "> running for " << settings.run.duration_ms << " ms in diagnostic time\n\n";
    mem->run_in_diagnostic_time(milliseconds(settings.run.duration_ms));
  }
  else {
    cout << "> running for " << settings.run.duration_ms << " ms\n\n";
    Thread::Sleep(milliseconds(settings.run.duration_ms));
  }

  cout << "\n> shutting rMem down...\n";
  mem->stop();

  export_objects(mem, settings, decompiler, starting_time);
  export_models(mem, settings, decompiler, starting_time, decompiled_file_name);

  delete mem;
  r_exec::PipeOStream::Close();

  return 0;
}
