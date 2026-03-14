

#include "utils.h"
#include "atom.h"

#include <iostream>

using namespace std;
using namespace std::chrono;

namespace r_code {

Atom::TraceContext::TraceContext() {
  members_to_go_ = 0;
  timestamp_data_ = 0;
  duration_data_ = 0;
  string_data_ = 0;
  char_count_ = 0;
}

void Atom::trace(TraceContext& context, std::ostream& out) const {

  context.write_indents(out);
  if (context.timestamp_data_) {
    // Output the timestamp value now. Otherwise, it could be interpreted
    // as an op code, etc.
    --context.timestamp_data_;
    out << atom_;

    if (context.timestamp_data_ == 1)
      // Save for the next step.
      context.int64_high_ = atom_;
    else {
      // Imitate Utils::GetTimestamp.
      auto timestamp = core::Timestamp(microseconds(context.int64_high_ << 32 | atom_));
      out << " " << Utils::RelativeTime(timestamp);
    }
    return;
  }
  if (context.duration_data_) {
    // Output the duration value now. Otherwise, it could be interpreted as an op code, etc.
    --context.duration_data_;
    out << atom_;

    if (context.duration_data_ == 1)
      // Save for the next step.
      context.int64_high_ = atom_;
    else {
      // Imitate Utils::GetDuration.
      auto duration = microseconds(context.int64_high_ << 32 | atom_);
      out << " " << Utils::ToString_us(duration);
    }
    return;
  }

  switch (getDescriptor()) {
  case NIL: out << "nil"; return;
  case BOOLEAN_: out << "bl: " << std::boolalpha << asBoolean(); return;
  case WILDCARD: out << ":"; return;
  case T_WILDCARD: out << "::"; return;
  case I_PTR: out << "iptr: " << std::dec << asIndex(); return;
  case VL_PTR: out << "vlptr: " << std::dec << asIndex(); return;
  case R_PTR: out << "rptr: " << std::dec << asIndex(); return;
  case IPGM_PTR: out << "ipgm_ptr: " << std::dec << asIndex(); return;
  case IN_OBJ_PTR: out << "in_obj_ptr: " << std::dec << (uint32)asInputIndex() << " " << asIndex(); return;
  case D_IN_OBJ_PTR: out << "d_in_obj_ptr: " << std::dec << (uint32)asRelativeIndex() << " " << asIndex(); return;
  case OUT_OBJ_PTR: out << "out_obj_ptr: " << std::dec << asIndex(); return;
  case VALUE_PTR: out << "value_ptr: " << std::dec << asIndex(); return;
  case PROD_PTR: out << "prod_ptr: " << std::dec << asIndex(); return;
  case ASSIGN_PTR: out << "assign_ptr: " << std::dec << (uint16)asAssignmentIndex() << " " << asIndex(); return;
  case CODE_VL_PTR: out << "code_vlptr: " << std::dec << asIndex(); return;
  case THIS: out << "this"; return;
  case VIEW: out << "view"; return;
  case MKS: out << "mks"; return;
  case VWS: out << "vws"; return;
  case NODE: out << "nid: " << std::dec << (uint32)getNodeID(); return;
  case DEVICE: out << "did: " << std::dec << (uint32)getNodeID() << " " << (uint32)getClassID() << " " << (uint32)getDeviceID(); return;
  case DEVICE_FUNCTION: out << "fid: " << std::dec << asOpcode() << " (" << GetOpcodeName(asOpcode()).c_str() << ")"; return;
  case C_PTR: out << "cptr: " << std::dec << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case SET: out << "set: " << std::dec << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case OBJECT: out << "obj: " << std::dec << asOpcode() << " (" << GetOpcodeName(asOpcode()).c_str() << ") " << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case S_SET: out << "s_set: " << std::dec << asOpcode() << " (" << GetOpcodeName(asOpcode()).c_str() << ") " << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case MARKER: out << "mk: " << std::dec << asOpcode() << " (" << GetOpcodeName(asOpcode()).c_str() << ") " << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case OPERATOR: out << "op: " << std::dec << asOpcode() << " (" << GetOpcodeName(asOpcode()).c_str() << ") " << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case STRING: out << "st: " << std::dec << (uint16)getAtomCount() << " " << (atom_ & 0x000000FF); context.members_to_go_ = context.string_data_ = getAtomCount(); context.char_count_ = (atom_ & 0x000000FF); return;
  case TIMESTAMP: out << "ts"; context.members_to_go_ = context.timestamp_data_ = 2; return;
  case DURATION: out << "us"; context.members_to_go_ = context.duration_data_ = 2; return;
  case GROUP: out << "grp: " << std::dec << asOpcode() << " (" << GetOpcodeName(asOpcode()).c_str() << ") " << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case INSTANTIATED_PROGRAM:
  case INSTANTIATED_ANTI_PROGRAM:
  case INSTANTIATED_INPUT_LESS_PROGRAM:
    out << "ipgm: " << std::dec << asOpcode() << " (" << GetOpcodeName(asOpcode()).c_str() << ") " << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case COMPOSITE_STATE: out << "cst: " << std::dec << asOpcode() << " (" << GetOpcodeName(asOpcode()).c_str() << ") " << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case MODEL: out << "mdl: " << std::dec << asOpcode() << " (" << GetOpcodeName(asOpcode()).c_str() << ") " << (uint16)getAtomCount(); context.members_to_go_ = getAtomCount(); return;
  case NULL_PROGRAM: out << "null pgm " << takesPastInputs() ? "all inputs" : "new inputs"; return;
  default:
    if (context.string_data_) {

      --context.string_data_;
      std::string s;
      char *content = (char *)&atom_;
      for (uint8 i = 0; i < 4; ++i) {

        if (context.char_count_-- > 0)
          s += content[i];
        else
          break;
      }
      out << s.c_str();
    } else if (isFloat()) {

      out << "nb: " << std::scientific << asFloat() << std::defaultfloat;
    } else
      out << "undef";
    return;
  }
}

void Atom::TraceContext::write_indents(std::ostream& out) {

  if (members_to_go_) {

    out << "   ";
    --members_to_go_;
  }
}

// These are filled by r_exec::Init().
unordered_map<uint16, set<string>> OpcodeNames;

string GetOpcodeName(uint16 opcode) {
  unordered_map<uint16, set<string>>::iterator names =
    OpcodeNames.find(opcode);
  if (names == OpcodeNames.end())
    return "unknown";

  string result;
  for (set<string>::iterator it = names->second.begin();
    it != names->second.end(); ++it) {
    if (result.size() != 0)
      result += "/";
    result += *it;
  }

  return result;
}

bool SetOpcodeNames(const std::unordered_map<uint16, std::set<std::string>>& opcode_names) {
  if (OpcodeNames.size() > 0)
    // GetOpcodeName has already been using a set of opcodes. The new codes could be inconsistent. Fail.
    return false;

  OpcodeNames = opcode_names;
  return true;
}

}
