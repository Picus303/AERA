
#include "runtime_runner.h"

using namespace core;

int32 main(int argc, char** argv) {
  const char* file_name = (argc >= 2 ? argv[1] : "settings.xml");
  const char* decompiled_file_name = (argc >= 3 ? argv[2] : "");

  return start_AERA(file_name, decompiled_file_name);
}

