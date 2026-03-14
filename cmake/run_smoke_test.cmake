if(NOT DEFINED AERA_EXE)
  message(FATAL_ERROR "AERA_EXE is required")
endif()

if(NOT DEFINED AERA_WORKING_DIR)
  message(FATAL_ERROR "AERA_WORKING_DIR is required")
endif()

if(NOT DEFINED AERA_SETTINGS_FILE)
  message(FATAL_ERROR "AERA_SETTINGS_FILE is required")
endif()

file(REMOVE "${AERA_WORKING_DIR}/decompiled_objects.txt")
file(REMOVE "${AERA_WORKING_DIR}/runtime_out.txt")

execute_process(
  COMMAND "${AERA_EXE}" "${AERA_SETTINGS_FILE}"
  WORKING_DIRECTORY "${AERA_WORKING_DIR}"
  RESULT_VARIABLE run_exit_code
  OUTPUT_VARIABLE run_output
  ERROR_VARIABLE run_error
)

set(combined_output "${run_output}${run_error}")

if(NOT run_exit_code EQUAL 0)
  message(FATAL_ERROR "AERA smoke test failed with exit code ${run_exit_code}\n${combined_output}")
endif()

set(expected_console_markers
  "usr operators initialized"
  "running for 1000 ms in diagnostic time"
  "hello world 1"
)

foreach(marker IN LISTS expected_console_markers)
  string(FIND "${combined_output}" "${marker}" marker_index)
  if(marker_index EQUAL -1)
    message(FATAL_ERROR "Expected console marker not found: ${marker}\n${combined_output}")
  endif()
endforeach()

set(decompiled_path "${AERA_WORKING_DIR}/decompiled_objects.txt")

if(NOT EXISTS "${decompiled_path}")
  message(FATAL_ERROR "Expected decompiled output was not created: ${decompiled_path}")
endif()

file(READ "${decompiled_path}" decompiled_output)

set(expected_decompiled_markers
  "root:(grp"
  "stdin:(grp"
  "stdout:(grp"
  "pgm0:(pgm"
)

foreach(marker IN LISTS expected_decompiled_markers)
  string(FIND "${decompiled_output}" "${marker}" marker_index)
  if(marker_index EQUAL -1)
    message(FATAL_ERROR "Expected decompiled marker not found: ${marker}")
  endif()
endforeach()

message(STATUS "AERA smoke test passed.")
