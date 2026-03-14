#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <set>
#include <sstream>

#include "init.h"

#include "callbacks.h"
#include "compiler.h"
#include "controller.h"
#include "cpp_programs.h"
#include "mem.h"
#include "../object.h"
#include "opcodes.h"
#include "operator.h"
#include "overlay.h"
#include "preprocessor.h"

#ifdef WINDOWS
#include <process.h>
#endif

using namespace std;
using namespace std::chrono;
using namespace r_code;

namespace r_exec {

dll_export Timestamp (*Now)();

dll_export r_comp::Metadata Metadata;
dll_export r_comp::Image Seed;

unordered_map<std::string, uint16> _Opcodes;

dll_export r_comp::Compiler Compiler;
r_exec_dll r_comp::Preprocessor Preprocessor;

namespace {

bool Compile(std::istream& source_code, const std::string& file_path, std::string& error, bool compile_metadata) {

  ostringstream preprocessed_code_out;
  if (!r_exec::Preprocessor.process(&source_code, file_path, &preprocessed_code_out, error, compile_metadata ? &Metadata : NULL))
    return false;

  istringstream preprocessed_code_in(preprocessed_code_out.str());

  if (!r_exec::Compiler.compile(&preprocessed_code_in, &Seed, &Metadata, error, false)) {

    streampos i = preprocessed_code_in.tellg();
    cerr.write(preprocessed_code_in.str().c_str(), i);
    cerr << " <- " << error << endl;
    return false;
  }

  return true;
}

bool Compile(const char* filename, std::string& error, bool compile_metadata) {

  ifstream source_code(filename, ios::binary | ios::in);
  if (!source_code.good()) {

    error = "unable to load file ";
    error += filename;
    return false;
  }

  bool result = Compile(source_code, filename, error, compile_metadata);
  source_code.close();
  return result;
}

uint16 RetrieveOpcode(const char* name) {

  return _Opcodes.find(name)->second;
}

bool InitRuntime(FunctionLibrary* userOperatorLibrary,
  Timestamp (*time_base)()) {

  Now = time_base;

  if (!InitOpcodes(Metadata))
    return false;

  if (!userOperatorLibrary)
    return true;

  typedef uint16 (*OpcodeRetriever)(const char*);
  typedef r_code::resized_vector<uint16> (*UserInit)(OpcodeRetriever);
  auto init_user_operators = (UserInit)userOperatorLibrary->getFunction("Init");
  if (!init_user_operators)
    return false;

  typedef uint16 (*UserGetOperatorCount)();
  auto get_operator_count = (UserGetOperatorCount)userOperatorLibrary->getFunction("GetOperatorCount");
  if (!get_operator_count)
    return false;

  typedef void (*UserGetOperatorName)(char*);
  auto get_operator_name = (UserGetOperatorName)userOperatorLibrary->getFunction("GetOperatorName");
  if (!get_operator_name)
    return false;

  Metadata.usr_classes_ = init_user_operators(RetrieveOpcode);

  typedef bool (*UserOperator)(const Context&);

  uint16 operator_count = get_operator_count();
  for (uint16 i = 0; i < operator_count; ++i) {

    char op_name[256];
    memset(op_name, 0, 256);
    get_operator_name(op_name);

    unordered_map<std::string, uint16>::iterator it = _Opcodes.find(op_name);
    if (it == _Opcodes.end()) {

      cerr << "Operator " << op_name << " is undefined" << endl;
      exit(-1);
    }
    auto op = (UserOperator)userOperatorLibrary->getFunction(op_name);
    if (!op)
      return false;

    Operator::Register(it->second, op);
  }

  typedef uint16 (*UserGetProgramCount)();
  auto get_program_count = (UserGetProgramCount)userOperatorLibrary->getFunction("GetProgramCount");
  if (!get_program_count)
    return false;

  typedef void (*UserGetProgramName)(char*);
  auto get_program_name = (UserGetProgramName)userOperatorLibrary->getFunction("GetProgramName");
  if (!get_program_name)
    return false;

  typedef Controller* (*UserProgram)(_View*);

  uint16 program_count = get_program_count();
  for (uint16 i = 0; i < program_count; ++i) {

    char pgm_name[256];
    memset(pgm_name, 0, 256);
    get_program_name(pgm_name);

    auto pgm = (UserProgram)userOperatorLibrary->getFunction(pgm_name);
    if (!pgm)
      return false;

    CPPPrograms::Register(pgm_name, pgm);
  }

  typedef uint16 (*UserGetCallbackCount)();
  auto get_callback_count = (UserGetCallbackCount)userOperatorLibrary->getFunction("GetCallbackCount");
  if (!get_callback_count)
    return false;

  typedef void (*UserGetCallbackName)(char*);
  auto get_callback_name = (UserGetCallbackName)userOperatorLibrary->getFunction("GetCallbackName");
  if (!get_callback_name)
    return false;

  typedef bool (*UserCallback)(microseconds, bool, const char*, core::uint8, Code**);

  uint16 callback_count = get_callback_count();
  for (uint16 i = 0; i < callback_count; ++i) {

    char callback_name[256];
    memset(callback_name, 0, 256);
    get_callback_name(callback_name);

    auto callback = (UserCallback)userOperatorLibrary->getFunction(callback_name);
    if (!callback)
      return false;

    Callbacks::Register(callback_name, callback);
  }

  cout << "> user-defined operator library loaded" << endl;

  return true;
}

}  // namespace

bool Compile(const char* filename, std::string& error) {

  return Compile(filename, error, false);
}

bool Compile(std::istream& source_code, const std::string& file_path, std::string& error) {

  return Compile(source_code, file_path, error, false);
}

bool InitOpcodes(const r_comp::Metadata& metadata) {
  unordered_map<uint16, set<string>> opcode_names;
  unordered_map<std::string, r_comp::Class>::const_iterator it;
  for (it = metadata.classes_.begin(); it != metadata.classes_.end(); ++it) {

    _Opcodes[it->first] = it->second.atom_.asOpcode();
    auto opcode = it->second.atom_.asOpcode();
    if (opcode_names.find(opcode) == opcode_names.end())
      opcode_names[opcode] = set<string>();
    opcode_names[opcode].insert(it->first.c_str());
  }
  for (it = metadata.sys_classes_.begin(); it != metadata.sys_classes_.end(); ++it) {

    _Opcodes[it->first] = it->second.atom_.asOpcode();
    auto opcode = it->second.atom_.asOpcode();
    if (opcode_names.find(opcode) == opcode_names.end())
      opcode_names[opcode] = set<string>();
    opcode_names[opcode].insert(it->first.c_str());
  }
  if (!r_code::SetOpcodeNames(opcode_names))
    return false;

  View::ViewOpcode_ = _Opcodes.find("view")->second;

  Opcodes::View = _Opcodes.find("view")->second;
  Opcodes::PgmView = _Opcodes.find("pgm_view")->second;
  Opcodes::GrpView = _Opcodes.find("grp_view")->second;

  Opcodes::TI = _Opcodes.find("ti")->second;

  Opcodes::Ent = _Opcodes.find("ent")->second;
  Opcodes::Ont = _Opcodes.find("ont")->second;
  Opcodes::MkVal = _Opcodes.find("mk.val")->second;

  Opcodes::Grp = _Opcodes.find("grp")->second;

  Opcodes::Ptn = _Opcodes.find("ptn")->second;
  Opcodes::AntiPtn = _Opcodes.find("|ptn")->second;

  Opcodes::IPgm = _Opcodes.find("ipgm")->second;
  Opcodes::ICppPgm = _Opcodes.find("icpp_pgm")->second;

  Opcodes::Pgm = _Opcodes.find("pgm")->second;
  Opcodes::AntiPgm = _Opcodes.find("|pgm")->second;

  Opcodes::ICmd = _Opcodes.find("icmd")->second;
  Opcodes::Cmd = _Opcodes.find("cmd")->second;

  Opcodes::Fact = _Opcodes.find("fact")->second;
  Opcodes::AntiFact = _Opcodes.find("|fact")->second;

  Opcodes::Cst = _Opcodes.find("cst")->second;
  Opcodes::Mdl = _Opcodes.find("mdl")->second;

  Opcodes::ICst = _Opcodes.find("icst")->second;
  Opcodes::IMdl = _Opcodes.find("imdl")->second;

  Opcodes::Pred = _Opcodes.find("pred")->second;
  Opcodes::Goal = _Opcodes.find("goal")->second;

  Opcodes::Success = _Opcodes.find("success")->second;

  Opcodes::MkGrpPair = _Opcodes.find("mk.grp_pair")->second;

  Opcodes::MkRdx = _Opcodes.find("mk.rdx")->second;
  Opcodes::Perf = _Opcodes.find("perf")->second;

  Opcodes::MkNew = _Opcodes.find("mk.new")->second;

  Opcodes::MkLowRes = _Opcodes.find("mk.low_res")->second;
  Opcodes::MkLowSln = _Opcodes.find("mk.low_sln")->second;
  Opcodes::MkHighSln = _Opcodes.find("mk.high_sln")->second;
  Opcodes::MkLowAct = _Opcodes.find("mk.low_act")->second;
  Opcodes::MkHighAct = _Opcodes.find("mk.high_act")->second;
  Opcodes::MkSlnChg = _Opcodes.find("mk.sln_chg")->second;
  Opcodes::MkActChg = _Opcodes.find("mk.act_chg")->second;

  Opcodes::Sim = _Opcodes.find("sim")->second;
  Opcodes::Id = _Opcodes.find("id")->second;

  Opcodes::Inject = _Opcodes.find("_inj")->second;
  Opcodes::Eject = _Opcodes.find("_eje")->second;
  Opcodes::Mod = _Opcodes.find("_mod")->second;
  Opcodes::Set = _Opcodes.find("_set")->second;
  Opcodes::NewClass = _Opcodes.find("_new_class")->second;
  Opcodes::DelClass = _Opcodes.find("_del_class")->second;
  Opcodes::LDC = _Opcodes.find("_ldc")->second;
  Opcodes::Swap = _Opcodes.find("_swp")->second;
  Opcodes::Prb = _Opcodes.find("_prb")->second;
  Opcodes::Stop = _Opcodes.find("_stop")->second;

  Opcodes::Gtr = _Opcodes.find("gtr")->second;
  Opcodes::Lsr = _Opcodes.find("lsr")->second;
  Opcodes::Gte = _Opcodes.find("gte")->second;
  Opcodes::Lse = _Opcodes.find("lse")->second;
  Opcodes::Add = _Opcodes.find("add")->second;
  Opcodes::Sub = _Opcodes.find("sub")->second;
  Opcodes::Mul = _Opcodes.find("mul")->second;
  Opcodes::Div = _Opcodes.find("div")->second;

  Opcodes::Var = _Opcodes.find("var")->second;

  uint16 operator_opcode = 0;
  Operator::Register(operator_opcode++, now);
  Operator::Register(operator_opcode++, rnd);
  Operator::Register(operator_opcode++, equ);
  Operator::Register(operator_opcode++, neq);
  Operator::Register(operator_opcode++, gtr);
  Operator::Register(operator_opcode++, lsr);
  Operator::Register(operator_opcode++, gte);
  Operator::Register(operator_opcode++, lse);
  Operator::Register(operator_opcode++, add);
  Operator::Register(operator_opcode++, sub);
  Operator::Register(operator_opcode++, mul);
  Operator::Register(operator_opcode++, div);
  Operator::Register(operator_opcode++, dis);
  Operator::Register(operator_opcode++, ln);
  Operator::Register(operator_opcode++, exp);
  Operator::Register(operator_opcode++, log);
  Operator::Register(operator_opcode++, e10);
  Operator::Register(operator_opcode++, syn);
  Operator::Register(operator_opcode++, ins);
  Operator::Register(operator_opcode++, red);
  Operator::Register(operator_opcode++, fvw);
  Operator::Register(operator_opcode++, is_sim);
  Operator::Register(operator_opcode++, minimum);
  Operator::Register(operator_opcode++, maximum);
  Operator::Register(operator_opcode++, id);

  return true;
}

bool Init(FunctionLibrary* userOperatorLibrary,
  Timestamp (*time_base)(),
  const char* seed_path) {

  std::string error;
  if (!Compile(seed_path, error, true)) {
    cerr << error << endl;
    return false;
  }

  return InitRuntime(userOperatorLibrary, time_base);
}

bool Init(FunctionLibrary* userOperatorLibrary,
  Timestamp (*time_base)(),
  const r_comp::Metadata& metadata,
  const r_comp::Image& seed) {

  Metadata = metadata;
  Seed = seed;

  return InitRuntime(userOperatorLibrary, time_base);
}

uint16 GetOpcode(const char* name) {

  unordered_map<std::string, uint16>::iterator it = _Opcodes.find(name);
  if (it == _Opcodes.end())
    return 0xFFFF;
  return it->second;
}

std::string GetAxiomName(uint16 index) {

  return Compiler.getObjectName(index);
}

bool hasUserDefinedOperators(uint16 opcode) {
  const std::vector<uint16>* usr_defined_opcodes = Metadata.usr_classes_.as_std();
  return std::find(usr_defined_opcodes->begin(), usr_defined_opcodes->end(), opcode) != usr_defined_opcodes->end();
}

bool hasUserDefinedOperators(const std::string class_name) {
  if (_Opcodes.find(class_name) == _Opcodes.end())
    return false;
  return hasUserDefinedOperators(_Opcodes[class_name]);
}

}  // namespace r_exec
