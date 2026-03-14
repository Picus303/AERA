

#ifndef operators_h
#define operators_h

#include "../types.h"


namespace usr_operators {

bool add(const r_exec::Context &context);
bool sub(const r_exec::Context &context);
bool mul(const r_exec::Context &context);
bool div(const r_exec::Context& context);
bool dis(const r_exec::Context &context);

}

class Operators {
public:
  static r_code::resized_vector<uint16> Init(OpcodeRetriever r);
};


#endif
