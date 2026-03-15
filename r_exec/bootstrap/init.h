#ifndef init_h
#define init_h

#include <istream>
#include <string>

#include "utils.h"
#include "runtime_diagnostics.h"

namespace r_comp {
class Metadata;
class Image;
}

namespace r_exec {

// Time base; either Time::Get or network-aware synced time.
extern r_exec_dll Timestamp (*Now)();

// Loaded once for all.
// Results from the compilation of user.classes.replicode.
// The latter contains all class definitions and all shared objects (e.g. ontology); does not contain any dynamic (res!=forever) objects.
extern r_exec_dll r_comp::Metadata Metadata;
extern r_exec_dll r_comp::Image Seed;

// A preprocessor and a compiler are maintained throughout the life of the runtime to retain, respectively, macros and global references.
// Both functions add the compiled object to Seed.code_image.
// Source files: use ANSI encoding (not Unicode).
bool r_exec_dll Compile(const char* filename, std::string& error);
bool r_exec_dll Compile(std::istream& source_code, const std::string& file_path, std::string& error);

/**
 * Use the given metadata (not r_exec::Metadata) to initialize
 * r_exec::_Opcodes, r_code::OpcodeNames, View::ViewOpcode_, and the
 * values in the Opcodes class such as Opcodes::Fact. Also call
 * Operator::Register to set up standard operators in Operator::Operators_.
 * \return True for success.
 */
bool r_exec_dll InitOpcodes(const r_comp::Metadata& metadata);

// Initialize Now, compile built-in extensions, build the Seed and load user-defined operators.
// Return false in case of a problem (e.g. file not found, operator not found, etc.).
bool r_exec_dll Init(Timestamp (*time_base)(),
  const char* seed_path);

// Alternate taking a ready-made metadata and seed (will be copied into Metadata and Seed).
bool r_exec_dll Init(Timestamp (*time_base)(),
  const r_comp::Metadata& metadata,
  const r_comp::Image& seed);

uint16 r_exec_dll GetOpcode(const char* name); // classes, operators and functions.

std::string r_exec_dll GetAxiomName(uint16 index); // for constant objects (ex: self, position, and other axioms).

bool r_exec_dll hasUserDefinedOperators(const std::string class_name);
bool r_exec_dll hasUserDefinedOperators(uint16 opcode);
}

#endif
