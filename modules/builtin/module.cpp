#include "module.h"

#include <iostream>

#include "callbacks/print_callback.h"
#include "operators/spatial_operators.h"
#include "programs/auto_focus_program.h"
#include "programs/test_program.h"

namespace {

bool register_spatial_operators(r_exec::ExtensionRegistrar& registrar) {
  return registrar.RegisterOperator("add", &aera::builtin::add)
    && registrar.RegisterOperator("sub", &aera::builtin::sub)
    && registrar.RegisterOperator("mul", &aera::builtin::mul)
    && registrar.RegisterOperator("div", &aera::builtin::div)
    && registrar.RegisterOperator("dis", &aera::builtin::dis);
}

}  // namespace

namespace aera::builtin {

bool RegisterModule(r_exec::OpcodeRetriever opcode_retriever,
  r_exec::ExtensionRegistrar& registrar) {
  const r_code::resized_vector<uint16> user_defined_operator_classes =
    InitializeSpatialOperatorClasses(opcode_retriever);

  for (uint16 opcode : *user_defined_operator_classes.as_std())
    registrar.MarkUserDefinedOperatorClass(opcode);

  if (!register_spatial_operators(registrar))
    return false;

  registrar.RegisterProgram("test_program", &CreateTestProgram);
  registrar.RegisterProgram("auto_focus", &CreateAutoFocusProgram);
  registrar.RegisterCallback("print", &print);

  std::cout << "> builtin extension module registered" << std::endl;
  return true;
}

}  // namespace aera::builtin
