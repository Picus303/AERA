

#ifndef mem_output_h
#define mem_output_h

#include "../../core/base.h"
#include "../../core/utils.h"

namespace r_exec {

/**
 * The TraceLevel enum defines bit positions for the "trace_levels" parameter
 * for "Debug" in settings.xml.
 * The number of bits should match RUNTIME_OUTPUT_STREAM_COUNT.
 */
typedef enum {
  CST_IN = 0,
  CST_OUT = 1,
  MDL_IN = 2,
  MDL_OUT = 3,
  PRED_MON = 4,
  GOAL_MON = 5,
  MDL_REV = 6,
  HLP_INJ = 7,
  IO_DEVICE_INJ_EJT = 8,
  AUTO_FOCUS = 9
} TraceLevel;

/**
 * See _Mem::Output. This namespace-level function is declared here so that it can be 
 * used without including mem.h, which causes dependency loops if an inline header 
 * method needs to use it.
 */
std::ostream dll_export &_Mem_Output(TraceLevel l);

 /**
  * This similar to "_Mem_Output(level) << vals << endl", where vals can be "x << y << z". Except
  * that we first use an ostringstream to buffer the values plus the line terminator, and
  * then send the entire string to the output stream and flush. Assuming that the output
  * stream will output the entire string as a single operation, then when all threads use
  * OUTPUT_LINE it will avoid scrambling output of individual values.
  */
#define OUTPUT_LINE(level, vals) (_Mem_Output(level) << (std::ostringstream() << vals << '\n').str()).flush()

}

#endif
