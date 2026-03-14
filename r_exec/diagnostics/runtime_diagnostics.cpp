#include "runtime_diagnostics.h"

#include <cstring>
#include <sstream>

#include "../../r_code/object.h"
#include "../../r_code/utils.h"
#include "decompiler.h"
#include "init.h"
#include "mem.h"

using namespace std;

namespace r_exec {

thread_ret TDecompiler::Decompile(void* args) {

  P<TDecompiler> _this = (TDecompiler*)args;
  _this->spawned_ = 1;

  r_comp::Decompiler decompiler;
  decompiler.init(&r_exec::Metadata);

  vector<r_code::SysObject*> imported_objects;

  r_comp::Image* image = new r_comp::Image();
  image->add_objects(_this->objects_, imported_objects);
  image->object_names_.symbols_ = r_exec::Seed.object_names_.symbols_;

  ostringstream decompiled_code;
  decompiler.decompile(image, &decompiled_code, r_code::Utils::GetTimeReference(), imported_objects);

  if (_this->ostream_id_ == 0) {
    cout << _this->header_;
    cout << decompiled_code.str();
  }
  else {
    PipeOStream::Get(_this->ostream_id_ - 1) << _this->header_;
    PipeOStream::Get(_this->ostream_id_ - 1) << decompiled_code.str();
  }

  if (!_this->thread_)
    return 0;
  thread_ret_val(0);
}

TDecompiler::TDecompiler(uint32 ostream_id, string header) : _Object(), ostream_id_(ostream_id), header_(std::move(header)), thread_(NULL), spawned_(0) {

  objects_.reserve(ObjectsInitialSize);
}

TDecompiler::~TDecompiler() {

  if (thread_)
    delete thread_;
}

void TDecompiler::add_object(r_code::Code* object) {

  objects_.push_back(object);
}

void TDecompiler::add_objects(const r_code::list<P<r_code::Code>>& objects) {

  r_code::list<P<r_code::Code>>::const_iterator object;
  for (object = objects.begin(); object != objects.end(); ++object)
    objects_.push_back(*object);
}

void TDecompiler::add_objects(const vector<P<r_code::Code>>& objects) {

  vector<P<r_code::Code>>::const_iterator object;
  for (object = objects.begin(); object != objects.end(); ++object)
    objects_.push_back(*object);
}

void TDecompiler::decompile() {

  if (_Mem::Get()->get_reduction_core_count() == 0 && _Mem::Get()->get_time_core_count() == 0)
    Decompile(this);
  else {
    thread_ = Thread::New<_Thread>(Decompile, this);
    while (spawned_ == 0);
  }
}

vector<PipeOStream*> PipeOStream::Streams_;

PipeOStream PipeOStream::NullStream_;

void PipeOStream::Open(uint8 count) {

  for (uint8 i = 0; i < count; ++i)
    Streams_.push_back(new PipeOStream());
}

void PipeOStream::Close() {

#ifdef WINDOWS
  for (uint8 i = 0; i < Streams_.size(); ++i)
    delete Streams_[i];
  Streams_.clear();
#endif
}

PipeOStream& PipeOStream::Get(uint8 id) {

  if (id < Streams_.size())
    return *Streams_[id];
  return NullStream_;
}

PipeOStream::PipeOStream() : std::ostream(NULL)
#ifdef WINDOWS
  , pipe_read_(0), pipe_write_(0)
#endif
{
}

PipeOStream::~PipeOStream() {

#ifdef WINDOWS
  if (pipe_read_ == 0)
    return;

  CloseHandle(pipe_read_);
  CloseHandle(pipe_write_);
#endif
}

PipeOStream& PipeOStream::operator <<(std::string& s) {

#ifdef WINDOWS
  if (pipe_read_ == 0)
    return *this;

  uint32 to_write = static_cast<uint32>(s.length());
  uint32 written;

  WriteFile(pipe_write_, s.c_str(), to_write, &written, NULL);
#endif

  return *this;
}

PipeOStream& PipeOStream::operator <<(const char* s) {

#ifdef WINDOWS
  if (pipe_read_ == 0)
    return *this;

  uint32 to_write = static_cast<uint32>(strlen(s));
  uint32 written;

  WriteFile(pipe_write_, s, to_write, &written, NULL);
#endif

  return *this;
}

}  // namespace r_exec
