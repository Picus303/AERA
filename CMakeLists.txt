cmake_minimum_required(VERSION 3.24)

project(
  AERA
  VERSION 1.0
  DESCRIPTION "Autocatalytic Endogenous Reflective Architecture"
  LANGUAGES CXX
)

include(CTest)

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
set(CORELIBRARY_DIR "${AERA_ROOT_DIR}/core")
set(R_CODE_DIR "${AERA_ROOT_DIR}/r_code")
set(R_COMP_DIR "${AERA_ROOT_DIR}/r_comp")
set(R_EXEC_DIR "${AERA_ROOT_DIR}/r_exec")
set(AERA_APP_DIR "${AERA_ROOT_DIR}/AERA")
set(MODULES_DIR "${AERA_ROOT_DIR}/modules")

set(R_CODE_INCLUDE_DIRS
  "${R_CODE_DIR}"
  "${R_CODE_DIR}/atoms"
  "${R_CODE_DIR}/images"
)

set(R_COMP_INCLUDE_DIRS
  "${R_COMP_DIR}"
  "${R_COMP_DIR}/frontend"
  "${R_COMP_DIR}/model"
  "${R_COMP_DIR}/backend"
)

set(R_EXEC_INCLUDE_DIRS
  "${R_EXEC_DIR}"
  "${R_EXEC_DIR}/attention"
  "${R_EXEC_DIR}/bootstrap"
  "${R_EXEC_DIR}/construction"
  "${R_EXEC_DIR}/controllers"
  "${R_EXEC_DIR}/diagnostics"
  "${R_EXEC_DIR}/evaluation"
  "${R_EXEC_DIR}/extensions"
  "${R_EXEC_DIR}/monitoring"
  "${R_EXEC_DIR}/metadata"
  "${R_EXEC_DIR}/scheduling"
  "${R_EXEC_DIR}/runtime"
  "${R_EXEC_DIR}/overlays"
  "${R_EXEC_DIR}/learning"
)

set(AERA_APP_INCLUDE_DIRS
  "${AERA_APP_DIR}/bootstrap"
  "${AERA_APP_DIR}/config"
  "${AERA_APP_DIR}/io"
)

set(MODULE_INCLUDE_DIRS
  "${MODULES_DIR}"
  "${MODULES_DIR}/builtin"
  "${MODULES_DIR}/builtin/callbacks"
  "${MODULES_DIR}/builtin/operators"
  "${MODULES_DIR}/builtin/programs"
)

add_library(aera_project_options INTERFACE)
target_compile_features(aera_project_options INTERFACE cxx_std_17)
target_include_directories(
  aera_project_options
  INTERFACE
    "${CORELIBRARY_DIR}"
    ${R_CODE_INCLUDE_DIRS}
    ${R_COMP_INCLUDE_DIRS}
    ${R_EXEC_INCLUDE_DIRS}
    ${MODULE_INCLUDE_DIRS}
)

if(WIN32)
  target_link_libraries(aera_project_options INTERFACE ws2_32)
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

add_library(aera_config STATIC "${AERA_APP_DIR}/config/settings.cpp")
target_link_libraries(aera_config PUBLIC aera_project_options CoreLibrary)
target_include_directories(aera_config PUBLIC "${AERA_APP_DIR}/config")

set(R_CODE_SOURCES
  "${R_CODE_DIR}/atoms/atom.cpp"
  "${R_CODE_DIR}/code_utils.cpp"
  "${R_CODE_DIR}/images/image.cpp"
  "${R_CODE_DIR}/images/image_impl.cpp"
  "${R_CODE_DIR}/object.cpp"
  "${R_CODE_DIR}/r_code.cpp"
)

add_library(r_code SHARED ${R_CODE_SOURCES})
target_link_libraries(r_code PUBLIC aera_project_options CoreLibrary)
target_include_directories(r_code PUBLIC ${R_CODE_INCLUDE_DIRS})
target_compile_definitions(r_code PRIVATE R_CODE_EXPORTS BUILD_DLL)
set_target_properties(r_code PROPERTIES OUTPUT_NAME "r_code")

set(R_COMP_SOURCES
  "${R_COMP_DIR}/model/class.cpp"
  "${R_COMP_DIR}/frontend/compiler.cpp"
  "${R_COMP_DIR}/backend/decompiler.cpp"
  "${R_COMP_DIR}/frontend/preprocessor.cpp"
  "${R_COMP_DIR}/frontend/r_comp.cpp"
  "${R_COMP_DIR}/model/segments.cpp"
  "${R_COMP_DIR}/model/structure_member.cpp"
)

add_library(r_comp SHARED ${R_COMP_SOURCES})
target_link_libraries(r_comp PUBLIC aera_project_options CoreLibrary r_code)
target_include_directories(r_comp PUBLIC ${R_COMP_INCLUDE_DIRS})
set_target_properties(r_comp PROPERTIES OUTPUT_NAME "r_comp")

