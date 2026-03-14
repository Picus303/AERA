

#ifndef test_program_h
#define test_program_h

#include "../types.h"
#include "overlay.h"


extern "C" {
r_exec::Controller dll_export *test_program(r_code::_View *view);
}


#endif
