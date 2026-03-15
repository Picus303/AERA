#ifndef builtin_module_h
#define builtin_module_h

#include "extension_registration.h"

namespace aera::builtin {

bool r_exec_dll RegisterModule(r_exec::OpcodeRetriever opcode_retriever,
  r_exec::ExtensionRegistrar& registrar);

}  // namespace aera::builtin

#endif
