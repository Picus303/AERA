#ifndef extension_registration_h
#define extension_registration_h

#include <chrono>
#include <string>

#include "dll.h"
#include "types.h"

namespace r_code {
class Code;
class _View;
}

namespace r_exec {

class Context;
class Controller;

using OpcodeRetriever = core::uint16 (*)(const char*);
using OperatorFunction = bool (*)(const Context&);
using ProgramFactory = Controller* (*)(r_code::_View*);
using CallbackFunction = bool (*)(std::chrono::microseconds relative_time,
  bool suspended,
  const char* msg,
  core::uint8 object_count,
  r_code::Code** objects);

class r_exec_dll ExtensionRegistrar {
public:
  virtual ~ExtensionRegistrar() {}

  virtual void MarkUserDefinedOperatorClass(core::uint16 opcode) = 0;
  virtual bool RegisterOperator(const std::string& operator_name, OperatorFunction function) = 0;
  virtual void RegisterProgram(const std::string& program_name, ProgramFactory program) = 0;
  virtual void RegisterCallback(const std::string& callback_name, CallbackFunction callback) = 0;
};

}  // namespace r_exec

#endif
