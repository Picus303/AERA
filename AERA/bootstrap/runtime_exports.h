#ifndef runtime_exports_h
#define runtime_exports_h

#include "compiler.h"
#include "decompiler.h"

void decompile_image(r_comp::Decompiler& decompiler,
  r_comp::Image* image,
  core::Timestamp time_reference,
  bool ignore_named_objects);

void write_image_to_file(r_comp::Image* image,
  const std::string& image_path,
  r_comp::Decompiler* decompiler,
  core::Timestamp time_reference);

#endif
