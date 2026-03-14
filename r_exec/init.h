

#ifndef init_h
#define init_h

#include "../submodules/CoreLibrary/CoreLibrary/utils.h"

#include "../r_code/list.h"

#include "../r_comp/segments.h"
#include "../r_comp/compiler.h"
#include "../r_comp/preprocessor.h"

#include "dll.h"


namespace r_exec {

// Time base; either Time::Get or network-aware synced time.
extern r_exec_dll Timestamp (*Now)();

// Loaded once for all.
// Results from the compilation of user.classes.replicode.
// The latter contains all class definitions and all shared objects (e.g. ontology); does not contain any dynamic (res!=forever) objects.
extern r_exec_dll r_comp::Metadata Metadata;
extern r_exec_dll r_comp::Image Seed;

// A preprocessor and a compiler are maintained throughout the life of the dll to retain, respectively, macros and global references.
// Both functions add the compiled object to Seed.code_image.
// Source files: use ANSI encoding (not Unicode).
bool r_exec_dll Compile(const char *filename, std::string &error);
bool r_exec_dll Compile(std::istream &source_code, std::string &error);

// Threaded decompiler, for decompiling on the fly asynchronously.
// Named objects are referenced but not decompiled.
class r_exec_dll TDecompiler :
  public _Object {
private:
  static const uint32 ObjectsInitialSize = 16;
  static thread_ret thread_function_call Decompile(void *args);

  class _Thread :
    public Thread {
  };

  _Thread *thread_;
  volatile uint32 spawned_;

  r_code::list<P<r_code::Code> > objects_;

  uint32 ostream_id_; // 0 is std::cout.

  std::string header_;
public:
  TDecompiler(uint32 ostream_id, std::string header);
  ~TDecompiler();

  void add_object(r_code::Code *object);
  void add_objects(const r_code::list<P<r_code::Code> > &objects);
  void add_objects(const std::vector<P<r_code::Code> > &objects);
  void decompile();
};

// Spawns an instance of output_window.exe (see output_window project) and opens a pipe between the main process and output_window.
// Temporary solution:
// (a) not portable,
// (b) shall be defined in CoreLibrary instead of here,
// (c) the stream pool management (PipeOStream::Open(), PipeOStream::Close() and PipeOStream::Get()) shall be decoupled from this implementation (it's an IDE feature),
// (d) PipeOStream shall be based on std::ostringstream instead of std::ostream with a refined std::stringbuf (override sync() to write in the pipe).
class r_exec_dll PipeOStream :
  public std::ostream {
private:
  static std::vector<PipeOStream *> Streams_;
  static PipeOStream NullStream_;

#ifdef WINDOWS
  HANDLE pipe_read_;
  HANDLE pipe_write_;

  void init(); // create one child process and a pipe.
#endif
  PipeOStream();
public:
  static void Open(uint8 count); // open count streams.
  static void Close(); // close all streams.
  static PipeOStream &Get(uint8 id); // return NullStream if id is out of range.

  ~PipeOStream();

  PipeOStream &operator <<(std::string &s);
  PipeOStream &operator <<(const char *s);
};

/**
 * Use the given metadata (not r_exec::Metatdata) to initialize
 * r_exec::_Opcodes, r_code::OpcodeNames, View::ViewOpcode_,  and the
 * values in the Opcodes class such as Opcodes::Fact. Also call
 * Operator::Register to set up standard operators in Operator::Operators_.
 * \return True for success.
 */
bool r_exec_dll InitOpcodes(const r_comp::Metadata& metadata);

/**
 * Library is an abstract base class for a shared or static library with getFunction.
 */
class FunctionLibrary {
public:
  /**
   * Return the function with function_name or NULL if not found.
   */
  virtual void* getFunction(const char* function_name) = 0;
};

/**
 * SharedFunctionLibrary extends FunctionLibrary to implement
 * functionName using the SharedLibrary class.
 */
class SharedFunctionLibrary : public FunctionLibrary {
public:
  SharedFunctionLibrary() : getUserOperatorFunction_(0) {}

  SharedLibrary* load(const char* file_name) { 
    SharedLibrary* library = sharedLibrary_.load(file_name);
    getUserOperatorFunction_ = (void* (*)(const char*))sharedLibrary_.getFunction("GetUserOperatorFunction");
    return library;
  }

  void* getFunction(const char* function_name) override {
    if (getUserOperatorFunction_) {
      // Try GetUserOperatorFunction first.
      void* result = getUserOperatorFunction_(function_name);
      if (result)
        return result;
    }

    // Fall back to searching for the function in the shared library global name space.
    return sharedLibrary_.getFunction(function_name);
  }
private:
  SharedLibrary sharedLibrary_;
  void* (*getUserOperatorFunction_)(const char* function_name);
};

// Initialize Now, compile userOperatorLibrary, builds the Seed and loads the user-defined operators.
// Return false in case of a problem (e.g. file not found, operator not found, etc.).
bool r_exec_dll Init(FunctionLibrary* userOperatorLibrary,
  Timestamp (*time_base)(),
  const char *seed_path);

// Alternate taking a ready-made metadata and seed (will be copied into Metadata and Seed).
bool r_exec_dll Init(FunctionLibrary* userOperatorLibrary,
  Timestamp (*time_base)(),
  const r_comp::Metadata &metadata,
  const r_comp::Image &seed);

uint16 r_exec_dll GetOpcode(const char *name); // classes, operators and functions.

std::string r_exec_dll GetAxiomName(const uint16 index); // for constant objects (ex: self, position, and other axioms).

bool r_exec_dll hasUserDefinedOperators(const std::string class_name);

bool r_exec_dll hasUserDefinedOperators(const uint16 opcode);
}


#endif
