

#include "usr_operators.h"

#include "../r_exec/init.h"
#include "auto_focus_callback.h"

#include <iostream>
#include <cmath>


r_code::resized_vector<uint16> Init(OpcodeRetriever r) {

  r_code::resized_vector<uint16> val = Operators::Init(r);

  std::cout << "> usr operators initialized" << std::endl;

  return val;
}

uint16 GetOperatorCount() {

  return 5;
}

void GetOperatorName(char *op_name) {

  static uint16 op_index = 0;

  if (op_index == 0) {

    std::string s = "add";
    memcpy(op_name, s.c_str(), s.length());
    ++op_index;
    return;
  }

  if (op_index == 1) {

    std::string s = "sub";
    memcpy(op_name, s.c_str(), s.length());
    ++op_index;
    return;
  }

  if (op_index == 2) {

    std::string s = "mul";
    memcpy(op_name, s.c_str(), s.length());
    ++op_index;
    return;
  }

  if (op_index == 3) {

    std::string s = "div";
    memcpy(op_name, s.c_str(), s.length());
    ++op_index;
    return;
  }

  if (op_index == 4) {

    std::string s = "dis";
    memcpy(op_name, s.c_str(), s.length());
    ++op_index;
    return;
  }
}

////////////////////////////////////////////////////////////////////////////////

uint16 GetProgramCount() {

  return 2;
}

void GetProgramName(char *pgm_name) {

  static uint16 pgm_index = 0;

  if (pgm_index == 0) {

    std::string s = "test_program";
    memcpy(pgm_name, s.c_str(), s.length());
    ++pgm_index;
    return;
  }
  /*
      if(pgm_index==1){

          std::string s="correlator";
          memcpy(pgm_name,s.c_str(),s.length());
          ++pgm_index;
          return;
      }
  */
  if (pgm_index == 1) {

    std::string s = "auto_focus";
    memcpy(pgm_name, s.c_str(), s.length());
    ++pgm_index;
    return;
  }
}

////////////////////////////////////////////////////////////////////////////////

uint16 GetCallbackCount() {

  return 1;
}

void GetCallbackName(char *callback_name) {

  static uint16 callback_index = 0;

  if (callback_index == 0) {

    std::string s = "print";
    memcpy(callback_name, s.c_str(), s.length());
    ++callback_index;
    return;
  }
}

void* GetUserOperatorFunction(const char* function_name) {
  if (strcmp(function_name, "Init") == 0)
    return (void*)&Init;
  else if (strcmp(function_name, "GetOperatorCount") == 0)
    return (void*)&GetOperatorCount;
  else if (strcmp(function_name, "GetOperatorName") == 0)
    return (void*)&GetOperatorName;
  else if (strcmp(function_name, "add") == 0)
    return (void*)&usr_operators::add;
  else if (strcmp(function_name, "sub") == 0)
    return (void*)&usr_operators::sub;
  else if (strcmp(function_name, "mul") == 0)
    return (void*)&usr_operators::mul;
  else if (strcmp(function_name, "div") == 0)
    return (void*)&usr_operators::div;
  else if (strcmp(function_name, "dis") == 0)
    return (void*)&usr_operators::dis;
  else if (strcmp(function_name, "GetProgramCount") == 0)
    return (void*)&GetProgramCount;
  else if (strcmp(function_name, "GetProgramName") == 0)
    return (void*)&GetProgramName;
  else if (strcmp(function_name, "test_program") == 0)
    return (void*)&test_program;
  else if (strcmp(function_name, "auto_focus") == 0)
    return (void*)&auto_focus;
  else if (strcmp(function_name, "GetCallbackCount") == 0)
    return (void*)&GetCallbackCount;
  else if (strcmp(function_name, "GetCallbackName") == 0)
    return (void*)&GetCallbackName;
  else if (strcmp(function_name, "print") == 0)
    return (void*)&usr_operators::print;
  else
    return NULL;
}
