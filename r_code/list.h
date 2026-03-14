

#ifndef r_code_list_h
#define r_code_list_h

#include <vector>
#include "../submodules/CoreLibrary/CoreLibrary/types.h"


using namespace core;

namespace r_code {

// Minimalist list implemented as a vector.
// Possible optimization: get rid of the std::vector and manage allocation oneself.
// Insertion not needed for now; not implemented.
template<typename T> class list {
protected:
  static const int32 null_ = -1;

  class cell { // int32: to be robust WRT realloc(); this means that Ts can hold a cell index to speed up erasure.
  public:
    int32 next_;
    int32 prev_;
    T data_;
    cell() : next_(null_), prev_(null_) {}
  };

  std::vector<cell> cells_;

  int32 used_cells_head_;
  int32 used_cells_tail_;
  int32 free_cells_; // backward links unused.
  uint32 used_cell_count_;
  uint32 free_cell_count_;

  void push_back_free_cell(const T &t) {

    int32 free = free_cells_;
    free_cells_ = cells_[free_cells_].next_;
    --free_cell_count_;
    cells_[free].data_ = t;
    cells_[free].next_ = null_;
    cells_[free].prev_ = used_cells_tail_;
    used_cells_tail_ = free;
  }

  void push_back_new_cell(const T &t) {

    cell c;
    c.data_ = t;
    c.next_ = null_;
    c.prev_ = used_cells_tail_;
    cells_.push_back(c);
    used_cells_tail_ = cells_.size() - 1;
  }

  void update_used_cells_tail_state() {

    if (cells_[used_cells_tail_].prev_ != null_)
      cells_[cells_[used_cells_tail_].prev_].next_ = used_cells_tail_;
    if (used_cells_head_ == null_)
      used_cells_head_ = used_cells_tail_;
    ++used_cell_count_;
  }

  void push_front_free_cell(const T &t) {

    int32 free = free_cells_;
    free_cells_ = cells_[free_cells_].next_;
    --free_cell_count_;
    cells_[free].data_ = t;
    cells_[free].next_ = used_cells_head_;
    cells_[free].prev_ = null_;
    used_cells_head_ = free;
  }

  void push_front_new_cell(const T &t) {

    cell c;
    c.data_ = t;
    c.next_ = used_cells_head_;
    c.prev_ = null_;
    cells_.push_back(c);
    used_cells_head_ = cells_.size() - 1;
  }

  void update_used_cells_head_state() {

    if (cells_[used_cells_head_].next_ != null_)
      cells_[cells_[used_cells_head_].next_].prev_ = used_cells_head_;
    if (used_cells_tail_ == null_)
      used_cells_tail_ = used_cells_head_;
    ++used_cell_count_;
  }

  void __erase(int32 c) {

    if (cells_[c].prev_ != null_)
      cells_[cells_[c].prev_].next_ = cells_[c].next_;
    else
      used_cells_head_ = cells_[c].next_;
    if (cells_[c].next_ != null_)
      cells_[cells_[c].next_].prev_ = cells_[c].prev_;
    else
      used_cells_tail_ = cells_[c].prev_;
    cells_[c].next_ = free_cells_;
    free_cells_ = c;
    ++free_cell_count_;
    --used_cell_count_;
  }
  int32 _erase(int32 c) {

    int32 next_ = cells_[c].next_;
    __erase(c);
    return next_;
  }
public:
  list() : used_cells_head_(null_), used_cells_tail_(null_), free_cells_(null_), used_cell_count_(0), free_cell_count_(0) {}

  uint32 size() const { return used_cell_count_; }
  void reserve(uint32 size) { cells_.reserve(size); }
  void clear() {

    used_cells_head_ = used_cells_tail_ = free_cells_ = null_;
    used_cell_count_ = free_cell_count_ = 0;
    cells_.clear();
  }
  void push_back(const T &t) {

    if (free_cell_count_)
      push_back_free_cell(t);
    else
      push_back_new_cell(t);
    update_used_cells_tail_state();
  }
  void push_back(const T &t, int32 &location) {

    if (free_cell_count_) {

      location = free_cells_;
      push_back_free_cell(t);
    } else {

      push_back_new_cell(t);
      location = used_cells_tail_;
    }
    update_used_cells_tail_state();
  }
  void push_front(const T &t) {

    if (free_cell_count_)
      push_front_free_cell(t);
    else
      push_front_new_cell(t);
    update_used_cells_head_state();
  }
  void push_front(const T &t, int32 &location) {

    if (free_cell_count_) {

      location = free_cells_;
      push_front_free_cell(t);
    } else {

      push_front_new_cell(t);
      location = used_cells_tail_;
    }
    update_used_cells_head_state();
  }

  class _iterator {
  protected:
    int32 cell_;
    _iterator(int32 c) : cell_(c) {}
    _iterator() : cell_(null_) {}
  public:
    bool operator ==(const _iterator &i) const { return cell_ == i.cell_; }
    bool operator !=(const _iterator &i) const { return cell_ != i.cell_; }
  };

  class iterator;
  class const_iterator :
    public _iterator {
    friend class list;
  private:
    const list *list_;
    const_iterator(const list *l, int32 c) : _iterator(c), list_(l) {}
  public:
    const_iterator() : _iterator(), list_(NULL) {}
    const T &operator *() const { return list_->cells_[_iterator::cell_].data_; }
    const T *operator ->() const { return &(list_->cells_[_iterator::cell_].data_); }
    const_iterator &operator ++() {

      _iterator::cell_ = list_->cells_[_iterator::cell_].next_;
      return *this;
    }
    const_iterator &operator =(const const_iterator &i) {

      list_ = i.list_;
      _iterator::cell_ = i.cell_;
      return *this;
    }
    const_iterator &operator =(const iterator &i) {

      list_ = i.list_;
      _iterator::cell_ = i.cell_;
      return *this;
    }
  };

  class iterator :
    public _iterator {
    friend class list;
    friend class const_iterator;
  protected:
    list *list_;
    iterator(list *l, int32 c) : _iterator(c), list_(l) {}
  public:
    iterator() : _iterator(), list_(NULL) {}
    T &operator *() const { return list_->cells_[_iterator::cell_].data_; }
    T *operator ->() const { return &(list_->cells_[_iterator::cell_].data_); }
    iterator &operator ++() {

      _iterator::cell_ = list_->cells_[_iterator::cell_].next_;
      return *this;
    }
    iterator &operator =(const iterator &i) {

      list_ = i.list_;
      _iterator::cell_ = i.cell_;
      return *this;
    }
  };
private:
  static const_iterator end_iterator_;
public:
  iterator begin() { return iterator(this, used_cells_head_); }
  const_iterator begin() const { return const_iterator(this, used_cells_head_); }
  const_iterator &end() const { return end_iterator_; }
  iterator erase(iterator &i) { return iterator(this, _erase(i.cell_)); } // no check for i.cell_==null_.
  const_iterator erase(const_iterator &i) { return const_iterator(this, _erase(i.cell_)); } // no check for i.cell_==null_.
  void erase(int32 c) { __erase(c); } // use for random object deletion.
  void remove(const T &t) {

    const_iterator i(this, used_cells_head_);
    for (; i != end_iterator_; ++i) {

      if ((*i) == t) {

        erase(i);
        return;
      }
    }
  }
  T &front() { return cells_[used_cells_head_].data_; }
  T &back() { return cells_[used_cells_tail_].data_; }
};

template<typename T> typename list<T>::const_iterator list<T>::end_iterator_;
}


#endif
