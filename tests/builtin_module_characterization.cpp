#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "builtin/module.h"

namespace {

class MockRegistrar : public r_exec::ExtensionRegistrar {
public:
  void MarkUserDefinedOperatorClass(core::uint16 opcode) override {
    user_defined_classes.insert(opcode);
  }

  bool RegisterOperator(const std::string& operator_name, r_exec::OperatorFunction function) override {
    operators[operator_name] = function;
    return true;
  }

  void RegisterProgram(const std::string& program_name, r_exec::ProgramFactory program) override {
    programs[program_name] = program;
  }

  void RegisterCallback(const std::string& callback_name, r_exec::CallbackFunction callback) override {
    callbacks[callback_name] = callback;
  }

  std::map<std::string, r_exec::OperatorFunction> operators;
  std::map<std::string, r_exec::ProgramFactory> programs;
  std::map<std::string, r_exec::CallbackFunction> callbacks;
  std::set<core::uint16> user_defined_classes;
};

core::uint16 retrieve_opcode(const char* name) {
  static const std::map<std::string, core::uint16> opcodes = {
    {"vec3", 11},
    {"vec2", 12},
    {"vec", 13},
    {"quat", 14},
  };

  const auto it = opcodes.find(name);
  if (it == opcodes.end())
    return 0xFFFF;
  return it->second;
}

int fail(const std::string& message) {
  std::cerr << message << std::endl;
  return 1;
}

int expect_registration_contract() {
  MockRegistrar registrar;
  if (!aera::builtin::RegisterModule(&retrieve_opcode, registrar))
    return fail("Expected builtin module registration to succeed.");

  const std::vector<std::string> expected_operators = {"add", "dis", "div", "mul", "sub"};
  for (const std::string& name : expected_operators) {
    if (registrar.operators.find(name) == registrar.operators.end())
      return fail("Missing builtin operator registration: " + name);
  }

  if (registrar.programs.find("test_program") == registrar.programs.end())
    return fail("Missing builtin program registration: test_program");
  if (registrar.programs.find("auto_focus") == registrar.programs.end())
    return fail("Missing builtin program registration: auto_focus");
  if (registrar.callbacks.find("print") == registrar.callbacks.end())
    return fail("Missing builtin callback registration: print");

  const std::set<core::uint16> expected_classes = {11, 12, 13, 14};
  if (registrar.user_defined_classes != expected_classes)
    return fail("Unexpected builtin user-defined operator classes.");

  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2)
    return fail("Usage: builtin_module_characterization <mode>");

  const std::string mode = argv[1];
  if (mode == "registration_contract")
    return expect_registration_contract();

  return fail("Unknown test mode: " + mode);
}
