

#include "class.h"
#include "segments.h"

using namespace std;
using namespace r_code;

namespace r_comp {

const char *Class::Expression = "xpr";
const char *Class::Type = "type";

bool Class::has_offset() const {

  switch (atom_.getDescriptor()) {
  case Atom::OBJECT:
  case Atom::GROUP:
  case Atom::INSTANTIATED_PROGRAM:
  case Atom::INSTANTIATED_CPP_PROGRAM:
  case Atom::S_SET:
  case Atom::MARKER: return true;
  default: return false;
  }
}

Class::Class(ReturnType t) : type(t), str_opcode("undefined") {
}

Class::Class(Atom atom,
  std::string str_opcode,
  vector<StructureMember> r,
  ReturnType t) : atom_(atom),
  str_opcode(str_opcode),
  things_to_read(r),
  type(t),
  use_as(StructureMember::I_CLASS) {
}

bool Class::is_pattern(Metadata *metadata) const {

  return (metadata->classes_.find("ptn")->second.atom_ == atom_) || (metadata->classes_.find("|ptn")->second.atom_ == atom_);
}

bool Class::is_fact(Metadata *metadata) const {

  return (metadata->classes_.find("fact")->second.atom_ == atom_) || (metadata->classes_.find("|fact")->second.atom_ == atom_);
}

bool Class::get_member_index(Metadata *metadata, std::string &name, uint16 &index, Class *&p) const {

  for (uint16 i = 0; i < things_to_read.size(); ++i)
    if (things_to_read[i].name_ == name) {

      index = (has_offset() ? i + 1 : i); // in expressions the lead r-atom is at 0; in objects, members start at 1
      if (things_to_read[i].used_as_expression()) // the class is: [::a-class]
        p = NULL;
      else
        p = things_to_read[i].get_class(metadata);
      return true;
    }
  return false;
}

std::string Class::get_member_name(uint32 index) {

  return things_to_read[has_offset() ? index - 1 : index].name_;
}

ReturnType Class::get_member_type(const uint16 index) {

  return things_to_read[has_offset() ? index - 1 : index].get_return_type();
}

Class *Class::get_member_class(Metadata *metadata, const std::string &name) {

  for (uint16 i = 0; i < things_to_read.size(); ++i)
    if (things_to_read[i].name_ == name)
      return things_to_read[i].get_class(metadata);
  return NULL;
}

void Class::write(word32 *storage) {

  storage[0] = atom_.atom_;
  r_code::Write(storage + 1, str_opcode);
  uint32 offset = 1 + r_code::GetSize(str_opcode);
  storage[offset++] = type;
  storage[offset++] = use_as;
  storage[offset++] = things_to_read.size();
  for (uint32 i = 0; i < things_to_read.size(); ++i) {

    things_to_read[i].write(storage + offset);
    offset += things_to_read[i].get_size();
  }
}

void Class::read(word32 *storage) {

  atom_ = storage[0];
  r_code::Read(storage + 1, str_opcode);
  uint32 offset = 1 + r_code::GetSize(str_opcode);
  type = (ReturnType)storage[offset++];
  use_as = (StructureMember::Iteration)storage[offset++];
  uint32 member_count = storage[offset++];
  for (uint32 i = 0; i < member_count; ++i) {

    StructureMember m;
    m.read(storage + offset);
    things_to_read.push_back(m);
    offset += m.get_size();
  }
}

uint32 Class::get_size() { // see segments.cpp for the RAM layout

  uint32 size = 4; // atom, return type, usage, number of members
  size += r_code::GetSize(str_opcode);
  for (uint32 i = 0; i < things_to_read.size(); ++i)
    size += things_to_read[i].get_size();
  return size;
}
}
