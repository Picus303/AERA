

#ifndef image_impl_h
#define image_impl_h

#include "../submodules/CoreLibrary/CoreLibrary/types.h"


using namespace core;

namespace r_code {

class dll_export ImageImpl {
private:
  word32 *data_; // [object map|code segment|object names]
  Timestamp timestamp_;
  uint32 map_size_;
  uint32 code_size_;
  uint32 names_size_;
protected:
  Timestamp timestamp() const;
  uint32 map_size() const;
  uint32 code_size() const;
  uint32 names_size() const;
  word32 *data() const;
  word32 &data(uint32 i);
  word32 &data(uint32 i) const;
public:
  void *operator new(size_t, uint32 data_size);
  void operator delete(void *o);
  ImageImpl(Timestamp timestamp, uint32 map_size, uint32 code_size, uint32 names_size);
  ~ImageImpl();
};
}


#endif
