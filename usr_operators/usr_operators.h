

#ifndef usr_operators_h
#define usr_operators_h

#include "types.h"


extern "C" {
  r_code::resized_vector<uint16> dll_export Init(OpcodeRetriever r); // OpcodeRetriever allows the dll to retrieve opcodes from the r_rxec dll without referencing the corresponding STL structures.

// Operators //////////////////////////////////////////////////////////////////////////////

uint16 dll_export GetOperatorCount();
void dll_export GetOperatorName(char *op_name);

// CPP Programs //////////////////////////////////////////////////////////////////////////////

uint16 dll_export GetProgramCount();
void dll_export GetProgramName(char *pgm_name);

// Callbacks //////////////////////////////////////////////////////////////////////////////

uint16 dll_export GetCallbackCount();
void dll_export GetCallbackName(char *callback_name);

/**
 * Return the function with function_name or NULL if not found.
 * This can be used to implement r_exec::FunctionLibrary getFunction.
 */
void dll_export *GetUserOperatorFunction(const char* function_name);
}

#include "./Operators/operators.h"
#include "./TestProgram/test_program.h"
//#include "./Correlator/correlator.h" -- jm
#include "./Callbacks/callbacks.h"


#endif
