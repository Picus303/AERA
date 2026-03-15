

#ifndef class_h
#define class_h

#include <vector>
#include "structure_member.h"


namespace r_comp {

class Compiler;

class Class {
private:
  bool has_offset() const;
public:
  static const char *Expression;
  static const char *Type;

  Class(ReturnType t = ANY);
  Class(r_code::Atom atom,
    std::string str_opcode,
    std::vector<StructureMember> r,
    ReturnType t = ANY);
  bool is_pattern(Metadata *metadata) const;
  bool is_fact(Metadata *metadata) const;
  bool get_member_index(Metadata *metadata, std::string &name, uint16 &index, Class *&p) const;
  std::string get_member_name(uint32 index); // for decompilation
  ReturnType get_member_type(const uint16 index);
  Class *get_member_class(Metadata *metadata, const std::string &name);
  r_code::Atom atom_;
  std::string str_opcode; // unused for anything but objects, markers and operators.
  std::vector<StructureMember> things_to_read;
  ReturnType type; // ANY for non-operators.
  StructureMember::Iteration use_as;

  void write(word32 *storage);
  void read(word32 *storage);
  uint32 get_size();
};
}


#endif
