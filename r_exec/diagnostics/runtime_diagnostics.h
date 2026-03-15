#ifndef runtime_diagnostics_h
#define runtime_diagnostics_h

#include <ostream>
#include <string>
#include <vector>

#include "dll.h"
#include "list.h"
#include "../runtime/object.h"
#include "utils.h"

namespace r_exec {

// Threaded decompiler, for decompiling on the fly asynchronously.
// Named objects are referenced but not decompiled.
class r_exec_dll TDecompiler :
  public _Object {
private:
  static const uint32 ObjectsInitialSize = 16;
  static thread_ret thread_function_call Decompile(void* args);

  class _Thread :
    public Thread {
  };

  _Thread* thread_;
  volatile uint32 spawned_;

  r_code::list<P<r_code::Code>> objects_;

  uint32 ostream_id_; // 0 is std::cout.

  std::string header_;
public:
  TDecompiler(uint32 ostream_id, std::string header);
  ~TDecompiler();

  void add_object(r_code::Code* object);
  void add_objects(const r_code::list<P<r_code::Code>>& objects);
  void add_objects(const std::vector<P<r_code::Code>>& objects);
  void decompile();
};

// Legacy debug windows have been removed from the repository.
// PipeOStream now behaves as a sink stream placeholder so that the
// existing runtime tracing code can stay intact while the diagnostics
// layer is rebuilt later.
class r_exec_dll PipeOStream :
  public std::ostream {
private:
  static std::vector<PipeOStream*> Streams_;
  static PipeOStream NullStream_;

#ifdef WINDOWS
  HANDLE pipe_read_;
  HANDLE pipe_write_;
#endif
  PipeOStream();
public:
  static void Open(uint8 count); // open count streams.
  static void Close(); // close all streams.
  static PipeOStream& Get(uint8 id); // return NullStream if id is out of range.

  ~PipeOStream();

  PipeOStream& operator <<(std::string& s);
  PipeOStream& operator <<(const char* s);
};

}  // namespace r_exec

#endif
