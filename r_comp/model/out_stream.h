

#ifndef out_stream_h
#define out_stream_h

#include <iostream>

#include "../r_code/resized_vector.h"

namespace r_comp {

// Allows inserting data at a specified index and right shifting the current content;
// ex: labels and variables, i.e. when iptrs are discovered and these hold indexes are < read_index and do not point to variables
class OutStream {
public:
  r_code::resized_vector<uint16> code_indexes_to_stream_indexes_;
  r_code::resized_vector<std::streampos> positions_;
  OutStream(std::ostringstream *s) : stream_(s) {}
  std::ostringstream *stream_;
  template<typename T> OutStream &push(const T &t, uint16 code_index) {
    positions_.push_back(stream_->tellp());
    code_indexes_to_stream_indexes_[code_index] = positions_.size() - 1;
    return *this << t;
  }
  OutStream &push() { // to keep adding entries in v without outputing anything (e.g. for wildcards after ::)
    std::streampos p;
    positions_.push_back(p);
    return *this;
  }
  template<typename T> OutStream &operator <<(const T &t) {
    *stream_ << t;
    return *this;
  }
  template<typename T> OutStream &insert(uint32 index, const T &t) { // inserts before code_indexes_to_stream_indexes_[index]
    uint16 stream_index = code_indexes_to_stream_indexes_[index];
    stream_->seekp(positions_[stream_index]);
    std::string s = stream_->str().substr(positions_[stream_index]);
    *stream_ << t;
    std::streamoff offset = stream_->tellp() - positions_[stream_index];
    *stream_ << s;
    for (uint16 i = stream_index + 1; i < positions_.size(); ++i) // right-shift
      positions_[i] += offset;
    return *this;
  }
};

class NoStream :
  public std::ostream {
public:
  NoStream() : std::ostream(NULL) {}
  template<typename T> NoStream& operator <<(T &t) {
    return *this;
  }
};

class CompilerOutput :
  public std::ostream {
public:
  CompilerOutput() : std::ostream(NULL) {}
  template<typename T> std::ostream& operator <<(T &t) {
    if (1) // njt: was if (Output); HACK for linux compatibility
      return std::cout << t;
    return *this;
  }
};
}


#endif
