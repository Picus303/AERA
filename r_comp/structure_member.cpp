

#include "structure_member.h"
#include "compiler.h"


namespace r_comp {

StructureMember::StructureMember() {
}

StructureMember::StructureMember(_Read r,
  std::string m,
  std::string p,
  Iteration i) : read_(r),
  name_(m),
  class_(p),
  iteration_(i) {

  if (read_ == &Compiler::read_any) type_ = ANY;
  else if (read_ == &Compiler::read_number) type_ = NUMBER;
  else if (read_ == &Compiler::read_timestamp) type_ = TIMESTAMP;
  else if (read_ == &Compiler::read_duration) type_ = DURATION;
  else if (read_ == &Compiler::read_boolean) type_ = BOOLEAN;
  else if (read_ == &Compiler::read_string) type_ = STRING;
  else if (read_ == &Compiler::read_node) type_ = NODE_ID;
  else if (read_ == &Compiler::read_device) type_ = DEVICE_ID;
  else if (read_ == &Compiler::read_function) type_ = FUNCTION_ID;
  else if (read_ == &Compiler::read_expression) type_ = ANY;
  else if (read_ == &Compiler::read_set) type_ = ReturnType::SET;
  else if (read_ == &Compiler::read_class) type_ = ReturnType::CLASS;
}

Class *StructureMember::get_class(Metadata *metadata) const {

  return class_ == "" ? NULL : &metadata->classes_.find(class_)->second;
}

ReturnType StructureMember::get_return_type() const {

  return type_;
}

bool StructureMember::used_as_expression() const {

  return iteration_ == I_EXPRESSION;
}

StructureMember::Iteration StructureMember::getIteration() const {

  return iteration_;
}

_Read StructureMember::read() const {

  return read_;
}

void StructureMember::write(word32 *storage) const {

  if (read_ == &Compiler::read_any)
    storage[0] = R_ANY;
  else if (read_ == &Compiler::read_number)
    storage[0] = R_NUMBER;
  else if (read_ == &Compiler::read_timestamp)
    storage[0] = R_TIMESTAMP;
  else if (read_ == &Compiler::read_duration)
    storage[0] = R_DURATION;
  else if (read_ == &Compiler::read_boolean)
    storage[0] = R_BOOLEAN;
  else if (read_ == &Compiler::read_string)
    storage[0] = R_STRING;
  else if (read_ == &Compiler::read_node)
    storage[0] = R_NODE;
  else if (read_ == &Compiler::read_device)
    storage[0] = R_DEVICE;
  else if (read_ == &Compiler::read_function)
    storage[0] = R_FUNCTION;
  else if (read_ == &Compiler::read_expression)
    storage[0] = R_EXPRESSION;
  else if (read_ == &Compiler::read_set)
    storage[0] = R_SET;
  else if (read_ == &Compiler::read_class)
    storage[0] = R_CLASS;
  uint32 offset = 1;
  storage[offset++] = type_;
  r_code::Write(storage + offset, class_);
  offset += r_code::GetSize(class_);
  storage[offset++] = iteration_;
  r_code::Write(storage + offset, name_);
}

void StructureMember::read(word32 *storage) {

  switch (storage[0]) {
  case R_ANY: read_ = &Compiler::read_any; break;
  case R_NUMBER: read_ = &Compiler::read_number; break;
  case R_TIMESTAMP: read_ = &Compiler::read_timestamp; break;
  case R_DURATION: read_ = &Compiler::read_duration; break;
  case R_BOOLEAN: read_ = &Compiler::read_boolean; break;
  case R_STRING: read_ = &Compiler::read_string; break;
  case R_NODE: read_ = &Compiler::read_node; break;
  case R_DEVICE: read_ = &Compiler::read_device; break;
  case R_FUNCTION: read_ = &Compiler::read_function; break;
  case R_EXPRESSION: read_ = &Compiler::read_expression; break;
  case R_SET: read_ = &Compiler::read_set; break;
  case R_CLASS: read_ = &Compiler::read_class; break;
  }
  uint32 offset = 1;
  type_ = (ReturnType)storage[offset++];
  r_code::Read(storage + offset, class_);
  offset += r_code::GetSize(class_);
  iteration_ = (Iteration)storage[offset++];
  r_code::Read(storage + offset, name_);
}

uint32 StructureMember::get_size() { // see segments.cpp for the RAM layout

  uint32 size = 3; // read ID, return type, iteration
  size += r_code::GetSize(class_);
  size += r_code::GetSize(name_);
  return size;
}
}
