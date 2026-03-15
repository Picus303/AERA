#include <iostream>
#include <string>

#include "settings.h"

namespace {

bool contains(const std::string& value, const std::string& fragment) {
  return value.find(fragment) != std::string::npos;
}

int fail(const std::string& message) {
  std::cerr << message << std::endl;
  return 1;
}

int expect_valid_generated(const char* path) {
  aera::RuntimeSettings settings;
  if (!settings.load_from_file(path))
    return fail("Expected generated smoke settings to load successfully.");

  if (settings.io_device != "test_mem")
    return fail("Expected io_device to be test_mem.");
  if (settings.execution.base_period != 50000)
    return fail("Expected base_period to be 50000.");
  if (settings.execution.reduction_core_count != 0 || settings.execution.time_core_count != 0)
    return fail("Expected generated smoke settings to use diagnostic time.");
  if (!settings.debug.objects.get || !settings.debug.objects.decompile)
    return fail("Expected object capture and decompilation to be enabled.");
  if (!settings.run.models.get || !settings.run.models.decompile)
    return fail("Expected model capture and decompilation to be enabled.");
  if (!contains(settings.load.source_file_name, "main.replicode"))
    return fail("Expected generated smoke settings to point to main.replicode.");
  if (!contains(settings.load.usr_class_path, "user.classes.replicode"))
    return fail("Expected generated smoke settings to point to user.classes.replicode.");

  return 0;
}

int expect_repo_layout(const char* path) {
  aera::RuntimeSettings settings;
  if (!settings.load_from_file(path))
    return fail("Expected repository settings.xml to load successfully.");

  if (settings.io_device != "test_mem")
    return fail("Expected repository settings.xml to use test_mem.");
  if (!contains(settings.load.source_file_name, "../examples/replicode/main.replicode"))
    return fail("Expected repository settings.xml to point to examples/replicode/main.replicode.");
  if (!contains(settings.load.usr_class_path, "../examples/replicode/user.classes.replicode"))
    return fail("Expected repository settings.xml to point to examples/replicode/user.classes.replicode.");

  return 0;
}

int expect_invalid_missing_attribute(const char* path) {
  aera::RuntimeSettings settings;
  if (settings.load_from_file(path))
    return fail("Expected invalid settings fixture to fail loading.");

  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 3)
    return fail("Usage: settings_characterization <mode> <path>");

  const std::string mode = argv[1];
  const char* path = argv[2];

  if (mode == "valid_generated")
    return expect_valid_generated(path);
  if (mode == "repo_layout")
    return expect_repo_layout(path);
  if (mode == "invalid_missing_attribute")
    return expect_invalid_missing_attribute(path);

  return fail("Unknown test mode: " + mode);
}
