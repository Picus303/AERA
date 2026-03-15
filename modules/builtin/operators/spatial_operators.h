
#ifndef builtin_spatial_operators_h
#define builtin_spatial_operators_h

#include "extension_registration.h"
#include "operator.h"
#include "resized_vector.h"

namespace aera::builtin {

r_code::resized_vector<uint16> InitializeSpatialOperatorClasses(r_exec::OpcodeRetriever opcode_retriever);

bool add(const r_exec::Context& context);
bool sub(const r_exec::Context& context);
bool mul(const r_exec::Context& context);
bool div(const r_exec::Context& context);
bool dis(const r_exec::Context& context);

}  // namespace aera::builtin

#endif