set(R_EXEC_SOURCES
  "${R_EXEC_DIR}/attention/auto_focus.cpp"
  "${R_EXEC_DIR}/bootstrap/init.cpp"
  "${R_EXEC_DIR}/construction/factory.cpp"
  "${R_EXEC_DIR}/controllers/ast_controller.cpp"
  "${R_EXEC_DIR}/controllers/cst_controller.cpp"
  "${R_EXEC_DIR}/diagnostics/runtime_diagnostics.cpp"
  "${R_EXEC_DIR}/evaluation/_context.cpp"
  "${R_EXEC_DIR}/evaluation/binding_map.cpp"
  "${R_EXEC_DIR}/evaluation/context.cpp"
  "${R_EXEC_DIR}/evaluation/hlp_context.cpp"
  "${R_EXEC_DIR}/evaluation/operator.cpp"
  "${R_EXEC_DIR}/extensions/callbacks.cpp"
  "${R_EXEC_DIR}/extensions/cpp_programs.cpp"
  "${R_EXEC_DIR}/monitoring/g_monitor.cpp"
  "${R_EXEC_DIR}/controllers/guard_builder.cpp"
  "${R_EXEC_DIR}/controllers/hlp_controller.cpp"
  "${R_EXEC_DIR}/overlays/hlp_overlay.cpp"
  "${R_EXEC_DIR}/controllers/mdl_controller.cpp"
  "${R_EXEC_DIR}/metadata/opcodes.cpp"
  "${R_EXEC_DIR}/monitoring/monitor.cpp"
  "${R_EXEC_DIR}/overlays/overlay.cpp"
  "${R_EXEC_DIR}/learning/pattern_extractor.cpp"
  "${R_EXEC_DIR}/controllers/pgm_controller.cpp"
  "${R_EXEC_DIR}/overlays/pgm_overlay.cpp"
  "${R_EXEC_DIR}/monitoring/p_monitor.cpp"
  "${R_EXEC_DIR}/runtime/group.cpp"
  "${R_EXEC_DIR}/runtime/mem.cpp"
  "${R_EXEC_DIR}/runtime/model_base.cpp"
  "${R_EXEC_DIR}/runtime/view.cpp"
  "${R_EXEC_DIR}/scheduling/reduction_core.cpp"
  "${R_EXEC_DIR}/scheduling/reduction_job.cpp"
  "${R_EXEC_DIR}/scheduling/time_core.cpp"
  "${R_EXEC_DIR}/scheduling/time_job.cpp"
  "${MODULES_DIR}/builtin/module.cpp"
  "${MODULES_DIR}/builtin/callbacks/print_callback.cpp"
  "${MODULES_DIR}/builtin/operators/spatial_operators.cpp"
  "${MODULES_DIR}/builtin/programs/auto_focus_program.cpp"
  "${MODULES_DIR}/builtin/programs/test_program.cpp"
)

add_library(r_exec SHARED ${R_EXEC_SOURCES})
target_link_libraries(r_exec PUBLIC aera_project_options CoreLibrary r_code r_comp)
target_include_directories(r_exec PUBLIC ${R_EXEC_INCLUDE_DIRS})
target_compile_definitions(r_exec PRIVATE EXECUTIVE_EXPORTS)
set_target_properties(r_exec PROPERTIES OUTPUT_NAME "r_exec")

set(AERA_APP_SOURCES
  "${AERA_APP_DIR}/bootstrap/AERA_main.cpp"
  "${AERA_APP_DIR}/bootstrap/runtime_exports.cpp"
  "${AERA_APP_DIR}/bootstrap/runtime_runner.cpp"
  "${AERA_APP_DIR}/io/test_mem.cpp"
)

add_executable(aera ${AERA_APP_SOURCES})
target_link_libraries(aera PRIVATE aera_project_options CoreLibrary aera_config r_code r_comp r_exec)
target_include_directories(aera PRIVATE "${AERA_APP_DIR}" ${AERA_APP_INCLUDE_DIRS})
set_target_properties(aera PROPERTIES OUTPUT_NAME "AERA")

file(TO_CMAKE_PATH "${AERA_ROOT_DIR}/examples/replicode/user.classes.replicode" AERA_SMOKE_USER_CLASSES)
file(TO_CMAKE_PATH "${AERA_ROOT_DIR}/examples/replicode/main.replicode" AERA_SMOKE_SOURCE_FILE)

configure_file(
  "${AERA_ROOT_DIR}/cmake/smoke_settings.xml.in"
  "${CMAKE_BINARY_DIR}/smoke_settings.xml"
  @ONLY
)

configure_file(
  "${AERA_ROOT_DIR}/tests/invalid_settings.xml.in"
  "${CMAKE_BINARY_DIR}/invalid_settings.xml"
  @ONLY
)

add_executable(
  settings_characterization
  "${AERA_ROOT_DIR}/tests/settings_characterization.cpp"
)
target_link_libraries(settings_characterization PRIVATE aera_project_options aera_config)
target_include_directories(settings_characterization PRIVATE ${AERA_APP_INCLUDE_DIRS})

add_executable(
  builtin_module_characterization
  "${AERA_ROOT_DIR}/tests/builtin_module_characterization.cpp"
)
target_link_libraries(
  builtin_module_characterization
  PRIVATE
    aera_project_options
    CoreLibrary
    r_code
    r_comp
    r_exec
)

if(BUILD_TESTING)
  add_test(
    NAME settings_generated_smoke
    COMMAND
      "$<TARGET_FILE:settings_characterization>"
      valid_generated
      "${CMAKE_BINARY_DIR}/smoke_settings.xml"
  )

  add_test(
    NAME settings_repo_layout
    COMMAND
      "$<TARGET_FILE:settings_characterization>"
      repo_layout
      "${AERA_ROOT_DIR}/AERA/settings.xml"
  )

  add_test(
    NAME settings_invalid_missing_attribute
    COMMAND
      "$<TARGET_FILE:settings_characterization>"
      invalid_missing_attribute
      "${CMAKE_BINARY_DIR}/invalid_settings.xml"
  )

  add_test(
    NAME builtin_module_registration_contract
    COMMAND
      "$<TARGET_FILE:builtin_module_characterization>"
      registration_contract
  )

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
