

#ifndef r_code_vector_h
#define r_code_vector_h

#include <vector>
#include "../core/types.h"


using namespace core;

namespace r_code {

// auto-resized vector
#if 0 // If resize causes a crash, then enable this temporary fix which overrides resize but leaks memory.
  template<typename T> class resized_vector {
  public:
    resized_vector() : vector_(new std::vector<T>()) {}
    resized_vector(size_t n) : vector_(new std::vector<T>(n)) {}
    ~resized_vector() { delete vector_; }
    size_t size() const { return vector_->size(); }
    T& operator [](size_t i) {

      if (i >= size())
        resize(i + 1);
      return (*vector_)[i];
    }
    const T& operator [](size_t i) const {

      return (*vector_)[i];
    }
    void push_back(T t) { vector_->push_back(t); }
    void clear() { vector_->clear(); }
    void resize(size_t new_size) {
      if (new_size <= vector_->size()) {
        // No need to realloc.
        vector_->resize(new_size);
        return;
      }

      std::vector<T>* new_vector = new std::vector<T>(new_size);
      for (size_t i = 0; i < vector_->size(); ++i)
        (*new_vector)[i] = (*vector_)[i];
#if 0 // This fails, so comment it out. It's a memory leak, but the code runs.
      delete vector_;
#endif
      vector_ = new_vector;
    }
    const std::vector<T>* as_std() const { return vector_; }
  private:
    std::vector<T>* vector_;
  };
#else
template<typename T> class resized_vector {
public:
  resized_vector() {}
  resized_vector(size_t n) : vector_(n) {}
  size_t size() const { return vector_.size(); }
  T &operator [](size_t i) {

    if (i >= size())
      vector_.resize(i + 1);
    return vector_[i];
  }
  const T &operator [](size_t i) const {

    return vector_[i];
  }
  void push_back(T t) { vector_.push_back(t); }
  void clear() { vector_.clear(); }
  void resize(size_t new_size) { vector_.resize(new_size); }
  const std::vector<T>* as_std() const { return &vector_; }
private:
  std::vector<T> vector_;
};
#endif
}


#endif
