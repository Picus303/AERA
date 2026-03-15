

#ifndef r_code_time_buffer_h
#define r_code_time_buffer_h

#include "list.h"
#include "utils.h"


using namespace core;

namespace r_code {

// Time limited buffer.
// T is expected a function: bool is_invalidated(uint64 time_reference,uint32 thz) const where time_reference and thz are valuated with the buffer's own.
template<typename T, class IsInvalidated> class time_buffer :
  public list<T> {
protected:
  std::chrono::microseconds thz_; // time horizon.
  Timestamp time_reference_;
public:
  time_buffer() : thz_(Utils::MaxTHZ) {}

  void set_thz(std::chrono::microseconds thz) { thz_ = thz; }

  class iterator {
    friend class time_buffer;
  private:
    time_buffer *buffer_;
    int32 cell_;
    iterator(time_buffer *b, int32 c) : buffer_(b), cell_(c) {}
  public:
    iterator() : buffer_(NULL), cell_(list<T>::null_) {}
    T &operator *() const { return buffer_->cells_[cell_].data_; }
    T *operator ->() const { return &(buffer_->cells_[cell_].data_); }
    iterator &operator ++() { // moves to the next time-compliant cell and erase old cells met in the process.

      cell_ = buffer_->cells_[cell_].next_;
      if (cell_ != list<T>::null_) {

        IsInvalidated i;
      check: if (i(buffer_->cells_[cell_].data_, buffer_->time_reference_, buffer_->thz_)) {

        cell_ = buffer_->_erase(cell_);
        if (cell_ != list<T>::null_)
          goto check;
      }
      }
      return *this;
    }
    bool operator==(iterator &i) const { return cell_ == i.cell_; }
    bool operator!=(iterator &i) const { return cell_ != i.cell_; }
  };
private:
  static iterator end_iterator_;
public:
  iterator begin(Timestamp time_reference) {

    time_reference_ = time_reference;
    return iterator(this, list<T>::used_cells_head_);
  }
  iterator &end() { return end_iterator_; }
  iterator find(Timestamp time_reference, const T &t) {

    iterator i;
    for (i = begin(time_reference); i != end(); ++i) {

      if ((*i) == t)
        return i;
    }
    return end_iterator_;
  }
  iterator find(const T &t) {

    for (int32 c = list<T>::used_cells_head_; c != list<T>::null_; c = list<T>::cells_[c].next_) {

      if (list<T>::cells_[c].data_ == t)
        return iterator(this, c);
    }
    return end_iterator_;
  }
  iterator erase(iterator &i) { return iterator(this, list<T>::_erase(i.cell_)); }
};

template<typename T, class IsInvalidated> typename time_buffer<T, IsInvalidated>::iterator time_buffer<T, IsInvalidated>::end_iterator_;
}


#endif
