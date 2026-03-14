

#include "image_impl.h"


namespace r_code {

void *ImageImpl::operator new(size_t s, uint32 data_size) {

  return ::operator new(s);
}

void ImageImpl::operator delete(void *o) {

  ::operator delete(o);
}

ImageImpl::ImageImpl(Timestamp timestamp, uint32 map_size, uint32 code_size, uint32 names_size) : timestamp_(timestamp), map_size_(map_size), code_size_(code_size), names_size_(names_size) {

  data_ = new word32[map_size_ + code_size_ + names_size_];
}

ImageImpl::~ImageImpl() {

  delete[] data_;
}

Timestamp ImageImpl::timestamp() const {

  return timestamp_;
}

uint32 ImageImpl::map_size() const {

  return map_size_;
}

uint32 ImageImpl::code_size() const {

  return code_size_;
}

uint32 ImageImpl::names_size() const {

  return names_size_;
}

word32 *ImageImpl::data() const {

  return data_;
}

word32 &ImageImpl::data(uint32 i) {

  return data_[i];
}

word32 &ImageImpl::data(uint32 i) const {

  return data_[i];
}
}
