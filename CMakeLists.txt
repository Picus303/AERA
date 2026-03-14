cmake_minimum_required(VERSION 3.24)

project(
  AERA
  VERSION 1.0
  DESCRIPTION "Autocatalytic Endogenous Reflective Architecture"
  LANGUAGES CXX
)

include(CTest)

option(AERA_BUILD_OUTPUT_WINDOW "Build the legacy output_window helper" ON)
option(AERA_ENABLE_PROTOBUF "Enable TCP protobuf support" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(AERA_RUNTIME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
set(AERA_LIBRARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${AERA_RUNTIME_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${AERA_RUNTIME_OUTPUT_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${AERA_LIBRARY_OUTPUT_DIR}")

if(CMAKE_CONFIGURATION_TYPES)
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER "${config}" config_upper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${AERA_RUNTIME_OUTPUT_DIR}/${config}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${AERA_RUNTIME_OUTPUT_DIR}/${config}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${AERA_LIBRARY_OUTPUT_DIR}/${config}")
  endforeach()
endif()

set(AERA_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CORELIBRARY_DIR "${AERA_ROOT_DIR}/submodules/CoreLibrary/CoreLibrary")
set(R_CODE_DIR "${AERA_ROOT_DIR}/r_code")
set(R_COMP_DIR "${AERA_ROOT_DIR}/r_comp")
set(R_EXEC_DIR "${AERA_ROOT_DIR}/r_exec")
set(AERA_APP_DIR "${AERA_ROOT_DIR}/AERA")
set(USR_OPERATORS_DIR "${AERA_ROOT_DIR}/usr_operators")

add_library(aera_project_options INTERFACE)
target_compile_features(aera_project_options INTERFACE cxx_std_17)
target_include_directories(
  aera_project_options
  INTERFACE
    "${CORELIBRARY_DIR}"
    "${R_CODE_DIR}"
    "${R_COMP_DIR}"
    "${R_EXEC_DIR}"
)

if(WIN32)
  target_link_libraries(aera_project_options INTERFACE ws2_32)
endif()

if(AERA_ENABLE_PROTOBUF)
  find_package(Protobuf REQUIRED)
endif()

set(CORELIBRARY_SOURCES
  "${CORELIBRARY_DIR}/base.cpp"
  "${CORELIBRARY_DIR}/utils.cpp"
  "${CORELIBRARY_DIR}/xml_parser.cpp"
)

add_library(CoreLibrary SHARED ${CORELIBRARY_SOURCES})
target_link_libraries(CoreLibrary PUBLIC aera_project_options)
target_include_directories(CoreLibrary PUBLIC "${CORELIBRARY_DIR}")
target_compile_definitions(CoreLibrary PRIVATE CORELIBRARY_EXPORTS)
set_target_properties(CoreLibrary PROPERTIES OUTPUT_NAME "CoreLibrary")

set(R_CODE_SOURCES
  "${R_CODE_DIR}/atom.cpp"
  "${R_CODE_DIR}/code_utils.cpp"
  "${R_CODE_DIR}/image.cpp"
  "${R_CODE_DIR}/image_impl.cpp"
  "${R_CODE_DIR}/object.cpp"
  "${R_CODE_DIR}/r_code.cpp"
)

add_library(r_code SHARED ${R_CODE_SOURCES})
target_link_libraries(r_code PUBLIC aera_project_options CoreLibrary)
target_include_directories(r_code PUBLIC "${R_CODE_DIR}")
target_compile_definitions(r_code PRIVATE R_CODE_EXPORTS BUILD_DLL)
set_target_properties(r_code PROPERTIES OUTPUT_NAME "r_code")

set(R_COMP_SOURCES
  "${R_COMP_DIR}/class.cpp"
  "${R_COMP_DIR}/compiler.cpp"
  "${R_COMP_DIR}/decompiler.cpp"
  "${R_COMP_DIR}/preprocessor.cpp"
  "${R_COMP_DIR}/r_comp.cpp"
  "${R_COMP_DIR}/segments.cpp"
  "${R_COMP_DIR}/structure_member.cpp"
)

add_library(r_comp SHARED ${R_COMP_SOURCES})
target_link_libraries(r_comp PUBLIC aera_project_options CoreLibrary r_code)
target_include_directories(r_comp PUBLIC "${R_COMP_DIR}")
set_target_properties(r_comp PROPERTIES OUTPUT_NAME "r_comp")

set(R_EXEC_SOURCES
  "${R_EXEC_DIR}/ast_controller.cpp"
  "${R_EXEC_DIR}/auto_focus.cpp"
  "${R_EXEC_DIR}/binding_map.cpp"
  "${R_EXEC_DIR}/callbacks.cpp"
  "${R_EXEC_DIR}/_context.cpp"
  "${R_EXEC_DIR}/context.cpp"
  "${R_EXEC_DIR}/cpp_programs.cpp"
  "${R_EXEC_DIR}/cst_controller.cpp"
  "${R_EXEC_DIR}/factory.cpp"
  "${R_EXEC_DIR}/g_monitor.cpp"
  "${R_EXEC_DIR}/group.cpp"
  "${R_EXEC_DIR}/guard_builder.cpp"
  "${R_EXEC_DIR}/hlp_context.cpp"
  "${R_EXEC_DIR}/hlp_controller.cpp"
  "${R_EXEC_DIR}/hlp_overlay.cpp"
  "${R_EXEC_DIR}/init.cpp"
  "${R_EXEC_DIR}/mdl_controller.cpp"
  "${R_EXEC_DIR}/mem.cpp"
  "${R_EXEC_DIR}/model_base.cpp"
  "${R_EXEC_DIR}/monitor.cpp"
  "${R_EXEC_DIR}/opcodes.cpp"
  "${R_EXEC_DIR}/operator.cpp"
  "${R_EXEC_DIR}/overlay.cpp"
  "${R_EXEC_DIR}/pattern_extractor.cpp"
  "${R_EXEC_DIR}/pgm_controller.cpp"
  "${R_EXEC_DIR}/pgm_overlay.cpp"
  "${R_EXEC_DIR}/p_monitor.cpp"
  "${R_EXEC_DIR}/reduction_core.cpp"
  "${R_EXEC_DIR}/reduction_job.cpp"
  "${R_EXEC_DIR}/time_core.cpp"
  "${R_EXEC_DIR}/time_job.cpp"
  "${R_EXEC_DIR}/view.cpp"
)

add_library(r_exec SHARED ${R_EXEC_SOURCES})
target_link_libraries(r_exec PUBLIC aera_project_options CoreLibrary r_code r_comp)
target_include_directories(r_exec PUBLIC "${R_EXEC_DIR}")
target_compile_definitions(r_exec PRIVATE EXECUTIVE_EXPORTS)
set_target_properties(r_exec PROPERTIES OUTPUT_NAME "r_exec")

set(USR_OPERATORS_SOURCES
  "${USR_OPERATORS_DIR}/auto_focus_callback.cpp"
  "${USR_OPERATORS_DIR}/usr_operators.cpp"
  "${USR_OPERATORS_DIR}/Callbacks/callbacks.cpp"
  "${USR_OPERATORS_DIR}/Operators/operators.cpp"
  "${USR_OPERATORS_DIR}/TestProgram/test_program.cpp"
)

add_library(usr_operators SHARED ${USR_OPERATORS_SOURCES})
target_link_libraries(usr_operators PRIVATE aera_project_options CoreLibrary r_code r_comp r_exec)
target_include_directories(
  usr_operators
  PRIVATE
    "${USR_OPERATORS_DIR}"
    "${USR_OPERATORS_DIR}/Callbacks"
    "${USR_OPERATORS_DIR}/Operators"
    "${USR_OPERATORS_DIR}/TestProgram"
)
set_target_properties(usr_operators PROPERTIES OUTPUT_NAME "usr_operators")

set(AERA_APP_SOURCES
  "${AERA_APP_DIR}/AERA_main.cpp"
  "${AERA_APP_DIR}/main.cpp"
  "${AERA_APP_DIR}/test_mem.cpp"
  "${AERA_APP_DIR}/IODevices/TCP/tcp_io_device.cpp"
  "${AERA_APP_DIR}/IODevices/video_screen/video_screen.cpp"
  "${AERA_APP_DIR}/IODevices/video_screen/video_screen_io_device.cpp"
)

if(AERA_ENABLE_PROTOBUF)
  list(APPEND AERA_APP_SOURCES
    "${AERA_APP_DIR}/IODevices/TCP/AERA_Protobuf/tcp_connection.cpp"
    "${AERA_APP_DIR}/IODevices/TCP/AERA_Protobuf/tcp_data_message.pb.cc"
  )
endif()

add_executable(aera ${AERA_APP_SOURCES})
target_link_libraries(aera PRIVATE aera_project_options CoreLibrary r_code r_comp r_exec)
target_include_directories(
  aera
  PRIVATE
    "${AERA_APP_DIR}"
    "${AERA_APP_DIR}/IODevices/TCP"
    "${AERA_APP_DIR}/IODevices/video_screen"
)
target_compile_definitions(aera PRIVATE $<$<BOOL:${AERA_ENABLE_PROTOBUF}>:ENABLE_PROTOBUF>)
target_compile_definitions(usr_operators PRIVATE $<$<BOOL:${AERA_ENABLE_PROTOBUF}>:ENABLE_PROTOBUF>)
set_target_properties(aera PROPERTIES OUTPUT_NAME "AERA")
add_dependencies(aera usr_operators)

if(AERA_ENABLE_PROTOBUF)
  target_include_directories(
    aera
    PRIVATE
      "${AERA_APP_DIR}/IODevices/TCP/AERA_Protobuf"
  )
  target_link_libraries(aera PRIVATE protobuf::libprotobuf)
endif()

if(AERA_BUILD_OUTPUT_WINDOW)
  add_executable(output_window "${AERA_ROOT_DIR}/output_window/main.cpp")
  target_link_libraries(output_window PRIVATE aera_project_options CoreLibrary)
  set_target_properties(output_window PROPERTIES OUTPUT_NAME "output_window")
endif()

file(TO_CMAKE_PATH "${AERA_ROOT_DIR}/AERA/replicode_v1.2/user.classes.replicode" AERA_SMOKE_USER_CLASSES)
file(TO_CMAKE_PATH "${AERA_ROOT_DIR}/AERA/replicode_v1.2/main.replicode" AERA_SMOKE_SOURCE_FILE)
set(AERA_SMOKE_USER_OPERATOR_FILENAME "${CMAKE_SHARED_LIBRARY_PREFIX}usr_operators${CMAKE_SHARED_LIBRARY_SUFFIX}")

configure_file(
  "${AERA_ROOT_DIR}/cmake/smoke_settings.xml.in"
  "${CMAKE_BINARY_DIR}/smoke_settings.xml"
  @ONLY
)

if(BUILD_TESTING)
  add_test(
    NAME smoke_hello_world
    COMMAND
      "${CMAKE_COMMAND}"
      -DAERA_EXE=$<TARGET_FILE:aera>
      -DAERA_WORKING_DIR=$<TARGET_FILE_DIR:aera>
      -DAERA_SETTINGS_FILE=${CMAKE_BINARY_DIR}/smoke_settings.xml
      -P "${AERA_ROOT_DIR}/cmake/run_smoke_test.cmake"
  )
endif()
